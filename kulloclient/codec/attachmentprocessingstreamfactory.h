/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <vector>

#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/filterchain.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Codec {

class AttachmentProcessingStreamFactory
{
public:
    class Stream
    {
    public:
        Stream(std::unique_ptr<Util::Sink> &&sink);
        virtual ~Stream() = default;

        Util::FilterChain &stream();

    protected:
        Util::FilterChain stream_;
    };

    AttachmentProcessingStreamFactory(id_type msgId);
    virtual ~AttachmentProcessingStreamFactory() = default;

    virtual std::unique_ptr<Stream> makeStream(
            const Db::SharedSessionPtr &session,
            id_type attachmentIndex,
            size_t attachmentSize,
            const std::vector<unsigned char> &expectedHash);

protected:
    const id_type msgId_;
};

}
}
