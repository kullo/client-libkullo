/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <kulloclient/util/regex.h>

#include "tests/kullotest.h"

using namespace testing;
using namespace Kullo;

class Regex : public KulloTest
{
};

K_TEST_F(Regex, construct)
{
    EXPECT_NO_THROW(Util::Regex("a"));
    EXPECT_NO_THROW(Util::Regex("a*"));
    EXPECT_NO_THROW(Util::Regex("[a-z]*"));
    EXPECT_NO_THROW(Util::Regex("([a-z]|[0-9]|%)*"));
    EXPECT_NO_THROW(Util::Regex("(?:[a-z]|[0-9]|%)*"));

    EXPECT_THROW(Util::Regex("[a"), Util::RegexError);
    EXPECT_THROW(Util::Regex("([a-z]*"), Util::RegexError);
}

K_TEST_F(Regex, match)
{
    {
        Util::Regex re("a");
        EXPECT_THAT(Util::Regex::match("a", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("b", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("aa", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("ab", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("ba", re), Eq(false));
    }

    {
        Util::Regex re("a*");
        EXPECT_THAT(Util::Regex::match("a", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("aa", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("aaaaaaaa", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("b", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("aaabaaa", re), Eq(false));
    }

    {
        Util::Regex re("[a-z]+|-");
        EXPECT_THAT(Util::Regex::match("-", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("--", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("a-", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("-a", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("-a-", re), Eq(false));
        EXPECT_THAT(Util::Regex::match("a", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("ab", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("abc", re), Eq(true));
        EXPECT_THAT(Util::Regex::match("xyz", re), Eq(true));
    }
}

K_TEST_F(Regex, matchWithResult)
{
    {
        Util::Regex re("a");
        std::vector<std::string> matches;

        {
            EXPECT_THAT(Util::Regex::match("a", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(1u));
            EXPECT_THAT(matches[0], StrEq("a"));
        }

        {
            EXPECT_THAT(Util::Regex::match("aa", matches, re), Eq(false));
            EXPECT_THAT(matches.size(), Eq(0u));
        }
    }

    {
        Util::Regex re("([a-z]*)(-)?");
        std::vector<std::string> matches;

        {
            EXPECT_THAT(Util::Regex::match("-", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(3u));
            EXPECT_THAT(matches[0], StrEq("-"));
            EXPECT_THAT(matches[1], StrEq(""));
            EXPECT_THAT(matches[2], StrEq("-"));
        }

        {
            EXPECT_THAT(Util::Regex::match("abc-", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(3u));
            EXPECT_THAT(matches[0], StrEq("abc-"));
            EXPECT_THAT(matches[1], StrEq("abc"));
            EXPECT_THAT(matches[2], StrEq("-"));
        }

        {
            EXPECT_THAT(Util::Regex::match("xyz", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(3u));
            EXPECT_THAT(matches[0], StrEq("xyz"));
            EXPECT_THAT(matches[1], StrEq("xyz"));
            EXPECT_THAT(matches[2], StrEq(""));
        }
    }
}

K_TEST_F(Regex, search)
{
    {
        Util::Regex re("ab");

        // find
        EXPECT_THAT(Util::Regex::search("ab", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("abc", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("cab", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("abab", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("ab ab", re), Eq(true));
        EXPECT_THAT(Util::Regex::search(" ab ab ", re), Eq(true));

        // no find
        EXPECT_THAT(Util::Regex::search("xyz", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("a", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("b", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("ba", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("lorem ipsum", re), Eq(false));
    }

    {
        Util::Regex re("([a-z]+)(-)?");

        // find
        EXPECT_THAT(Util::Regex::search("lorem abc-", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("lorem -abc", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("hahah xyz", re), Eq(true));
        EXPECT_THAT(Util::Regex::search("0151xx-5457", re), Eq(true));

        // no find
        EXPECT_THAT(Util::Regex::search("-", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("_...,", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("015151 5457", re), Eq(false));
        EXPECT_THAT(Util::Regex::search("015151-5457", re), Eq(false));
    }
}

K_TEST_F(Regex, searchWithResult)
{
    {
        Util::Regex re("ab");
        std::vector<std::string> matches;

        {
            EXPECT_THAT(Util::Regex::search("ab", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(1u));
            EXPECT_THAT(matches[0], StrEq("ab"));
        }
        {
            EXPECT_THAT(Util::Regex::search("XabXab", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(1u));
            EXPECT_THAT(matches[0], StrEq("ab"));
        }
    }

    {
        Util::Regex re("a+");
        std::vector<std::string> matches;

        {
            EXPECT_THAT(Util::Regex::search("ab", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(1u));
            EXPECT_THAT(matches[0], StrEq("a"));
        }
        {
            EXPECT_THAT(Util::Regex::search("XabXab", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(1u));
            EXPECT_THAT(matches[0], StrEq("a"));
        }
        {
            EXPECT_THAT(Util::Regex::search("XaaaXaaXa", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(1u));
            EXPECT_THAT(matches[0], StrEq("aaa"));
        }
    }

    {
        Util::Regex re("X(a+)");
        std::vector<std::string> matches;

        {
            EXPECT_THAT(Util::Regex::search("ab", matches, re), Eq(false));
            EXPECT_THAT(matches.size(), Eq(0u));
        }
        {
            EXPECT_THAT(Util::Regex::search("XabXab", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(2u));
            EXPECT_THAT(matches[0], StrEq("Xa"));
            EXPECT_THAT(matches[1], StrEq("a"));
        }
        {
            EXPECT_THAT(Util::Regex::search("XaaaXaaXa", matches, re), Eq(true));
            ASSERT_THAT(matches.size(), Eq(2u));
            EXPECT_THAT(matches[0], StrEq("Xaaa"));
            EXPECT_THAT(matches[1], StrEq("aaa"));
        }
    }
}


K_TEST_F(Regex, matchVsSearch)
{
    // match vs. search
    // Because in match() a pattern only matches full input strings, the results between match() and
    // search() differ.

    {
        //
        // The following test code is ambiguous because of a language bug in regex_match
        // http://cplusplus.github.io/LWG/lwg-defects.html#2273
        //

        //Util::Regex re("(Foo|FooBar)");
        //std::vector<std::string> matches;

        // in = FooBar
        // Util::Regex::search("FooBar", matches, re);  // returns true and matches[0] == "Foo"
        //EXPECT_THAT(Util::Regex::match("FooBar", matches, re), Eq(true));
        //ASSERT_THAT(matches.size(), Eq(1u));
        //EXPECT_THAT(matches[0], StrEq("FooBar"));
        //
        // in = FooBar2
        //// Util::Regex::search("FooBar2", matches, re); // returns true and matches[0] == "Foo"
        //EXPECT_THAT(Util::Regex::match("FooBar2", matches, re), Eq(false));
    }

    {
        //
        // The following test code gives different results in libstdc++ and boost/libc++
        //
        // libstdc++: search "Foo|FooBar" in "FooBar2" = "FooBar"
        // boost/libc++: search "Foo|FooBar" in "FooBar2" = "Foo"
        //

        //Util::Regex re("Foo|FooBar");
        //std::vector<std::string> matches;
        //
        //// in = FooBar
        //// EXPECT_THAT(Util::Regex::match("Foo", matches, re), Eq(true));
        //EXPECT_THAT(Util::Regex::search("Foo", matches, re), Eq(true));
        //ASSERT_THAT(matches.size(), Eq(1u));
        //EXPECT_THAT(matches[0], StrEq("Foo"));
        //
        //// in = FooBar2
        //// EXPECT_THAT(Util::Regex::match("FooBar2", matches, re), Eq(false));
        //EXPECT_THAT(Util::Regex::search("FooBar2", matches, re), Eq(true));
        //ASSERT_THAT(matches.size(), Eq(1u));
        //EXPECT_THAT(matches[0], StrEq("Foo"));
    }
}

K_TEST_F(Regex, replace)
{
    {
        Util::Regex search("[a-z]+|-");

        // one find
        EXPECT_THAT(Util::Regex::replace("a", search, "_"), StrEq("_"));
        EXPECT_THAT(Util::Regex::replace("ab", search, "_"), StrEq("_"));
        EXPECT_THAT(Util::Regex::replace("12 a 34", search, "_"), StrEq("12 _ 34"));
        EXPECT_THAT(Util::Regex::replace("12 acd 34", search, "_"), StrEq("12 _ 34"));

        // multiple finds
        EXPECT_THAT(Util::Regex::replace("a-", search, "_"), StrEq("__"));
        EXPECT_THAT(Util::Regex::replace("-a", search, "_"), StrEq("__"));
        EXPECT_THAT(Util::Regex::replace("abc-", search, "_"), StrEq("__"));
        EXPECT_THAT(Util::Regex::replace("-abc", search, "_"), StrEq("__"));

        // not found
        EXPECT_THAT(Util::Regex::replace("123456", search, "_"), StrEq("123456"));
        EXPECT_THAT(Util::Regex::replace(",.*/", search, "_"), StrEq(",.*/"));
    }
}
