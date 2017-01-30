/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <memory>
#include <string>
#include <jsoncpp/jsoncpp.h>
#include <smartsqlite/exceptions.h>

#include <kulloclient/codec/exceptions.h>
#include <kulloclient/codec/messagedecoder.h>
#include <kulloclient/codec/messagedecryptor.h>
#include <kulloclient/crypto/hasher.h>
#include <kulloclient/crypto/symmetrickey.h>
#include <kulloclient/db/exceptions.h>
#include <kulloclient/protocol/exceptions.h>
#include <kulloclient/util/base64.h>
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
        : message_(initMessage())
    {
        initContent();
    }

    void addAllOptionalParts()
    {
        sender_["avatar"] = senderAvatar_;
        auto attIndex = Json::Value(Json::arrayValue);
        attIndex.append(attachmentsIndex_[0]);
        attIndex.append(attachmentsIndex_[1]);
        content_["attachmentsIndex"] = attIndex;
    }

    void assembleMessage()
    {
        content_["sender"] = sender_;
        content_["recipients"] = recipients_;
        message_.content.data = Util::to_vector(Json::FastWriter().write(content_));
        // leave out optional objects and arrays, as they could not be removed later on
    }

    Codec::MessageCompressor *makeMessageCompressor()
    {
        return new MessageCompressorStub();
    }

    std::shared_ptr<Codec::PrivateKeyProvider> makePrivKeyProvider()
    {
        return std::make_shared<MockPrivateKeyProvider>(sessionConfig_);
    }

    Json::Value sender_;
    Json::Value senderAvatar_;
    Json::Value recipients_;
    Json::Value attachmentsIndex_;
    Json::Value content_;
    Codec::DecryptedMessage message_;
    std::pair<metaVersion_type, std::string> decryptedMeta_;

private:
    void initContent()
    {
        sender_ = Json::Value(Json::objectValue);
        sender_["name"]         = "John Doe";
        sender_["address"]      = "john.doe#kullo.net";
        sender_["organization"] = "Doe Corp.";

        // echo -n "Hello, world." | base64
        senderAvatar_ = Json::Value(Json::objectValue);
        senderAvatar_["data"]     = "SGVsbG8sIHdvcmxkLg==";
        senderAvatar_["mimeType"] = "text/plain";

        recipients_ = Json::Value(Json::arrayValue);
        recipients_.append("alice#kullo.net");
        recipients_.append("bob#kullo.net");

        content_ = Json::Value(Json::objectValue);
        content_["dateSent"] = "2013-11-04T16:19:31+01:00";
        content_["text"]     = "Here comes the message text.";
        content_["footer"]   = "Here comes the footer.";

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

        attachmentsIndex_ = Json::Value(Json::arrayValue);
        attachmentsIndex_.append(attIndex0);
        attachmentsIndex_.append(attIndex1);
    }

    static Codec::DecryptedMessage initMessage()
    {
        Codec::DecryptedMessage result{Util::DateTime::nowUtc()};
        result.id = 42;
        result.lastModified = 1234567890;
        result.metaVersion = 1;
        result.meta = "{\"read\": 0, \"done\": 0}";
        return result;
    }
};


K_TEST_F(MessageDecoder, gettersReturnNullOrEmptyBeforeDecoding)
{
    TestData data;

    Codec::MessageDecoder decoder(
                message_,
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
                message_,
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
    auto &avatar = decoder.avatar();

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
    EXPECT_THAT(msgDao.id(), Eq(message_.id));
    EXPECT_THAT(msgDao.conversationId(), Eq(convDao.id()));
    EXPECT_THAT(msgDao.lastModified(), Eq(message_.lastModified));
    EXPECT_THAT(msgDao.sender(), Eq(sender_["address"].asString()));
    EXPECT_THAT(msgDao.deleted(), Eq(false));
    EXPECT_THAT(Util::DateTime(msgDao.dateSent()), Eq(Util::DateTime(content_["dateSent"].asString())));
    EXPECT_THAT(Util::DateTime(msgDao.dateReceived()), Eq(message_.dateReceived));
    EXPECT_THAT(msgDao.text(), Eq(content_["text"].asString()));
    EXPECT_THAT(msgDao.footer(), Eq(content_["footer"].asString()));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Read), Eq(false));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Done), Eq(false));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Unread), Eq(true));
    EXPECT_THAT(msgDao.state(Dao::MessageState::Undone), Eq(true));

    // attachments contents
    EXPECT_THAT(attDaos.size(), Eq(size_t{2}));
    for (Json::ArrayIndex i = 0; i < attDaos.size(); i++)
    {
        EXPECT_THAT(attDaos[i]->messageId(), Eq(message_.id));
        EXPECT_THAT(attDaos[i]->index(), Eq(i));
        EXPECT_THAT(attDaos[i]->filename(), Eq(attachmentsIndex_[i]["filename"].asString()));
        EXPECT_THAT(attDaos[i]->mimeType(), Eq(attachmentsIndex_[i]["mimeType"].asString()));
        EXPECT_THAT(attDaos[i]->size(), Eq(attachmentsIndex_[i]["size"].asDouble()));
        EXPECT_THAT(attDaos[i]->note(), Eq(attachmentsIndex_[i]["note"].asString()));
        EXPECT_THAT(attDaos[i]->deleted(), Eq(false));
        EXPECT_THROW(attDaos[i]->content(), SmartSqlite::Exception);
    }

    // sender contents
    EXPECT_THAT(senderDao.messageId(), Eq(message_.id));
    EXPECT_THAT(senderDao.name(), Eq(sender_["name"].asString()));
    EXPECT_THAT(senderDao.address().toString(), Eq(sender_["address"].asString()));
    EXPECT_THAT(senderDao.organization(), Eq(sender_["organization"].asString()));
    EXPECT_THAT(senderDao.avatarMimeType(), Eq(senderAvatar_["mimeType"].asString()));
    auto expectedAvatar = Util::Base64::decode(senderAvatar_["data"].asString());
    auto expectedAvatarHash = Crypto::Hasher::eightByteHash(expectedAvatar);
    ASSERT_THAT(senderDao.avatarHash().is_initialized(), Eq(true));
    EXPECT_THAT(*senderDao.avatarHash(), Eq(expectedAvatarHash));

    EXPECT_THAT(avatar, Eq(expectedAvatar));
}

