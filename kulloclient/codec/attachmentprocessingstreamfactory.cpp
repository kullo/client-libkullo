/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/codec/attachmentprocessingstreamfactory.h"

#include "kulloclient/crypto/hashverifyingfilter.h"
#include "kulloclient/dao/messageattachmentsink.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Codec {

AttachmentProcessingStreamFactory::Stream::Stream(
        std::unique_ptr<Util::Sink> &&sink)
    : stream_(std::move(sink))
{}

Util::FilterChain &AttachmentProcessingStreamFactory::Stream::stream()
{
    return stream_;
}

AttachmentProcessingStreamFactory::AttachmentProcessingStreamFactory(
        id_type msgId)
    : msgId_(msgId)
{}

std::unique_ptr<AttachmentProcessingStreamFactory::Stream>
AttachmentProcessingStreamFactory::makeStream(
        const Db::SharedSessionPtr &session,
        id_type attachmentIndex,
        size_t attachmentSize,
        const std::vector<unsigned char> &expectedHash)
{
    auto sink = make_unique<Dao::MessageAttachmentSink>(
                session, msgId_, attachmentIndex, attachmentSize);
    auto hashVerifier = make_unique<Crypto::HashVerifyingFilter>(expectedHash);

    auto result = make_unique<Stream>(std::move(sink));
    result->stream().pushFilter(std::move(hashVerifier));
    return result;
}

}
}
