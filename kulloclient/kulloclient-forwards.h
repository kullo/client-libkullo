/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

namespace Kullo {
    namespace Api {
        class AddressHelpers;
        class AsyncTask;
        class Challenge;
        class Client;
        class InternalEvent;
        class MasterKeyHelpers;
        class Registration;
        class Session;
        class UserSettings;

        // Records
        struct Address;
        struct AccountInfo;
        struct DateTime;
        struct Event;
        struct MasterKey;
        struct MessagesSearchResult;
        struct SenderPredicate;
        struct SyncProgress;

        enum class AddressNotAvailableReason;
        enum class ChallengeType;
        enum class LocalError;
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
        class MessageSearchDao;
        class PublicKeyDao;
        class SenderDao;
        class SymmetricKeyDao;
        class UnprocessedMessageDao;
    }
    namespace Http {
        // Interfaces
        class HttpClient;

        // Records
        struct Request;
        struct Response;
        struct TransferProgress;

        // Enums
        enum class ProgressResult;
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
        struct SyncIncomingMessagesProgress;
        struct SyncIncomingAttachmentsProgress;
        struct SyncOutgoingMessagesProgress;
        struct SyncProgress;
    }
    namespace Util {
        struct Credentials;
        struct Delivery;
        struct UserSettings;
        class DateTime;
        class KulloAddress;
        class MasterKey;
        class StlTaskRunner;
    }
}
