/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "kulloclient/api_impl/messagessearchworker.h"

#include <smartsqlite/scopedtransaction.h>

#include "kulloclient/api/MessagesSearchResult.h"
#include "kulloclient/api_impl/DateTime.h"
#include "kulloclient/dao/messagesearchdao.h"
#include "kulloclient/util/librarylogger.h"
#include "kulloclient/util/multithreading.h"
#include "kulloclient/util/misc.h"
#include "kulloclient/util/regex.h"

namespace Kullo {
namespace ApiImpl {

namespace {

const std::string BOUNDARY_CHARS = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const std::string::size_type BOUNDARY_LENGTH = 15; // approx 2^90 possibilities

std::string randomBoundary()
{
    std::string result(BOUNDARY_LENGTH, 'x');
    for (auto iter = result.begin(); iter != result.end(); ++iter)
    {
        // Yes, this is using rand instead of a cryptographically secure RNG.
        // Additionally, modular division doesn't lead to a perfectly uniform
        // distribution. However, this is good enough for our use case.
        *iter = BOUNDARY_CHARS.at(std::rand() % BOUNDARY_CHARS.size());
    }
    return result;
}

const Util::Regex WHTESPACE_MATCHER("\\s+");

std::string compressText(const std::string &in) {
    return Util::Regex::replace(in, WHTESPACE_MATCHER, " ");
}

}

MessagesSearchWorker::MessagesSearchWorker(
        const std::string &searchText,
        id_type convId,
        const boost::optional<Api::SenderPredicate> &sender,
        int32_t limitResults,
        const boost::optional<std::string> &boundary,
        const Db::SessionConfig sessionConfig,
        const std::shared_ptr<Api::MessagesSearchListener> listener)
    : searchText_(searchText)
    , convId_(convId)
    , sender_(sender)
    , limitResults_(limitResults)
    , boundary_(boundary)
    , sessionConfig_(sessionConfig)
    , canceled_(false)
    , listener_(listener)
{}

void MessagesSearchWorker::work()
{
    if (canceled_) { return; }

    Util::LibraryLogger::setCurrentThreadName("AttContentW");

    std::string boundary = boundary_ ? *boundary_ : randomBoundary();

    std::vector<Api::MessagesSearchResult> results;
    {
        auto session = Db::makeSession(sessionConfig_);
        auto dbResults = Dao::MessageSearchDao::search(
                    searchText_, convId_, sender_, limitResults_, boundary, session);
        while (auto &&result = dbResults->next())
        {
            results.emplace_back(
                        result->msgId,
                        result->convId,
                        result->senderAddress,
                        *Api::DateTime::fromRfc3339(result->dateReceived),
                        compressText(result->snippet),
                        boundary
                        );
        }
    }

    if (auto listener = Util::copyGuardedByMutex(listener_, mutex_)) {
        listener->finished(results);
    }
}

void MessagesSearchWorker::cancel()
{
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        listener_.reset();
    }
    canceled_ = true;
}

}
}
