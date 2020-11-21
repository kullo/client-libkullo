/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <kulloclient/api/InternalEvent.h>
#include <kulloclient/api/Session.h>
#include <kulloclient/api/SessionListener.h>
#include <kulloclient/event/definitions.h>
#include <kulloclient/util/misc.h>

#include <unordered_map>
#include <typeindex>
#include <mutex>

class StubSessionListener : public Kullo::Api::SessionListener
{
public:
    inline void internalEvent(
            const Kullo::nn_shared_ptr<Kullo::Api::InternalEvent> &event) override
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_); K_RAII(lock);

        auto &eventRef = *event;
        internalEventCounts_[std::type_index(typeid(eventRef))]++;

        if (auto session = session_.lock())
        {
            // This does NOT run notify() on main thread, because events
            // would be hard to test in an asynchonous environment
            auto result = session->notify(event);
            externalEvents_.insert(result.begin(), result.end());
        }
    }

    void setSession(std::shared_ptr<Kullo::Api::Session> session) {
        std::lock_guard<std::recursive_mutex> lock(mutex_); K_RAII(lock);
        session_ = session;
    }

    int internalEventCount(std::type_index type) {
        std::lock_guard<std::recursive_mutex> lock(mutex_); K_RAII(lock);
        return internalEventCounts_[type];
    }

    Kullo::Event::ApiEvents externalEvents() {
        std::lock_guard<std::recursive_mutex> lock(mutex_); K_RAII(lock);
        return externalEvents_;
    }

    Kullo::Event::ApiEvents externalEvents(const Kullo::Api::EventType typeFilter) {
        std::lock_guard<std::recursive_mutex> lock(mutex_); K_RAII(lock);

        Kullo::Event::ApiEvents out;
        for (const auto &event : externalEvents_)
        {
            if (event.event == typeFilter) out.insert(event);
        }
        return out;
    }

    void resetEvents() {
        std::lock_guard<std::recursive_mutex> lock(mutex_); K_RAII(lock);
        internalEventCounts_.clear();
        externalEvents_.clear();
    }

private:
    // Those members are accessed from different threads via internalEvent()
    std::recursive_mutex mutex_;
    std::unordered_map<std::type_index, int> internalEventCounts_;
    std::weak_ptr<Kullo::Api::Session> session_;
    Kullo::Event::ApiEvents externalEvents_;
};
