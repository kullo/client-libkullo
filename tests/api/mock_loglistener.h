/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/api/LogListener.h>

class MockLogListener : public Kullo::Api::LogListener
{
public:
    MOCK_METHOD7(writeLogMessage, void(
            const std::string &file,
            int32_t line,
            const std::string &function,
            Kullo::Api::LogType type,
            const std::string &message,
            const std::string &thread,
            const std::string &stacktrace));
};
