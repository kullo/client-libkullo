/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <gmock/gmock.h>

#include <kulloclient/util/misc.h>

class ReservedPrinter : public ::testing::TestEventListener
{
public:
    explicit ReservedPrinter(TestEventListener* listener)
        : TestEventListener()
        , listener_(listener)
    {}

    void OnTestProgramStart(const ::testing::UnitTest& ut) override
    { listener_->OnTestProgramStart(ut); }
    void OnTestIterationStart(const ::testing::UnitTest& ut, int iteration) override
    {
        K_UNUSED(ut);
        K_UNUSED(iteration);
        // Skip stats at the beginning
        // listener_->OnTestIterationStart(ut, iteration);
    }
    void OnEnvironmentsSetUpStart(const ::testing::UnitTest& ut) override
    {
        K_UNUSED(ut);
        // listener_->OnEnvironmentsSetUpStart(ut);
    }
    void OnEnvironmentsSetUpEnd(const ::testing::UnitTest& ut) override
    {
        K_UNUSED(ut);
        // listener_->OnEnvironmentsSetUpEnd(ut);
    }
    void OnTestCaseStart(const ::testing::TestCase& tc) override
    {
        K_UNUSED(tc);
        // listener_->OnTestCaseStart(tc);
    }
    void OnTestStart(const ::testing::TestInfo& ti) override
    {
        K_UNUSED(ti);
        // listener_->OnTestStart(ti);
    }
    void OnTestPartResult(const ::testing::TestPartResult& tpr) override
    {
        if (tpr.failed())
        {
            listener_->OnTestPartResult(tpr);
        }
    }
    void OnTestEnd(const ::testing::TestInfo& ti) override
    {
        if (ti.result()->Failed()) listener_->OnTestEnd(ti);
    }
    void OnTestCaseEnd(const ::testing::TestCase& tc) override
    {
        K_UNUSED(tc);
        // if (tc.Failed()) listener_->OnTestCaseEnd(tc);
    }
    void OnEnvironmentsTearDownStart(const ::testing::UnitTest& ut) override
    {
        K_UNUSED(ut);
        //listener_->OnEnvironmentsTearDownStart(ut);
    }
    void OnEnvironmentsTearDownEnd(const ::testing::UnitTest& ut) override
    {
        K_UNUSED(ut);
        // listener_->OnEnvironmentsTearDownEnd(ut);
    }
    void OnTestIterationEnd(const ::testing::UnitTest& ut, int iteration) override
    {
        // Show stats
        listener_->OnTestIterationEnd(ut, iteration);
    }
    void OnTestProgramEnd(const ::testing::UnitTest& ut) override
    {
        listener_->OnTestProgramEnd(ut);
    }

protected:
    testing::TestEventListener* listener_;
};
