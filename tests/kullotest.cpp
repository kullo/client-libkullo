/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "tests/kullotest.h"

KulloTest::KulloTest() {
    using LogType = Kullo::Util::LibraryLogger::LogType;
    // Reset log function for every test
    // turn off DEBUG and INFO messages
    setupLogFunction({ LogType::Debug,
                       LogType::Info });
}

void KulloTest::suppressErrorLogOutput() {
    using LogType = Kullo::Util::LibraryLogger::LogType;
    // turn off DEBUG, INFO and ERROR messages
    setupLogFunction({ LogType::Debug,
                       LogType::Info,
                       LogType::Error });
}

void KulloTest::setupLogFunction(const std::vector<Kullo::Util::LibraryLogger::LogType> excludedTypes) {
    using LibraryLogger = Kullo::Util::LibraryLogger;
    LibraryLogger::setLogFunction(
                [=](
                const std::string &file,
                const int line,
                const std::string &function,
                const LibraryLogger::LogType type,
                const std::string &msg,
                const std::string &thread,
                const std::string &stacktrace)
    {
        for (auto excludedType : excludedTypes)
        {
            if (type == excludedType) return;
        }

        LibraryLogger::defaultLogWrapper(
                    file,
                    line,
                    function,
                    type,
                    msg,
                    thread,
                    stacktrace);
    });
}
