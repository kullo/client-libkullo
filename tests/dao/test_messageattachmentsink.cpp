/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/dao/messageattachmentsink.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/ref.hpp>
#include <kulloclient/dao/attachmentdao.h>
#include <kulloclient/db/exceptions.h>
#include <kulloclient/util/binary.h>
#include <kulloclient/util/misc.h>
#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;
namespace io = boost::iostreams;

class MessageAttachmentSink : public DbTest
{
public:
    MessageAttachmentSink()
    {
        data_.id = 42;
        data_.msgId = 23;
        data_.index = 5;
        data_.filename = "bla.txt";
        data_.mimeType = "text/plain";
        data_.hash = "ad0c37c31d69b315f3a81f13c8cde701"
                     "094ad91725ba1b0dc3199ca9713661b8"
                     "280d6ef7e68f133e6211e2e5a9a31504"
                     "45d76f1708e04521b0ee034f0b0baf26";
        data_.content = Util::to_vector("Hello, world.");
        data_.filesize = data_.content.size();

        {
            Dao::AttachmentDao dao(session_);
            dao.setDraft(false);
            dao.setMessageId(data_.msgId);
            dao.setIndex(data_.index);
            dao.setFilename(data_.filename);
            dao.setMimeType(data_.mimeType);
            dao.setSize(data_.filesize);
            dao.setHash(data_.hash);
            dao.save();
        }

        uut_ = makeUut();
    }

    std::unique_ptr<Dao::MessageAttachmentSink> makeUut()
    {
        return make_unique<Dao::MessageAttachmentSink>(
                    session_, data_.msgId, data_.index, data_.filesize);
    }

    size_t writeDataToUut(std::vector<unsigned char>::size_type length)
    {
        auto bufPtr = reinterpret_cast<char*>(data_.content.data());
        auto writeLength = static_cast<std::streamsize>(length);
        auto bytesWritten = uut_->write(bufPtr, writeLength);
        return static_cast<size_t>(bytesWritten);
    }

protected:
    struct {
        id_type id;
        id_type msgId;
        id_type index;
        std::string filename;
        std::string mimeType;
        std::string hash;
        std::vector<unsigned char> content;
        size_t filesize;
    } data_;

    std::unique_ptr<Dao::MessageAttachmentSink> uut_;
};

K_TEST_F(MessageAttachmentSink, canBeAddedToFilteringOstream)
{
    io::filtering_ostream out;
    out.push(boost::ref(*uut_));
}

K_TEST_F(MessageAttachmentSink, closeThrowsWhenThereWasNoWrite)
{
    EXPECT_THROW(uut_->close(), Db::DatabaseIntegrityError);
}

K_TEST_F(MessageAttachmentSink, closeThrowsWhenDataIsTooShort)
{
    auto bytesWritten = writeDataToUut(data_.content.size() - 1);
    ASSERT_THAT(bytesWritten, Eq(data_.content.size() - 1));

    EXPECT_THROW(uut_->close(), Db::DatabaseIntegrityError);
}

K_TEST_F(MessageAttachmentSink, writeThrowsWhenDataIsTooLong)
{
    auto bytesWritten = writeDataToUut(data_.content.size());
    ASSERT_THAT(bytesWritten, Eq(data_.content.size()));

    EXPECT_THROW(uut_->write("A", 1), Db::DatabaseIntegrityError);
}

K_TEST_F(MessageAttachmentSink, writeSucceedsWhenDataHasTheCorrectLenght)
{
    auto bytesWritten = writeDataToUut(data_.content.size());
    EXPECT_THAT(bytesWritten, Eq(data_.content.size()));

    uut_->close();

    auto dao = Dao::AttachmentDao::load(
                Dao::IsDraft::No, data_.msgId, data_.index, session_);
    ASSERT_THAT(dao, Not(IsNull()));
    ASSERT_THAT(dao->content(), Eq(data_.content));
}
