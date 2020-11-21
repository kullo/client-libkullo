/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#pragma once

#include <kulloclient/api/ClientGenerateKeysListener.h>

class ClientGenerateKeysListener :
        public Kullo::Api::ClientGenerateKeysListener
{
public:
    void progress(int8_t progress) override;
    void finished(const Kullo::nn_shared_ptr<Kullo::Api::Registration> &registration) override;

    bool isFinished_ = false;
    std::shared_ptr<Kullo::Api::Registration> registration_;
};
