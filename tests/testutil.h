/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/util/assert.h>

class TestUtil
{
public:
    static int32_t asyncTimeoutMs()
    {
        return asyncTimeoutMs_;
    }

    static int32_t slowTimeoutMs()
    {
        return slowTimeoutMs_;
    }

    enum WaitAndCheckResult { OK, TIMEOUT, FAILURE };

    static WaitAndCheckResult waitAndCheck(
            std::shared_ptr<Kullo::Api::AsyncTask> task, const bool &flag)
    {
        return doWaitAndCheck(task, flag, asyncTimeoutMs_);
    }

    static WaitAndCheckResult waitAndCheckSlow(
            std::shared_ptr<Kullo::Api::AsyncTask> task, const bool &flag)
    {
        return doWaitAndCheck(task, flag, slowTimeoutMs_);
    }

    static std::string tempPath()
    {
        kulloAssert(!tempPath_.empty());
        return tempPath_;
    }

    static void setTempPath(const std::string &tempPath)
    {
        namespace fs = boost::filesystem;

        if (!fs::is_directory(fs::path(tempPath)))
        {
            throw std::invalid_argument(
                        std::string("DbTest: Couldn't set temp path: '") +
                        tempPath + "' is not a directory.");
        }

        tempPath_ = tempPath;
    }

    static std::string assetPath()
    {
        kulloAssert(!assetPath_.empty());
        return assetPath_;
    }

    static void setAssetPath(const std::string &path)
    {
        namespace fs = boost::filesystem;

        if (!fs::is_directory(fs::path(path)))
        {
            throw std::invalid_argument(
                        "Couldn't set test data path: '" +
                        path + "' is not a directory.");
        }

        assetPath_ = path;
    }

    /// Dependent on time, will most probably change on each invocation
    static std::string tempDbFileName()
    {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(
                    now.time_since_epoch()).count();
        return tempPath() + "/kullotest_" + std::to_string(time)
                + "_" + std::to_string(testCounter_++) + ".sqlite";
    }

private:
    static WaitAndCheckResult doWaitAndCheck(
            std::shared_ptr<Kullo::Api::AsyncTask> task,
            const bool &flag,
            int32_t timeout)
    {
        if (!task->waitForMs(timeout)) return TIMEOUT;
        return (flag) ? OK : FAILURE;
    }

    static const int32_t asyncTimeoutMs_ = 5 * 1000; // 5 sec
    static const int32_t slowTimeoutMs_ = 5 * 60 * 1000;  // 5 min

    static int testCounter_;
    static std::string tempPath_;
    static std::string assetPath_;
};

inline std::ostream& operator<<(
        std::ostream& out, const TestUtil::WaitAndCheckResult& result)
{
    switch (result)
    {
    case TestUtil::OK:      out << "OK";      break;
    case TestUtil::TIMEOUT: out << "TIMEOUT"; break;
    case TestUtil::FAILURE: out << "FAILURE"; break;
    }
    return out;
}
