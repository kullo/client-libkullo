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
    KulloTest();
    void suppressErrorLogOutput();
    void setupLogFunction(const std::vector<Kullo::Util::LibraryLogger::LogType> excludedTypes);
};
