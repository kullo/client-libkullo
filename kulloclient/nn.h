/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/nn/nn.hpp>
#include <kulloclient/util/assert.h>

namespace Kullo {

using namespace dropbox::oxygen;

}

// Takes a pointer of type PT (e.g. raw pointer, std::shared_ptr or std::unique_ptr)
// and returns a non-nullable pointer of type Kullo::nn<PT>.
// If input pointer is null, an assertion error is raised.
#define kulloForcedNn(_e) (([&] (typename std::remove_reference<decltype(_e)>::type p) { \
    kulloAssertMsg(p, #_e " must not be null"); \
    return ::Kullo::nn<typename std::remove_reference<decltype(p)>::type>( \
        ::Kullo::i_promise_i_checked_for_null, std::move(p)); \
    })(_e))
