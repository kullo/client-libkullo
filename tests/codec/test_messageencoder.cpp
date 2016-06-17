/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <cmath>
#include <memory>
#include <jsoncpp/jsoncpp.h>

#include <kulloclient/codec/messageencoder.h>
#include <kulloclient/dao/attachmentdao.h>
#include <kulloclient/dao/conversationdao.h>
#include <kulloclient/db/exceptions.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/checkedconverter.h>
#include <kulloclient/util/masterkey.h>
#include <kulloclient/util/misc.h>

#include "tests/dbtest.h"
#include "tests/testdata.h"

using namespace testing;
using namespace Kullo;
using namespace Kullo::Codec;
using namespace Kullo::Dao;

namespace {
    struct TestData
    {
        const Util::DateTime dateSent = Util::DateTime("2000-11-22T22:33:44+02:00");
        const std::vector<unsigned char> helloWorldData = Util::to_vector("Hello, world.");
    };
}

class MessageEncoderTest : public DbTest
{
protected:
    MessageEncoderTest()
        : settings(
              Util::Credentials(
                  std::make_shared<Util::KulloAddress>("john.doe#kullo.net"),
                  std::make_shared<Util::MasterKey>(MasterKeyData::VALID_DATA_BLOCKS)))
    {
        settings.name = "John Doe";
        participants.emplace_back("alice#kullo.net");
    }

    void addAvatar()
    {
        TestData data;
        settings.avatarMimeType = "text/plain";
        settings.avatarData = data.helloWorldData;
        // echo -n "Hello, world." | base64
        avatarEncoded = "SGVsbG8sIHdvcmxkLg==";
    }

    void addAttachment(const std::string &filename, const std::string &mimeType,
                       const std::string &note, const std::vector<unsigned char> &content)
    {
        id_type id = AttachmentDao::idForNewDraftAttachment(draft->conversationId(), session_);
        auto att = make_unique<AttachmentDao>(session_);
        att->setDraft(true);
        att->setMessageId(draft->conversationId());
        att->setIndex(id);
        att->setFilename(filename);
        att->setMimeType(mimeType);
        att->setNote(note);
        att->setSize(content.size());
        att->save();
        att->setContent(content);

        attachments.emplace_back(std::move(att));
    }

    void makeConvAndDraft()
    {
        std::sort(participants.begin(), participants.end());

        std::set<Util::KulloAddress> parts;
        for (auto partStr : participants)
        {
            parts.insert(Util::KulloAddress(partStr));
        }
        ConversationDao conv(session_);
        conv.setParticipants(parts);
        conv.save();

        draft.reset(new DraftDao(session_));
        draft->setConversationId(conv.id());
        draft->setState(DraftState::Sending);
        draft->setText(text);
        draft->setFooter(footer);
        draft->setSenderName(settings.name);
        draft->setSenderOrganization(settings.organization);
        draft->setSenderAvatar(settings.avatarData);
        draft->setSenderAvatarMimeType(settings.avatarMimeType);
        draft->save();
    }

    void checkOptionalString(const Json::Value &val, const std::string &expected)
    {
        if (expected.empty())
        {
            if (val.isNull())
            {
                // everything is okay (expected is empty, val doesn't exist)
            }
            else
            {
                EXPECT_THAT(val.isString(), Eq(true));
                EXPECT_THAT(val.asString(), Eq(std::string()));
            }
        }
        else
        {
            EXPECT_THAT(val.asString(), expected);
        }
    }

    void checkResults(EncodedMessage &encMsg)
    {
        TestData data;

        auto contentString = Util::to_string(encMsg.content);

        Json::Reader jsonReader(Json::Features::strictMode());
        Json::Value content;
        jsonReader.parse(contentString, content, false);

        checkSender(content["sender"]);
        checkRecipients(content["recipients"]);

        auto dateSentFromJson = Util::CheckedConverter::toDateTime(content["dateSent"]);
        EXPECT_THAT(dateSentFromJson, Eq(data.dateSent));

        {
            SCOPED_TRACE("content.text");
            checkOptionalString(content["text"], text);
        }

        {
            SCOPED_TRACE("content.footer");
            checkOptionalString(content["footer"], footer);
        }

        checkAttachments(content["attachmentsIndex"], encMsg);
    }

    void checkSender(const Json::Value &sender)
    {
        EXPECT_THAT(sender["name"].asString(), Eq(settings.name));
        EXPECT_THAT(Util::KulloAddress(sender["address"].asString()),
                Eq(*settings.credentials.address));

        {
            SCOPED_TRACE("sender.organization");
            checkOptionalString(sender["organization"], settings.organization);
        }

        auto avatarJson = sender["avatar"];
        if (avatarEncoded.empty())
        {
            if (avatarJson.isNull())
            {
                // everything is okay
            }
            else
            {
                ASSERT_THAT(avatarJson.isObject(), Eq(true));
                EXPECT_THAT(avatarJson.empty(), Eq(true));
            }
        }
        else
        {
            EXPECT_THAT(avatarJson["mimeType"].asString(), Eq(settings.avatarMimeType));
            EXPECT_THAT(avatarJson["data"].asString(), Eq(avatarEncoded));
        }
    }

