/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include <memory>
#include <string>
#include <jsoncpp/jsoncpp.h>
#include <smartsqlite/exceptions.h>

#include <kulloclient/codec/exceptions.h>
#include <kulloclient/codec/messagedecoder.h>
#include <kulloclient/codec/messagedecryptor.h>
#include <kulloclient/crypto/symmetrickey.h>
#include <kulloclient/db/exceptions.h>
#include <kulloclient/protocol/exceptions.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/checkedconverter.h>
#include <kulloclient/util/datetime.h>

#include "tests/codec/mock_messagedecryptor.h"
#include "tests/codec/mock_privatekeyprovider.h"
#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;

namespace {
    struct TestData
    {
        const Util::KulloAddress userAddress = Util::KulloAddress("test#kullo.net");
        const Crypto::SymmetricKey privateDataKey = Crypto::SymmetricKey("AAAAAAAAccccccccAAAAAAAAcccccccc");
        const std::string keySafeJsonString = "{\"msgFormat\": 1, "
                                              "\"symmCipher\": \"AES-256/GCM\", "
                                              "\"symmKey\": \"0123456789abcdef0123456789abcdef0123456789a\", "
                                              "\"hashAlgo\": \"SHA-512\"}";
    };
}

class MessageCompressorStub : public Codec::MessageCompressor
{
public:
    std::vector<unsigned char> decompress(const std::vector<unsigned char> &data) const override
    {
        return data;
    }
};

class MessageDecoder : public DbTest
{
protected:
    MessageDecoder()
    {
        initContent();
        initMessage();
    }

    void addAllOptionalParts()
    {
        sender["avatar"] = senderAvatar;
        auto attIndex = Json::Value(Json::arrayValue);
        attIndex.append(attachmentsIndex[0]);
        attIndex.append(attachmentsIndex[1]);
        content["attachmentsIndex"] = attIndex;
    }

    void assembleMessage()
    {
        content["sender"] = sender;
        content["recipients"] = recipients;
        message.content.data = Util::to_vector(Json::FastWriter().write(content));
        // leave out optional objects and arrays, as they could not be removed later on
    }

    Codec::MessageCompressor *makeMessageCompressor()
    {
        return new MessageCompressorStub();
    }

    std::shared_ptr<Codec::PrivateKeyProvider> makePrivKeyProvider()
    {
        return std::make_shared<MockPrivateKeyProvider>(dbPath_);
    }

    Json::Value sender;
    Json::Value senderAvatar;
    Json::Value recipients;
    Json::Value attachmentsIndex;
    Json::Value content;
    Codec::DecryptedMessage message;
    std::pair<metaVersion_type, std::string> decryptedMeta;

private:
    void initContent()
    {
        sender = Json::Value(Json::objectValue);
        sender["name"]         = "John Doe";
        sender["address"]      = "john.doe#kullo.net";
        sender["organization"] = "Doe Corp.";

        // echo -n "Hello, world." | base64
        senderAvatar = Json::Value(Json::objectValue);
        senderAvatar["data"]     = "SGVsbG8sIHdvcmxkLg==";
        senderAvatar["mimeType"] = "text/plain";

        recipients = Json::Value(Json::arrayValue);
        recipients.append("alice#kullo.net");
        recipients.append("bob#kullo.net");

        content = Json::Value(Json::objectValue);
        content["dateSent"] = "2013-11-04T16:19:31+01:00";
        content["text"]     = "Here comes the message text.";
        content["footer"]   = "Here comes the footer.";

        auto attIndex0 = Json::Value(Json::objectValue);
        attIndex0["filename"] = "my_first_file.pdf";
        attIndex0["mimeType"] = "application/pdf";
        attIndex0["note"]     = "First file";
        attIndex0["size"]     = 123456;
        attIndex0["hash"]     = "330efc27491a2b69a7360a3d5b087104497093068200fc10aebdb6aacca3447271441a83299f5f46dde634d0cb525fbe88c0adbdff9987b914856d21d10db3ae";

        auto attIndex1 = Json::Value(Json::objectValue);
        attIndex1["filename"] = "my second file.jpg";
        attIndex1["mimeType"] = "application/jpeg";
        attIndex1["note"]     = "Last file";
        attIndex1["size"]     = 1234567;
        attIndex1["hash"]     = "35477e68e5476ace4ba9db8bc82d414e34c06d6b67ffba91a2145b0962ba37f2e7ded0ccef150851ec84d43e0e881788fa670ac30769ec391dfb011f3f7dd5a8";

        attachmentsIndex = Json::Value(Json::arrayValue);
        attachmentsIndex.append(attIndex0);
        attachmentsIndex.append(attIndex1);
    }

