/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <memory>

#include <kulloclient/nn.h>
#include <kulloclient/util/misc.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class NonNullable : public KulloTest
{
};

K_TEST_F(NonNullable, makeNonNulls)
{
    auto nn_shared_pointer = nn_make_shared<std::string>("abab");
    EXPECT_THAT(*nn_shared_pointer, StrEq("abab"));

    auto nn_unique_pointer = nn_make_unique<std::string>("CDCD");
    EXPECT_THAT(*nn_unique_pointer, StrEq("CDCD"));
}

K_TEST_F(NonNullable, convertFromNullable)
{
    auto shared_pointer = std::make_shared<std::string>("abab");
    auto nn_shared_pointer = kulloForcedNn(shared_pointer);
    EXPECT_THAT(*nn_shared_pointer, StrEq("abab"));

    auto unique_pointer = Kullo::make_unique<std::string>("CDCD");
    auto nn_unique_pointer = kulloForcedNn(std::move(unique_pointer));
    EXPECT_THAT(*nn_unique_pointer, StrEq("CDCD"));
}

K_TEST_F(NonNullable, convertFromNull)
{
    std::shared_ptr<std::string> shared_pointer;
    EXPECT_THROW(kulloForcedNn(shared_pointer), Util::AssertionFailed);

    std::unique_ptr<std::string> unique_pointer;
    EXPECT_THROW(kulloForcedNn(std::move(unique_pointer)), Util::AssertionFailed);

    std::string* raw_pointer = nullptr;
    EXPECT_THROW(kulloForcedNn(raw_pointer), Util::AssertionFailed);
}

K_TEST_F(NonNullable, expressionEvaluatedOnlyOnce)
{
    static int evaluationCounter = 0;
    auto makeTestPointer = []() { evaluationCounter++; return std::make_shared<std::string>("A"); };

    EXPECT_THAT(evaluationCounter, Eq(0));
    evaluationCounter++;
    EXPECT_THAT(evaluationCounter, Eq(1));
    makeTestPointer();
    EXPECT_THAT(evaluationCounter, Eq(2));

    auto nn1 = kulloForcedNn(makeTestPointer());
    EXPECT_THAT(*nn1, Eq("A"));

    EXPECT_THAT(evaluationCounter, Eq(3));

    auto nn2 = kulloForcedNn(makeTestPointer());
    auto nn3 = kulloForcedNn(makeTestPointer());
    EXPECT_THAT(nn2, Ne(nn3));

    EXPECT_THAT(evaluationCounter, Eq(5));
}
