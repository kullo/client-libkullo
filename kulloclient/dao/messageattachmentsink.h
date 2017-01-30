/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <memory>
#include <smartsqlite/blob.h>
#include "kulloclient/db/dbsession.h"
#include "kulloclient/util/filterchain.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Dao {

class MessageAttachmentSink : public Util::Sink
{
public:
    MessageAttachmentSink(
            const Db::SharedSessionPtr &session,
            id_type messageId,
            id_type attachmentIndex,
            size_t attachmentSize);

    void write(const unsigned char *buffer, std::size_t writeSize) override;
    void close() override;

private:
    void openIfNotOpen();

    struct Handle final
    {
        size_t bytesWritten = 0;
        SmartSqlite::Blob blob;

        explicit Handle(SmartSqlite::Blob &&blob) : blob(std::move(blob)) {}
    };

    Db::SharedSessionPtr session_;
    id_type messageId_;
    id_type attachmentIndex_;
    size_t attachmentSize_;
    std::unique_ptr<Handle> handle_;
};

}
}
