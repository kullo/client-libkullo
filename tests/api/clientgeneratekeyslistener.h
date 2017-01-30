/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#pragma once

#include <kulloclient/api/ClientGenerateKeysListener.h>

class ClientGenerateKeysListener :
        public Kullo::Api::ClientGenerateKeysListener
{
public:
    void progress(int8_t progress) override;
    void finished(const std::shared_ptr<Kullo::Api::Registration> &registration) override;

    bool isFinished_ = false;
    std::shared_ptr<Kullo::Api::Registration> registration_;
};