    void initMessage()
    {
        message.id = 42;
        message.lastModified = 1234567890;
        message.dateReceived = Util::DateTime::nowUtc();
        message.metaVersion = 1;
        message.meta = "{\"read\": 0, \"done\": 0}";
    }
};


K_TEST_F(MessageDecoder, gettersReturnNullOrEmptyBeforeDecoding)
{
    TestData data;

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_
    );

    EXPECT_THROW(decoder.conversation(), Util::AssertionFailed);
    EXPECT_THROW(decoder.attachments(), Util::AssertionFailed);
    EXPECT_THROW(decoder.sender(), Util::AssertionFailed);
}

K_TEST_F(MessageDecoder, validMax)
{
    TestData data;

    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    decoder.decode(Codec::VerifySignature::False);
    decoder.makeConversation();

    // things that should be non-empty
    auto &convDao = decoder.conversation();
    auto &attDaos = decoder.attachments();
    auto &senderDao = decoder.sender();

    // conversation contents
    EXPECT_THAT(convDao.id(), Eq(id_type{0}));
    std::string participants("alice#kullo.net,bob#kullo.net,john.doe#kullo.net");
    std::vector<std::string> participantsList = {
        "alice#kullo.net",
        "bob#kullo.net",
        "john.doe#kullo.net"
    };
    EXPECT_THAT(convDao.participants(), Eq(participants));
    EXPECT_THAT(convDao.participantsList(), Eq(participantsList));

    // message contents
    auto &msgDao = decoder.message();
    EXPECT_THAT(msgDao.old(), Eq(false));
    EXPECT_THAT(msgDao.id(), Eq(message.id));
    EXPECT_THAT(msgDao.conversationId(), Eq(convDao.id()));
    EXPECT_THAT(msgDao.lastModified(), Eq(message.lastModified));
    EXPECT_THAT(msgDao.sender(), Eq(sender["address"].asString()));
    EXPECT_THAT(msgDao.deleted(), Eq(false));
    EXPECT_THAT(Util::DateTime(msgDao.dateSent()), Eq(Util::DateTime(content["dateSent"].asString())));
    EXPECT_THAT(Util::DateTime(msgDao.dateReceived()), Eq(message.dateReceived));
    EXPECT_THAT(msgDao.text(), Eq(content["text"].asString()));
    EXPECT_THAT(msgDao.footer(), Eq(content["footer"].asString()));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Read), Eq(false));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Done), Eq(false));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Unread), Eq(true));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Undone), Eq(true));

    // attachments contents
    EXPECT_THAT(attDaos.size(), Eq(size_t{2}));
    for (Json::ArrayIndex i = 0; i < attDaos.size(); i++)
    {
        EXPECT_THAT(attDaos[i]->messageId(), Eq(message.id));
        EXPECT_THAT(attDaos[i]->index(), Eq(i));
        EXPECT_THAT(attDaos[i]->filename(), Eq(attachmentsIndex[i]["filename"].asString()));
        EXPECT_THAT(attDaos[i]->mimeType(), Eq(attachmentsIndex[i]["mimeType"].asString()));
        EXPECT_THAT(attDaos[i]->size(), Eq(attachmentsIndex[i]["size"].asDouble()));
        EXPECT_THAT(attDaos[i]->note(), Eq(attachmentsIndex[i]["note"].asString()));
        EXPECT_THAT(attDaos[i]->deleted(), Eq(false));
        EXPECT_THROW(attDaos[i]->content(), SmartSqlite::Exception);
    }

    // sender contents
    EXPECT_THAT(senderDao.messageId(), Eq(message.id));
    EXPECT_THAT(senderDao.name(), Eq(sender["name"].asString()));
    EXPECT_THAT(senderDao.address().toString(), Eq(sender["address"].asString()));
    EXPECT_THAT(senderDao.organization(), Eq(sender["organization"].asString()));
    EXPECT_THAT(senderDao.avatarMimeType(), Eq(senderAvatar["mimeType"].asString()));
    EXPECT_THAT(senderDao.avatar(), Eq(Util::to_vector("Hello, world.")));
}