bool compareElementsInParticipantsSet(const std::shared_ptr<Dao::ParticipantDao> &lhs, const std::shared_ptr<Dao::ParticipantDao> &rhs)
{
    return lhs->address() < rhs->address();
}

K_TEST_F(MessageDecoder, validMin)
{
    TestData data;

    sender_.removeMember("organization");
    recipients_.resize(1); // Send to alice#kullo.net only
    content_.removeMember("text");
    content_.removeMember("footer");
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    decoder.decode(Codec::VerifySignature::False);
    decoder.makeConversation();

    // things that should be empty
    EXPECT_THAT(decoder.attachments(), IsEmpty());

    // things that should be non-empty
    Dao::ParticipantDao &senderDao = decoder.sender();

    // Only check stuff that might be affected by removing optional stuff.
    // Everything else is checked in validMax.

    EXPECT_THAT(senderDao.organization(), IsEmpty());
    EXPECT_THAT(senderDao.avatarMimeType(), IsEmpty());
    EXPECT_THAT(senderDao.avatarHash(), Eq(boost::none));

    auto &msgDao = decoder.message();
    EXPECT_THAT(msgDao.text(), IsEmpty());
    EXPECT_THAT(msgDao.footer(), IsEmpty());
}

K_TEST_F(MessageDecoder, optionalAttachmentNote)
{
    TestData data;

    attachmentsIndex_[0].removeMember("note");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    decoder.decode(Codec::VerifySignature::False);
    decoder.makeConversation();

    std::vector<std::unique_ptr<Dao::AttachmentDao>> &attDaos = decoder.attachments();
    ASSERT_THAT(attDaos.size(), Eq(2u));

    EXPECT_THAT(attDaos[0]->note(), IsEmpty());
}

K_TEST_F(MessageDecoder, runDecodeTwice)
{
    TestData data;

    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
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
                message_,
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
                message_,
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
    message_.content.data = Util::to_vector("{this is invalid json");

    Codec::MessageDecoder decoder(
                message_,
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
    message_.content.data = Util::to_vector("[23, 42]");

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAvatar1)
{
    TestData data;

    senderAvatar_.removeMember("mimeType");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}


K_TEST_F(MessageDecoder, invalidAvatar2)
{
    TestData data;

    senderAvatar_.removeMember("data");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, recipientsEmpty)
{
    TestData data;

    recipients_.resize(0);
    EXPECT_THAT(recipients_, IsEmpty());

    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );

    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, recipientsContainSender)
{
    TestData data;

    recipients_.append(sender_["address"]);

    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );

    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment1)
{
    TestData data;

    attachmentsIndex_[0].removeMember("filename");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment2)
{
    TestData data;

    attachmentsIndex_[0].removeMember("mimeType");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment3)
{
    TestData data;

    attachmentsIndex_[0].removeMember("size");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}

K_TEST_F(MessageDecoder, invalidAttachment4)
{
    TestData data;

    attachmentsIndex_[0].removeMember("hash");
    addAllOptionalParts();
    assembleMessage();

    Codec::MessageDecoder decoder(
                message_,
                data.userAddress,
                session_,
                makeMessageCompressor()
    );
    ASSERT_THROW(decoder.decode(Codec::VerifySignature::False), Codec::InvalidContentFormat);
}
