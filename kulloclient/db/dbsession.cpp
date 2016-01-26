/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/db/dbsession.h"

#include <iostream>
#include <sstream>
#include <thread>

#include <smartsqlite/connection.h>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/db/exceptions.h"
#include "kulloclient/util/assert.h"
#include "kulloclient/util/librarylogger.h"

using namespace Kullo::Util;

namespace Kullo {
namespace Db {

const unsigned int CURRENT_DB_VERSION(7);

static void profilingCallback(void *, const char *sql, std::uint64_t durationNanos)
{
    std::uint64_t durationMillis = durationNanos / (1000 * 1000);
    if (durationMillis > 500)
    {
        std::stringstream tidStream;
        tidStream << std::this_thread::get_id();
        std::string tid = tidStream.str();

        Log.w() << "Long running SQL statement in thread " << tid << ": "
                << durationMillis << "ms, SQL: " << sql;
    }
    else if (durationMillis > 100)
    {
        std::stringstream tidStream;
        tidStream << std::this_thread::get_id();
        std::string tid = tidStream.str();

        Log.d() << "Long running SQL statement in thread " << tid << ": "
                << durationMillis << "ms, SQL: " << sql;
    }
}

SharedSessionPtr makeSession(const std::string &dbFileName)
{
    SharedSessionPtr session = std::make_shared<SmartSqlite::Connection>(dbFileName);

    session->setBusyTimeout(3000);
    session->setProfilingCallback(&profilingCallback);
    session->exec("PRAGMA journal_mode=WAL");
    return session;
}

void migrate(SharedSessionPtr session)
{
    bool upgrade = false;
    SmartSqlite::ScopedTransaction tx(session, SmartSqlite::Exclusive);

    std::uint32_t version;
    { // statement
        auto stmt = session->prepare("PRAGMA user_version");
        version = stmt.begin()->get<int>(0);
    }

    if (version > CURRENT_DB_VERSION)
    {
        throw DatabaseIntegrityError(
                    std::string("Downgrading the DB (version ") +
                    std::to_string(version) + " to " +
                    std::to_string(CURRENT_DB_VERSION) +
                    ") is not supported."
                    );
    }

    upgrade = version < CURRENT_DB_VERSION;
    if (upgrade)
    {
        Log.i() << "Migrating database from version " << version << " to " << CURRENT_DB_VERSION;
    }

    while (version < CURRENT_DB_VERSION) {
        switch (version) {
        case 0:
            session->exec(
                        "CREATE TABLE participants ("
                        "message_id INTEGER UNIQUE,"
                        "name TEXT,"
                        "address TEXT NOT NULL,"
                        "organization TEXT,"
                        "avatar_mime_type TEXT,"
                        "avatar BLOB);");

            session->exec(
                        "CREATE INDEX participants_address_message_id "
                        "ON participants (address, message_id);");

            session->exec(
                        "CREATE TABLE conversations ("
                        "id INTEGER PRIMARY KEY,"
                        "participants TEXT UNIQUE NOT NULL);");

            session->exec(
                        "CREATE TABLE attachments ("
                        "draft BOOLEAN NOT NULL,"
                        "message_id INTEGER NOT NULL,"
                        "idx INTEGER NOT NULL,"
                        "filename TEXT,"
                        "mime_type TEXT,"
                        "size INTEGER NOT NULL,"
                        "hash TEXT,"
                        "note TEXT,"
                        "content BLOB,"
                        "PRIMARY KEY (draft, message_id, idx));");

            session->exec(
                        "CREATE TABLE messages ("
                        "id INTEGER NOT NULL,"
                        "conversation_id INTEGER NOT NULL,"
                        "sender TEXT,"
                        "last_modified INTEGER NOT NULL,"
                        "deleted BOOLEAN NOT NULL,"
                        "read BOOLEAN NOT NULL,"
                        "done BOOLEAN NOT NULL,"
                        "sent DATETIME,"
                        "received DATETIME,"
                        "text TEXT,"
                        "footer TEXT,"
                        "symmetric_key BLOB,"
                        "old BOOLEAN NOT NULL,"
                        "PRIMARY KEY (id, old));");

            session->exec(
                        "CREATE INDEX messages_sender ON messages (sender);");

            session->exec(
                        "CREATE TABLE messages_unprocessed ("
                        "id INTEGER PRIMARY KEY,"
                        "sender TEXT,"
                        "signature_key_id INTEGER NOT NULL,"
                        "last_modified INTEGER NOT NULL,"
                        "deleted BOOLEAN NOT NULL,"
                        "received DATETIME,"
                        "meta BLOB,"
                        "key_safe BLOB,"
                        "content BLOB,"
                        "has_attachments BOOLEAN NOT NULL);"
                        );

            session->exec(
                        "CREATE TABLE drafts ("
                        "id INTEGER NOT NULL,"
                        "conversation_id INTEGER NOT NULL,"
                        "last_modified INTEGER NOT NULL,"
                        "text TEXT,"
                        "footer TEXT,"
                        "sender_name TEXT,"
                        "sender_organization TEXT,"
                        "sender_avatar BLOB,"
                        "sender_avatar_mime_type TEXT,"
                        "state INTEGER NOT NULL,"
                        "old BOOLEAN NOT NULL,"
                        "PRIMARY KEY (conversation_id, old));");

            session->exec(
                        "CREATE TABLE keys_symm ("
                        "key_type TEXT PRIMARY KEY,"
                        "key BLOB NOT NULL);");

            session->exec(
                        "CREATE TABLE keys_asymm ("
                        "id INTEGER PRIMARY KEY,"
                        "key_type TEXT NOT NULL,"
                        "pubkey BLOB NOT NULL,"
                        "privkey BLOB NOT NULL,"
                        "valid_from DATETIME NOT NULL,"
                        "valid_until DATETIME NOT NULL,"
                        "revocation BLOB);");

            session->exec(
                        "CREATE TABLE keys_public ("
                        "address TEXT,"
                        "key_type TEXT NOT NULL,"
                        "id INTEGER NOT NULL,"
                        "key BLOB NOT NULL,"
                        "PRIMARY KEY (address, id, key_type));");
            break;

        case 1:
            session->exec(
                        "CREATE TABLE sync ("
                        "type TEXT PRIMARY KEY,"
                        "timestamp INTEGER NOT NULL);");
            break;

        case 2:
            // force resync of messages and add column 'meta_version'
            session->exec("DELETE FROM sync WHERE type = 'messages'");
            session->exec("DELETE FROM participants WHERE message_id IS NOT NULL");
            session->exec("DROP TABLE messages");
            session->exec(
                        "CREATE TABLE messages ("
                        "id INTEGER NOT NULL,"
                        "conversation_id INTEGER NOT NULL,"
                        "sender TEXT,"
                        "last_modified INTEGER NOT NULL,"
                        "deleted BOOLEAN NOT NULL,"
                        "meta_version INTEGER NOT NULL,"
                        "read BOOLEAN NOT NULL,"
                        "done BOOLEAN NOT NULL,"
                        "sent DATETIME,"
                        "received DATETIME,"
                        "text TEXT,"
                        "footer TEXT,"
                        "symmetric_key BLOB,"
                        "old BOOLEAN NOT NULL,"
                        "PRIMARY KEY (id, old));");
            break;

        case 3:
            // Force resync for everything but attachments and keys to fix crashes
            // for people that registered prior to 0.20 (DB version <= 2)
            // and that have conversation in which they never wrote a message.
            session->exec("DELETE FROM conversations");
            session->exec("DELETE FROM drafts");
            session->exec("DELETE FROM messages");
            session->exec("DELETE FROM messages_unprocessed");
            session->exec("DELETE FROM participants");
            session->exec("DELETE FROM sync WHERE type = 'messages'");
            break;

        case 4:
            // split attachments table into metadata (attachments) and content (attachments_content)
            session->exec(
                        "CREATE TABLE new_attachments ("
                        "draft BOOLEAN NOT NULL,"
                        "message_id INTEGER NOT NULL,"
                        "idx INTEGER NOT NULL,"
                        "filename TEXT,"
                        "mime_type TEXT,"
                        "size INTEGER NOT NULL,"
                        "hash TEXT,"
                        "note TEXT,"
                        "PRIMARY KEY (draft, message_id, idx));");
            session->exec(
                        "INSERT INTO new_attachments "
                        "SELECT draft, message_id, idx, filename, mime_type, size, hash, note "
                        "FROM attachments");
            session->exec(
                        "CREATE TABLE attachments_content ("
                        "draft BOOLEAN NOT NULL,"
                        "message_id INTEGER NOT NULL,"
                        "idx INTEGER NOT NULL,"
                        "content BLOB,"
                        "PRIMARY KEY (draft, message_id, idx));");
            session->exec(
                        "INSERT INTO attachments_content "
                        "SELECT draft, message_id, idx, content FROM attachments");
            session->exec("DROP TABLE attachments");
            session->exec("ALTER TABLE new_attachments RENAME TO attachments");
            break;

        case 5:
            // add new delivery table
            session->exec(
                        "CREATE TABLE delivery ("
                        "message_id INTEGER NOT NULL,"
                        "recipient TEXT NOT NULL,"
                        "state TEXT NOT NULL,"
                        "reason TEXT,"
                        "date TEXT,"
                        "lock_holder TEXT,"
                        "lock_expires TEXT)");

            // clean up participants: delete now-unused non-sender records
            session->exec("DELETE FROM participants WHERE message_id IS NULL");

            // tighten up schema of participants
            session->exec(
                        "CREATE TABLE new_participants ("
                        "message_id INTEGER PRIMARY KEY,"
                        "name TEXT NOT NULL,"
                        "address TEXT NOT NULL,"
                        "organization TEXT NOT NULL,"
                        "avatar_mime_type TEXT NOT NULL,"
                        "avatar BLOB)");
            session->exec(
                        "INSERT INTO new_participants "
                        "SELECT message_id, name, address, organization, avatar_mime_type, avatar "
                        "FROM participants");
            session->exec("DROP TABLE participants");
            session->exec("ALTER TABLE new_participants RENAME TO participants");
            break;

        case 6:
            session->exec(
                        "CREATE INDEX messages__conversation_id "
                        "ON messages (conversation_id);");
            session->exec(
                        "CREATE INDEX drafts__conversation_id "
                        "ON drafts (conversation_id);");
            session->exec(
                        "CREATE INDEX attachments__message_id "
                        "ON attachments (message_id);");
            break;

        default:
            session->rollbackTransaction();
            kulloAssert(false); // found no migration for given version
        }

        ++version;
        // SQLite doesn't seem to support parameters for PRAGMAs, so use concatenation
        session->exec(std::string("PRAGMA user_version = ") + std::to_string(version));
        Log.i() << "Database migrated to version " << version;
    }
    tx.commit();

    if (upgrade)
    {
        session->exec("VACUUM");
    }
}

bool hasCurrentSchema(SharedSessionPtr session)
{
    auto stmt = session->prepare("PRAGMA user_version");
    auto version = stmt.begin()->get<int>(0);
    return version == CURRENT_DB_VERSION;
}

}
}