bool compareElementsInParticipantsSet(const std::shared_ptr<Dao::ParticipantDao> &lhs, const std::shared_ptr<Dao::ParticipantDao> &rhs)
{
    return lhs->address() < rhs->address();
}

K_TEST_F(MessageDecoder, validMin)
{
    TestData data;

    sender.removeMember("organization");
    recipients.resize(1); // Send to alice#kullo.net only
    content.removeMember("text");
    content.removeMember("footer");
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    decoder.decode(Codec::VerifySignature::False);
    decoder.makeConversation();

    // things that should be empty
    EXPECT_TRUE(decoder.attachments().empty());

    // things that should be non-empty
    Dao::ParticipantDao &senderDao = decoder.sender();

    // Only check stuff that might be affected by removing optional stuff.
    // Everything else is checked in validMax.

    EXPECT_TRUE(senderDao.organization().empty());
    EXPECT_TRUE(senderDao.avatarMimeType().empty());
    EXPECT_TRUE(senderDao.avatar().empty());

    auto &msgDao = decoder.message();
    EXPECT_TRUE(msgDao.text().empty());
    EXPECT_TRUE(msgDao.footer().empty());
}

K_TEST_F(MessageDecoder, optionalAttachmentNote)
{
    TestData data;

    attachmentsIndex[0].removeMember("note");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    decoder.decode(Codec::VerifySignature::False);
    decoder.makeConversation();

    std::vector<std::unique_ptr<Dao::AttachmentDao>> &attDaos = decoder.attachments();
    ASSERT_EQ(size_t{2}, attDaos.size());

    EXPECT_TRUE(attDaos[0]->note().empty());
}

K_TEST_F(MessageDecoder, runDecodeTwice)
{
    TestData data;

    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );

    EXPECT_NO_THROW(decoder.decode(Codec::VerifySignature::False));
    EXPECT_THROW(decoder.decode(Codec::VerifySignature::False), Util::AssertionFailed);
}

K_TEST_F(MessageDecoder, runDecodeMakeParticipantsAndConversationTwice)
{
    TestData data;

    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );

    EXPECT_NO_THROW(decoder.decode(Codec::VerifySignature::False));
    EXPECT_NO_THROW(decoder.makeConversation());
    EXPECT_THROW(decoder.makeConversation(), Util::AssertionFailed);
}

K_TEST_F(MessageDecoder, runMakeParticipantsAndConversationBeforeDecode)
{
    TestData data;

    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );

    EXPECT_THROW(decoder.makeConversation(), Util::AssertionFailed);
}


K_TEST_F(MessageDecoder, throwsOnInvalidContentJson)
{
    TestData data;

    assembleMessage();
    message.content.data = Util::to_vector("{this is invalid json");

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, throwsOnArrayInContentJson)
{
    TestData data;

    assembleMessage();
    message.content.data = Util::to_vector("[23, 42]");

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAvatar1)
{
    TestData data;

    senderAvatar.removeMember("mimeType");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}


K_TEST_F(MessageDecoder, invalidAvatar2)
{
    TestData data;

    senderAvatar.removeMember("data");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidRecipients)
{
    TestData data;

    recipients.resize(0);
    EXPECT_THAT(recipients, IsEmpty());

    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}


K_TEST_F(MessageDecoder, invalidAttachment1)
{
    TestData data;

    attachmentsIndex[0].removeMember("filename");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment2)
{
    TestData data;

    attachmentsIndex[0].removeMember("mimeType");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment3)
{
    TestData data;

    attachmentsIndex[0].removeMember("size");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment4)
{
    TestData data;

    attachmentsIndex[0].removeMember("hash");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}