    void checkRecipients(const Json::Value &recipients)
    {
        ASSERT_THAT(recipients.size(), Eq(participants.size()));
        for (size_t i = 0; i < participants.size(); ++i)
        {
            EXPECT_THAT(recipients[Json::ArrayIndex(i)].asString(), Eq(participants[i]));
        }
    }

    void checkAttachments(const Json::Value &attachmentsIndexJson, EncodedMessage &encMsg)
    {
        TestData data;

        size_t totalSize = 0;

        // content: attachmentIndex
        if (attachments.empty())
        {
            if (attachmentsIndexJson.isNull())
            {
                // everything is okay
            }
            else
            {
                ASSERT_THAT(attachmentsIndexJson.isArray(), Eq(true));
                EXPECT_THAT(attachmentsIndexJson.empty(), Eq(true));
            }
        }
        else
        {
            ASSERT_THAT(attachmentsIndexJson.isArray(), Eq(true));

            for (size_t i = 0; i < attachments.size(); ++i)
            {
                ASSERT_THAT(attachmentsIndexJson[Json::ArrayIndex(i)].isObject(), Eq(true));
                auto attachmentJson = attachmentsIndexJson[Json::ArrayIndex(i)];

                EXPECT_THAT(attachmentJson["filename"].asString(), Eq(attachments[i]->filename()));
                EXPECT_THAT(attachmentJson["mimeType"].asString(), Eq(attachments[i]->mimeType()));
                EXPECT_THAT(attachmentJson["size"].asUInt(), Eq(attachments[i]->size()));
                totalSize += attachmentJson["size"].asUInt();
                {
                    SCOPED_TRACE("attachment note");
                    checkOptionalString(attachmentJson["note"], attachments[i]->note());
                }
            }
        }

        // attachments
        EXPECT_THAT(encMsg.attachments.empty(), Eq(attachments.empty()));
        EXPECT_THAT(encMsg.attachments.size(), Eq(totalSize));
    }

    Util::UserSettings settings;
    std::vector<std::string> participants;
    std::string text, footer, avatarEncoded;
    std::unique_ptr<DraftDao> draft;
    std::vector<std::unique_ptr<AttachmentDao>> attachments;
};

K_TEST_F(MessageEncoderTest, validMin)
{
    makeConvAndDraft();

    TestData data;
    auto encMsg = MessageEncoder::encodeMessage(
                settings.credentials, *draft, data.dateSent, session_);
    checkResults(encMsg);
}

K_TEST_F(MessageEncoderTest, validTwoParticipants)
{
    participants.emplace_back("bob#kullo.net");

    makeConvAndDraft();

    TestData data;
    auto encMsg = MessageEncoder::encodeMessage(
                settings.credentials, *draft, data.dateSent, session_);
    checkResults(encMsg);
}

K_TEST_F(MessageEncoderTest, validAvatar)
{
    addAvatar();
    makeConvAndDraft();

    TestData data;
    auto encMsg = MessageEncoder::encodeMessage(
                settings.credentials, *draft, data.dateSent, session_);
    checkResults(encMsg);
}

K_TEST_F(MessageEncoderTest, validOneAttachment)
{
    TestData data;

    makeConvAndDraft();
    addAttachment("hello.txt", "text/plain", "", data.helloWorldData);

    auto encMsg = MessageEncoder::encodeMessage(
                settings.credentials, *draft, data.dateSent, session_);
    checkResults(encMsg);
}

K_TEST_F(MessageEncoderTest, validTwoAttachments)
{
    TestData data;

    makeConvAndDraft();
    addAttachment("hello.txt", "text/plain", "", data.helloWorldData);
    addAttachment("hello.html", "text/html", "This is a note", Util::to_vector("<b>I'm very bold.</b>"));

    auto encMsg = MessageEncoder::encodeMessage(
                settings.credentials, *draft, data.dateSent, session_);
    checkResults(encMsg);
}

K_TEST_F(MessageEncoderTest, validMax)
{
    TestData data;

    settings.organization = "Doe Corp.";
    addAvatar();

    text = "This is the body text.";
    footer = "This is the footer text.";

    makeConvAndDraft();
    addAttachment("hello.txt", "text/plain", "", data.helloWorldData);
    addAttachment("hello.html", "text/html", "This is a note", Util::to_vector("<b>I'm very bold.</b>"));

    auto encMsg = MessageEncoder::encodeMessage(
                settings.credentials, *draft, data.dateSent, session_);
    checkResults(encMsg);
}

K_TEST_F(MessageEncoderTest, noConversation)
{
    TestData data;

    draft.reset(new DraftDao(session_));
    draft->setConversationId(42);
    draft->setState(DraftState::Sending);
    draft->save();

    ASSERT_THROW(
                MessageEncoder::encodeMessage(
                    settings.credentials, *draft, data.dateSent, session_),
                Db::DatabaseIntegrityError);
}
