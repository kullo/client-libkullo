/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/event/event.h>

class MockEvent : public Kullo::Event::Event
{
public:
    MOCK_METHOD1(
            notify,
            Kullo::Event::ApiEvents(const Kullo::Event::EventListeners &));
};
