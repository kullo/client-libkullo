/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/api_impl/attachmentscontentworker.h"

#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/dao/attachmentdao.h"
#include "kulloclient/util/librarylogger.h"

namespace Kullo {
namespace ApiImpl {

AttachmentsContentWorker::AttachmentsContentWorker(
        bool isDraft, int64_t convOrMsgId, int64_t attId,
        const Db::SessionConfig &sessionConfig)
    : isDraft_(isDraft)
    , convOrMsgId_(convOrMsgId)
    , attId_(attId)
    , sessionConfig_(sessionConfig)
{}

void AttachmentsContentWorker::work()
{
    Util::LibraryLogger::setCurrentThreadName("AttContentW");

    std::vector<uint8_t> content;

    {
        auto session = Db::makeSession(sessionConfig_);
        SmartSqlite::ScopedTransaction tx(session);

        auto isDraft = isDraft_ ? Dao::IsDraft::Yes : Dao::IsDraft::No;
        auto dao = Dao::AttachmentDao::load(
                    isDraft, convOrMsgId_, attId_, session);
        if (dao) content = dao->content();

        tx.commit();
    }

    notifyListener(convOrMsgId_, attId_, content);
}

}
}
