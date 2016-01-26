/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/attachmentscontentworker.h"

#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

AttachmentsContentWorker::AttachmentsContentWorker(
        bool isDraft, int64_t convOrMsgId, int64_t attId, const std::string &dbPath)
    : isDraft_(isDraft)
    , convOrMsgId_(convOrMsgId)
    , attId_(attId)
    , dbPath_(dbPath)
{}

void AttachmentsContentWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AttContentW");

    std::vector<uint8_t> content;

    // copy attachment from filesystem to DB
    {
        auto session = Db::makeSession(dbPath_);
        SmartSqlite::ScopedTransaction tx(session);
        (void)tx;  // RAII only

        auto isDraft = isDraft_ ? Dao::IsDraft::Yes : Dao::IsDraft::No;
        auto dao = Dao::AttachmentDao::load(
                    isDraft, convOrMsgId_, attId_, session);
        if (dao) content = dao->content();
    }

    notifyListener(convOrMsgId_, attId_, content);
}

}
}
