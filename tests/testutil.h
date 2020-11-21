/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <kulloclient/api/AsyncTask.h>
#include <kulloclient/util/assert.h>
#include <kulloclient/nn.h>

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

    template <class T = Kullo::Api::AsyncTask>
    static WaitAndCheckResult waitAndCheck(
            Kullo::nn_shared_ptr<T> task, const bool &flag)
    {
        return doWaitAndCheck<T>(task, flag, asyncTimeoutMs_);
    }

    template <class T = Kullo::Api::AsyncTask>
    static WaitAndCheckResult waitAndCheckSlow(
            Kullo::nn_shared_ptr<T> task, const bool &flag)
    {
        return doWaitAndCheck<T>(task, flag, slowTimeoutMs_);
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

    /// Returns a new filename with full path on each invocation
    static std::string tempDbFileName()
    {
        const auto now = std::chrono::system_clock::now();
        const auto time = std::chrono::duration_cast<std::chrono::seconds>(
                    now.time_since_epoch()).count();
        return tempPath() + "/kullotest_" + std::to_string(time)
                + "_" + std::to_string(testCounter_++) + ".sqlite";
    }

private:
    template <class T = Kullo::Api::AsyncTask>
    static WaitAndCheckResult doWaitAndCheck(
            Kullo::nn_shared_ptr<T> task,
            const bool &flag,
            int32_t timeout)
    {
        if (!task->waitForMs(timeout)) return TIMEOUT;
        return (flag) ? OK : FAILURE;
    }

    static const int32_t asyncTimeoutMs_ = 5 * 1000; // 5 sec
    static const int32_t slowTimeoutMs_ = 15 * 60 * 1000;  // 15 min

    static std::atomic<int> testCounter_;
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
