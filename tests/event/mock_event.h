/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
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
