/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include "tests/api/clientgeneratekeyslistener.h"

void ClientGenerateKeysListener::progress(int8_t progress)
{
    (void)progress;
}

void ClientGenerateKeysListener::finished(const Kullo::nn_shared_ptr<Kullo::Api::Registration> &registration)
{
    isFinished_ = true;
    registration_ = registration;
}
