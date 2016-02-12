/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <boost/iostreams/concepts.hpp>
#include <memory>
#include <smartsqlite/blob.h>
#include "kulloclient/db/dbsession.h"
#include "kulloclient/types.h"

namespace Kullo {
namespace Dao {

struct ClosableSinkTag
        : boost::iostreams::closable_tag
        , boost::iostreams::sink_tag
{};

class MessageAttachmentSink final
{
public:
    using char_type = char;
    using category = ClosableSinkTag;

    MessageAttachmentSink(
            const Db::SharedSessionPtr &session,
            id_type messageId,
            id_type attachmentIndex,
            size_t attachmentSize);

    std::streamsize write(const char* buffer, std::streamsize writeSize);
    void close();

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
