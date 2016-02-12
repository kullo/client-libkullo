/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
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
            const std::shared_ptr<Kullo::Api::InternalEvent> &event) override
    {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);

        auto &eventRef = *event;
        internalEventCounts_[std::type_index(typeid(eventRef))]++;

        if (auto session = session_.lock())
        {
            auto result = session->notify(event);
            externalEvents_.insert(result.begin(), result.end());
        }
    }

    void setSession(std::shared_ptr<Kullo::Api::Session> session) {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        session_ = session;
    }

    int internalEventCount(std::type_index type) {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        return internalEventCounts_[type];
    }

    Kullo::Event::ApiEvents externalEvents() {
        std::lock_guard<std::mutex> lock(mutex_); K_RAII(lock);
        return externalEvents_;
    }

private:
    // Those members are accessed from different threads via internalEvent()
    std::mutex mutex_;
    std::unordered_map<std::type_index, int> internalEventCounts_;
    std::weak_ptr<Kullo::Api::Session> session_;
    Kullo::Event::ApiEvents externalEvents_;
};
