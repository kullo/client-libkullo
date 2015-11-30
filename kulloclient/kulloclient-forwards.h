/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

namespace Kullo {
    namespace Api {
        class Address;
        class AsyncTask;
        class Client;
        class MasterKey;
        class Registration;
        class Session;
        class UserSettings;

        enum class NetworkError;
    }
    namespace Codec {
        class MessageDecoder;
        class PrivateKeyProvider;
    }
    namespace Crypto {
        struct PrivateKeyImpl;
        struct PublicKeyImpl;
        class AsymmetricCryptor;
        class Signer;
        class SymmetricKey;
    }
    namespace Dao {
        class AsymmetricKeyPairDao;
        class AttachmentDao;
        class ConversationDao;
        class DraftDao;
        class MessageDao;
        class ParticipantDao;
        class PublicKeyDao;
        class SymmetricKeyDao;
        class UnprocessedMessageDao;
    }
    namespace Protocol {
        class AccountClient;
        class KeysClient;
        class MessagesClient;
        class PublicKeysClient;
    }
    namespace Sync {
        class AttachmentSyncer;
        class KeysSyncer;
        class MessagesUploader;
        class MessagesSender;
        class MessagesSyncer;

        enum struct SyncMode;
        struct SyncMessagesProgress;
        struct SyncProgress;
    }
    namespace Util {
        struct Delivery;
        class KulloAddress;
        class MasterKey;
        class UserSettings;
    }
}
