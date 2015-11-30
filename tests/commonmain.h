/* Copyright 2013–2015 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdlib>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include <gmock/gmock.h>
#include <kulloclient/util/librarylogger.h>

#include "tests/testutil.h"
#include "tests/reservedprinter.h"

namespace {

namespace fs = boost::filesystem;
namespace po = boost::program_options;

std::string getExeDir(const char *arg0)
{
    return fs::system_complete(arg0).parent_path().string();
}

std::string getUsername()
{
    const char *username =
    #ifdef _WIN32
            std::getenv("USERNAME");
    #else
            std::getenv("USER");
    #endif
    return (username) ? username : "unknown_user";
}

std::string getDefaultTempPath()
{
    #ifdef __ANDROID__
        return fs::path("/data/local/tmp").string();
    #else
        fs::path result = fs::temp_directory_path();

        #ifdef __linux
            fs::path(tmpfs) = "/run/shm";
            auto status = fs::status(tmpfs);
            if ((status.type() == fs::directory_file)
                    && (status.permissions() & fs::others_write))
            {
                result = tmpfs;
            }
        #endif

        result /= getUsername();

        return result.string();
    #endif
}

}

enum struct NeedAssets { Yes, No };

inline int commonMain(
        const std::string &reponame, NeedAssets needAssets,
        int argc, char** argv)
{
    Kullo::Util::LibraryLogger::setCurrentThreadName("main");

    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "show this help")
            ("verbose", "show succeeding tests")
            ("temp",
             po::value<std::string>()->default_value(getDefaultTempPath()),
             "set path for temporary files");
    if (needAssets == NeedAssets::Yes)
    {
        desc.add_options()
                ("assets",
                 po::value<std::string>()->default_value(
                     getExeDir(argv[0]) + "/../../" + reponame + "/tests/assets"),
                 "set path for test assets");
    }
    po::variables_map varMap;
    bool parsingFailed = false;
    try
    {
        auto parsed = po::command_line_parser(argc, argv)
                .options(desc)
                .allow_unregistered()
                .run();
        po::store(parsed, varMap);
        po::notify(varMap);
    }
    catch (boost::exception&)
    {
        parsingFailed = true;
    }

    if (parsingFailed || varMap.count("help"))
    {
        // print our help
        std::cout << desc << "\n";

        // print gtest/gmock help
        int fakeArgc = 2;
        char programName[] = "";
        char argument[] = "--help";
        char *fakeArgv[] = {programName, argument};
        ::testing::InitGoogleMock(&fakeArgc, fakeArgv);

        return 1;
    }

    fs::path assetPath;
    if (needAssets == NeedAssets::Yes)
    {
        assetPath = fs::path(varMap["assets"].as<std::string>());
    }
    auto tempPath = fs::path(varMap["temp"].as<std::string>()) / reponame;

    // Create our temp directory if it doesn't exist
    fs::create_directories(tempPath);

    // Clear our temp directory, so that we get a clean slate for each run and
    // don't fill up our temp directory with thousands of DB files.
    for (fs::directory_iterator it(tempPath), end; it != end; ++it)
    {
        fs::remove_all(*it);
    }

    TestUtil::setTempPath(tempPath.string());
    std::cout << "Temp path: " << TestUtil::tempPath() << std::endl;
    if (needAssets == NeedAssets::Yes)
    {
        TestUtil::setAssetPath(assetPath.string());
        std::cout << "Asset path: " << TestUtil::assetPath() << std::endl;
    }

    ::testing::InitGoogleMock(&argc, argv);

    if (!varMap.count("verbose"))
    {
        ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
        ::testing::TestEventListener* defaultListener = listeners.Release(listeners.default_result_printer());
        listeners.Append(new ReservedPrinter(defaultListener));
    }

    return RUN_ALL_TESTS();
}
