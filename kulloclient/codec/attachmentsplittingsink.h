/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <list>
#include <memory>

#include "kulloclient/codec/attachmentprocessingstreamfactory.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/filterchain.h"

namespace Kullo {
namespace Codec {

class AttachmentSplittingSink : public Util::Sink
{
public:
    AttachmentSplittingSink(
            const Db::SharedSessionPtr &session,
            id_type messageId,
            Codec::AttachmentProcessingStreamFactory &streamFactory);

    void write(const unsigned char *buffer, std::size_t size) override;
    void close() override;

private:
    void makeStream();
    void closeStream();

    Db::SharedSessionPtr session_;
    Codec::AttachmentProcessingStreamFactory &streamFactory_;
    std::list<std::unique_ptr<Dao::AttachmentDao>> attachmentDaos_;

    std::unique_ptr<Codec::AttachmentProcessingStreamFactory::Stream> stream_;
    std::size_t bytesLeftInCurrentStream_ = 0;
};

}
}
