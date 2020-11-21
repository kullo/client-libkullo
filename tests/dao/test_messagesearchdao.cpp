/*
 * Copyright 2013–2020 Kullo GmbH
 *
 * This source code is licensed under the 3-clause BSD license. See LICENSE.txt
 * in the root directory of this source tree for details.
 */
#include <kulloclient/dao/messagesearchdao.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

K_TEST(MessageSearchDaoTokenizer, empty)
{
    ASSERT_THAT(Dao::MessageSearchDao::tokenize(""), IsEmpty());
}

K_TEST(MessageSearchDaoTokenizer, nonTokenCharsOnly)
{
    ASSERT_THAT(Dao::MessageSearchDao::tokenize(" "), IsEmpty());
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("- \" "), IsEmpty());
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("🚵"), IsEmpty());
}

K_TEST(MessageSearchDaoTokenizer, tokenCharsOnly)
{
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a"), ElementsAre("a"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("A"), ElementsAre("A"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("à"), ElementsAre("à"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("À"), ElementsAre("À"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a123"), ElementsAre("a123"));
}

K_TEST(MessageSearchDaoTokenizer, singleTokenCharWithNonTokenChars)
{
    ASSERT_THAT(Dao::MessageSearchDao::tokenize(" a"), ElementsAre("a"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a "), ElementsAre("a"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize(" a "), ElementsAre("a"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("  a"), ElementsAre("a"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a  "), ElementsAre("a"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("  a  "), ElementsAre("a"));
}

K_TEST(MessageSearchDaoTokenizer, multipleTokenCharsWithNonTokenChars)
{
    ASSERT_THAT(Dao::MessageSearchDao::tokenize(" a123"), ElementsAre("a123"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a123 "), ElementsAre("a123"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize(" a123 "), ElementsAre("a123"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("  a123"), ElementsAre("a123"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a123  "), ElementsAre("a123"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("  a123  "), ElementsAre("a123"));
}

K_TEST(MessageSearchDaoTokenizer, multipleTokens)
{
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a b"), ElementsAre("a", "b"));
    ASSERT_THAT(Dao::MessageSearchDao::tokenize("a123 b456"), ElementsAre("a123", "b456"));
}
