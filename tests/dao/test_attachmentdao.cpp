/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "tests/dbtest.h"

#include <kulloclient/dao/attachmentdao.h>
#include <kulloclient/util/binary.h>

using namespace testing;
using namespace Kullo;

class AttachmentDao : public DbTest
{
public:
    AttachmentDao() {
        data_.id = 42;
        data_.convId = 23;
        data_.filename = "bla.txt";
        data_.mimeType = "text/plain";
        data_.hash = "ad0c37c31d69b315f3a81f13c8cde701"
                     "094ad91725ba1b0dc3199ca9713661b8"
                     "280d6ef7e68f133e6211e2e5a9a31504"
                     "45d76f1708e04521b0ee034f0b0baf26";
        data_.content = Util::to_vector("Hello, world.");
        data_.filesize = data_.content.size();
    }

protected:
    struct {
        id_type id;
        id_type convId;
        std::string filename;
        std::string mimeType;
        std::string hash;
        std::vector<unsigned char> content;
        size_t filesize;
    } data_;
};

K_TEST_F(AttachmentDao, deleteAttachmentsForDraft)
{
    auto attachmentId1 = Kullo::id_type{0};
    auto attachmentId2 = Kullo::id_type{100};
    auto attachmentId3 = Kullo::id_type{200000};
    auto attachmentId4 = Kullo::ID_MAX;

    {
        Dao::AttachmentDao dao(session_);
        dao.setDraft(true);
        dao.setMessageId(data_.convId);
        dao.setIndex(attachmentId1);
        dao.setFilename(data_.filename);
        dao.setMimeType(data_.mimeType);
        dao.setSize(data_.filesize);
        dao.setHash(data_.hash);
        dao.save();
        dao.setContent(data_.content);
    }
    {
        Dao::AttachmentDao dao(session_);
        dao.setDraft(true);
        dao.setMessageId(data_.convId);
        dao.setIndex(attachmentId2);
        dao.setFilename(data_.filename);
        dao.setMimeType(data_.mimeType);
        dao.setSize(data_.filesize);
        dao.setHash(data_.hash);
        dao.save();
        dao.setContent(data_.content);
    }
    {
        Dao::AttachmentDao dao(session_);
        dao.setDraft(true);
        dao.setMessageId(data_.convId);
        dao.setIndex(attachmentId3);
        dao.setFilename(data_.filename);
        dao.setMimeType(data_.mimeType);
        dao.setSize(data_.filesize);
        dao.setHash(data_.hash);
        dao.save();
        dao.setContent(data_.content);
    }
    {
        Dao::AttachmentDao dao(session_);
        dao.setDraft(true);
        dao.setMessageId(data_.convId);
        dao.setIndex(attachmentId4);
        dao.setFilename(data_.filename);
        dao.setMimeType(data_.mimeType);
        dao.setSize(data_.filesize);
        dao.setHash(data_.hash);
        dao.save();
        dao.setContent(data_.content);
    }

    ASSERT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId1, session_), NotNull());
    ASSERT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId2, session_), NotNull());
    ASSERT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId3, session_), NotNull());
    ASSERT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId4, session_), NotNull());

    auto result = Dao::AttachmentDao::deleteAttachmentsForDraft(data_.convId, session_);

    EXPECT_THAT(result.size(), Eq(4u));
    EXPECT_THAT(result[0], Eq(attachmentId1));
    EXPECT_THAT(result[1], Eq(attachmentId2));
    EXPECT_THAT(result[2], Eq(attachmentId3));
    EXPECT_THAT(result[3], Eq(attachmentId4));

    EXPECT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId1, session_), IsNull());
    EXPECT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId2, session_), IsNull());
    EXPECT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId3, session_), IsNull());
    EXPECT_THAT(Dao::AttachmentDao::load(Dao::IsDraft::Yes, data_.convId, attachmentId4, session_), IsNull());
}
