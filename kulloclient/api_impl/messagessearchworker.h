/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <boost/optional.hpp>

#include "kulloclient/types.h"
#include "kulloclient/api/MessagesSearchListener.h"
#include "kulloclient/api/SenderPredicate.h"
#include "kulloclient/api_impl/worker.h"
#include "kulloclient/db/dbsession.h"

namespace Kullo {
namespace ApiImpl {

class MessagesSearchWorker : public Worker
{
public:
    MessagesSearchWorker(
            const std::string &searchText,
            id_type convId,
            const boost::optional<Api::SenderPredicate> &sender,
            int32_t limitResults,
            const boost::optional<std::string> &boundary,
            const Db::SessionConfig sessionConfig,
            const std::shared_ptr<Api::MessagesSearchListener> listener);

    void work() override;
    void cancel() override;

private:
    // not synchronized, non-threadsafe stuff is only used from work()
    const std::string searchText_;
    const int64_t convId_;
    const boost::optional<Api::SenderPredicate> sender_;
    const int32_t limitResults_;
    const boost::optional<std::string> boundary_;
    const Db::SessionConfig sessionConfig_;
    std::atomic<bool> canceled_;

    // all uses must be synchronized
    std::shared_ptr<Api::MessagesSearchListener> listener_;
};

}
}
