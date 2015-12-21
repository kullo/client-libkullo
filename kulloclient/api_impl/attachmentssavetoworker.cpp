/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/attachmentssavetoworker.h"

#include <fstream>
#include <iostream>
#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api_impl/exception_conversion.h"
#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/util/exceptions.h"
#include "kulloclient/util/filesystem.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

AttachmentsSaveToWorker::AttachmentsSaveToWorker(
        bool isDraft, int64_t convOrMsgId, int64_t attId,
        const std::string &path, const std::string &dbPath)
    : isDraft_(isDraft)
    , convOrMsgId_(convOrMsgId)
    , attId_(attId)
    , path_(path)
    , dbPath_(dbPath)
{}

namespace {

template <typename T>
void throwStreamFailureIfFailed(
        const std::basic_ios<T> &stream, const std::string &message)
{
    if (stream.fail())
    {
        throw std::ios_base::failure(message);
    }
}

}

void AttachmentsSaveToWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AttSaveToW");

    try
    {
        {
            auto session = Db::makeSession(dbPath_);
            SmartSqlite::ScopedTransaction tx(session);

            auto isDraft = isDraft_ ? Dao::IsDraft::Yes : Dao::IsDraft::No;
            auto dao = Dao::AttachmentDao::load(
                        isDraft, convOrMsgId_, attId_, session);
            if (dao)
            {
                auto stream = Util::Filesystem::makeOfstream(path_);
                dao->saveContent(*stream);
            }

            tx.commit();
        }

        notifyListener(convOrMsgId_, attId_, path_);
    }
    catch(std::exception &ex)
    {
        Log.e() << "AttachmentsSaveToWorker failed for path '"
                << path_ << "' with message:\n"
                << Util::formatException(ex);

        error(convOrMsgId_, attId_, path_,
              toLocalError(std::current_exception()));
    }
}

}
}
