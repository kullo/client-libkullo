/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <kulloclient/codec/attachmentsplittingsink.h>
#include <kulloclient/dao/attachmentdao.h>
#include <kulloclient/db/exceptions.h>
#include <kulloclient/util/filterchain.h>
#include <kulloclient/util/misc.h>

#include "tests/dbtest.h"

using namespace testing;
using namespace Kullo;

namespace {

class FakeAttachmentProcessingStreamFactory
        : public Codec::AttachmentProcessingStreamFactory
{
public:
    class Stream
            : public Codec::AttachmentProcessingStreamFactory::Stream
    {
    public:
        Stream(FakeAttachmentProcessingStreamFactory &parent)
            : Codec::AttachmentProcessingStreamFactory::Stream(makeSink(parent))
            , parent_(parent)
        {}

    protected:
        FakeAttachmentProcessingStreamFactory &parent_;

    private:
        static std::unique_ptr<Util::Sink> makeSink(
                FakeAttachmentProcessingStreamFactory &parent)
        {
            parent.data_.push_back({});
            return make_unique<Util::BackInsertingSink>(parent.data_.back());
        }
    };

    FakeAttachmentProcessingStreamFactory(id_type msgId)
        : Codec::AttachmentProcessingStreamFactory(msgId)
    {}

    std::unique_ptr<Codec::AttachmentProcessingStreamFactory::Stream> makeStream(
            const Db::SharedSessionPtr &session,
            id_type attachmentIndex,
            size_t attachmentSize,
            const std::vector<unsigned char> &expectedHash) override
    {
        K_UNUSED(session);
        K_UNUSED(attachmentIndex);
        K_UNUSED(attachmentSize);
        K_UNUSED(expectedHash);

        return make_unique<Stream>(*this);
    }

    std::vector<std::vector<unsigned char>> data() const
    {
        return data_;
    }

protected:
    std::vector<std::vector<unsigned char>> data_;
};

class AttachmentSplittingSink : public DbTest
{
public:
    AttachmentSplittingSink()
        : streamFactory_(messageId_)
    {}

    void makeUut()
    {
        auto uut = make_unique<Codec::AttachmentSplittingSink>(
                    session_, messageId_, streamFactory_);
        stream_ = make_unique<Util::FilterChain>(std::move(uut));
    }

    void makeAttachment(id_type index, std::size_t size)
    {
        Dao::AttachmentDao att(session_);
        att.setDraft(false);
        att.setMessageId(messageId_);
        att.setIndex(index);
        att.setSize(size);
        att.save();
    }

protected:
    const id_type messageId_ = 23;
    FakeAttachmentProcessingStreamFactory streamFactory_;
    std::unique_ptr<Util::FilterChain> stream_;
};

}

K_TEST_F(AttachmentSplittingSink, worksForZeroAttachments)
{
    std::vector<unsigned char> input;

    makeUut();
    stream_->write(input.data(), input.size());
    stream_->close();

    EXPECT_THAT(streamFactory_.data(), IsEmpty());
}

K_TEST_F(AttachmentSplittingSink, worksForOneEmptyAttachment)
{
    std::vector<unsigned char> input;
    makeAttachment(42, input.size());

    makeUut();
    stream_->write(input.data(), input.size());
    stream_->close();

    ASSERT_THAT(streamFactory_.data(), SizeIs(1));
    EXPECT_THAT(streamFactory_.data()[0], IsEmpty());
}

K_TEST_F(AttachmentSplittingSink, worksForOneAttachment)
{
    std::vector<unsigned char> input = {'H', 'e', 'l', 'l', 'o'};
    makeAttachment(42, input.size());

    makeUut();
    stream_->write(input.data(), input.size());
    stream_->close();

    ASSERT_THAT(streamFactory_.data(), SizeIs(1));
    EXPECT_THAT(streamFactory_.data()[0], Eq(input));
}

K_TEST_F(AttachmentSplittingSink, worksForTwoAttachments)
{
    std::vector<unsigned char> input1 = {'H', 'e', 'l', 'l', 'o'};
    std::vector<unsigned char> input2 = {'w', 'o', 'r', 'l', 'd'};
    makeAttachment(23, input1.size());
    makeAttachment(42, input2.size());

    std::vector<unsigned char> input = input1;
    input.insert(input.end(), input2.cbegin(), input2.cend());

    makeUut();
    stream_->write(input.data(), input.size());
    stream_->close();

    ASSERT_THAT(streamFactory_.data(), SizeIs(2));
    EXPECT_THAT(streamFactory_.data()[0], Eq(input1));
    EXPECT_THAT(streamFactory_.data()[1], Eq(input2));
}

K_TEST_F(AttachmentSplittingSink, worksForTwoAttachmentsInTwoWrites)
{
    std::vector<unsigned char> input1 = {'H', 'e', 'l', 'l', 'o'};
    std::vector<unsigned char> input2 = {'w', 'o', 'r', 'l', 'd'};
    makeAttachment(23, input1.size());
    makeAttachment(42, input2.size());

    makeUut();
    stream_->write(input1.data(), input1.size());
    stream_->write(input2.data(), input2.size());
    stream_->close();

    ASSERT_THAT(streamFactory_.data(), SizeIs(2));
    EXPECT_THAT(streamFactory_.data()[0], Eq(input1));
    EXPECT_THAT(streamFactory_.data()[1], Eq(input2));
}

K_TEST_F(AttachmentSplittingSink, worksForTwoAttachmentsInTwoAsymmetricWrites)
{
    std::vector<unsigned char> input1 = {'H', 'e', 'l', 'l', 'o'};
    std::vector<unsigned char> input2 = {'w', 'o', 'r', 'l', 'd'};
    makeAttachment(23, input1.size());
    makeAttachment(42, input2.size());

    std::vector<unsigned char> write1 = input1;
    write1.push_back(input2[0]);
    std::vector<unsigned char> write2(input2.begin() + 1, input2.end());

    makeUut();
    stream_->write(write1.data(), write1.size());
    stream_->write(write2.data(), write2.size());
    stream_->close();

    ASSERT_THAT(streamFactory_.data(), SizeIs(2));
    EXPECT_THAT(streamFactory_.data()[0], Eq(input1));
    EXPECT_THAT(streamFactory_.data()[1], Eq(input2));
}

K_TEST_F(AttachmentSplittingSink, throwsOnInputTooShort)
{
    std::vector<unsigned char> input = {'H', 'e', 'l', 'l'};
    makeAttachment(42, input.size() + 1);

    makeUut();
    stream_->write(input.data(), input.size());
    EXPECT_THROW(stream_->close(), Db::DatabaseIntegrityError);
}

K_TEST_F(AttachmentSplittingSink, throwsOnInputTooLong)
{
    std::vector<unsigned char> input = {'H', 'e', 'l', 'l', 'o'};
    makeAttachment(42, input.size() - 1);

    makeUut();
    EXPECT_THROW(stream_->write(input.data(), input.size()),
                 Db::DatabaseIntegrityError);
}
