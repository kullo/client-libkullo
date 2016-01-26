/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>
#include <kulloclient/util/librarylogger.h>

#if KULLO_STATIC_TESTS_LIB
    // must be called to ensure that all tests are pulled into the binary
    void pullInTests();

    // replaces the TEST macro, makes the test being pulled in when pullInTests() is called
    #define K_TEST(testcase, test) \
        int PULL_IN_##testcase##_##test; \
        TEST(testcase, test)

    // replaces the TEST_F macro, makes the test being pulled in when pullInTests() is called
    #define K_TEST_F(testcase, test) \
        int PULL_IN_##testcase##_##test; \
        TEST_F(testcase, test)
#else
    #define K_TEST(testcase, test) TEST(testcase, test)
    #define K_TEST_F(testcase, test) TEST_F(testcase, test)
#endif

class KulloTest : public ::testing::Test
{
public:
    KulloTest() {
        using LogType = Kullo::Util::LibraryLogger::LogType;
        // Reset log function for every test
        // turn off DEBUG and INFO messages
        setupLogFunction({ LogType::Debug,
                           LogType::Info });
    }

    void suppressErrorLogOutput() {
        using LogType = Kullo::Util::LibraryLogger::LogType;
        // turn off DEBUG, INFO and ERROR messages
        setupLogFunction({ LogType::Debug,
                           LogType::Info,
                           LogType::Error });
    }

private:
    void setupLogFunction(const std::vector<Kullo::Util::LibraryLogger::LogType> excludedTypes) {
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
};
