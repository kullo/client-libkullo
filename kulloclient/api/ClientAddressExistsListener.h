// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from client.djinni

#pragma once

namespace Kullo { namespace Api {

enum class NetworkError;
struct Address;

/** Listener used in Client.addressExistsAsync() */
class ClientAddressExistsListener {
public:
    virtual ~ClientAddressExistsListener() {}

    virtual void finished(const Address & address, bool exists) = 0;

    virtual void error(const Address & address, NetworkError error) = 0;
};

} }  // namespace Kullo::Api
