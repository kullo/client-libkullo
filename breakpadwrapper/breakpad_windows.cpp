/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "breakpad_windows.h"

#include <codecvt>
#include <iostream>
#include <locale>
#include <sstream>

#include <Knownfolders.h>
#include <Shlobj.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED 1
#include <boost/filesystem/operations.hpp>

namespace BreakpadWrapper {

static std::wstring getAndMakeDumpPath()
{
    PWSTR pathCstr = nullptr;
    HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &pathCstr);
    if (result != S_OK) return std::wstring(L"C:");

    std::wstring path(pathCstr);
    path += L"\\Kullo\\Kullo";
    CoTaskMemFree(pathCstr);

    try
    {
        boost::filesystem::create_directories(path);
    }
    catch (boost::filesystem::filesystem_error)
    {
        path = boost::filesystem::temp_directory_path().wstring();
    }
    return path;
}

BreakpadImpl::BreakpadImpl()
    : eh(getAndMakeDumpPath(), nullptr, &BreakpadImpl::dumpCallbackImpl, &m_context, google_breakpad::ExceptionHandler::HANDLER_ALL)
{
}

BreakpadImpl::~BreakpadImpl()
{
}

void BreakpadImpl::registerCallback(dumpCallback callback, void *context)
{
    m_context.origCallback = callback;
    m_context.origContext = context;
}

void BreakpadImpl::dump()
{
    eh.WriteMinidump();
}

bool BreakpadImpl::dumpCallbackImpl(const wchar_t *dump_path,
                                    const wchar_t *minidump_id,
                                    void *context,
                                    EXCEPTION_POINTERS *exinfo,
                                    MDRawAssertionInfo *assertion,
                                    bool succeeded)
{
    (void)exinfo;
    (void)assertion;

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::stringstream pathStream;
    pathStream << utf8_conv.to_bytes(dump_path) << "\\"
               << utf8_conv.to_bytes(minidump_id) << ".dmp";
    std::string path = pathStream.str();

    std::cerr << std::boolalpha << "Dumped to: " << path << " (success: " << succeeded << ")" << std::endl;

    if (context != nullptr)
    {
        auto callbackContext = static_cast<CallbackContext*>(context);
        if (callbackContext->origCallback != nullptr)
        {
            return callbackContext->origCallback(path.c_str(), callbackContext->origContext, succeeded);
        }
    }
    return succeeded;
}

}
