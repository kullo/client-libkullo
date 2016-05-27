/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

namespace Kullo {
    namespace Api {
        class Address;
        class AsyncTask;
        class Challenge;
        class Client;
        class MasterKey;
        class Registration;
        class Session;
        class UserSettings;

        // Records
        struct AccountInfo;
        struct DateTime;
        struct Event;
        struct SyncProgress;

        enum class ChallengeType;
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
        class ProfileSyncer;

        enum struct SyncMode;
        struct SyncMessagesProgress;
        struct SyncProgress;
    }
    namespace Util {
        struct Credentials;
        struct Delivery;
        struct UserSettings;
        class DateTime;
        class KulloAddress;
        class MasterKey;
    }
}
