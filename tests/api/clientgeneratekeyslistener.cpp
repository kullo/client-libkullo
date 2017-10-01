/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
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
