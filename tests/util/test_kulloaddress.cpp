/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <exception>
#include <kulloclient/util/kulloaddress.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class KulloAddress : public KulloTest
{
};

K_TEST_F(KulloAddress, hashes)
{
    EXPECT_THROW(Util::KulloAddress("testkullo.net"), std::invalid_argument);
    EXPECT_NO_THROW(Util::KulloAddress("test#kullo.net"));
    EXPECT_THROW(Util::KulloAddress("test#kullo#net"), std::invalid_argument);
}

K_TEST_F(KulloAddress, forbiddenCharsLocalPart)
{
    EXPECT_THROW(Util::KulloAddress("usr(name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr)name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr<name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr>name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr[name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr]name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr{name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr}name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr|name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr&name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr!name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr?name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr^name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr&name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr%name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr$name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr*name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr+name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr=name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr~name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr`name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr'name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr:name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr;name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr@name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr/name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr\\name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr,name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr\"name#kullo.net"), std::invalid_argument);
}

K_TEST_F(KulloAddress, localPartSeparators)
{
    EXPECT_NO_THROW(Util::KulloAddress("user.name#kullo.net"));
    EXPECT_NO_THROW(Util::KulloAddress("user-name#kullo.net"));
    EXPECT_NO_THROW(Util::KulloAddress("user_name#kullo.net"));

    EXPECT_THROW(Util::KulloAddress(".usr#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("-usr#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("_usr#kullo.net"), std::invalid_argument);

    EXPECT_THROW(Util::KulloAddress("usr.#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr-#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr_#kullo.net"), std::invalid_argument);

    EXPECT_THROW(Util::KulloAddress("usr..name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr--name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr__name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr.-name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr-.name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr-_name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr_-name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr_.name#kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("usr._name#kullo.net"), std::invalid_argument);
}

K_TEST_F(KulloAddress, domain)
{
    EXPECT_THROW(Util::KulloAddress("test#.kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo..net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo.net."), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo"), std::invalid_argument);

    EXPECT_NO_THROW(Util::KulloAddress("test#ku-llo.net"));
    EXPECT_NO_THROW(Util::KulloAddress("test#kullo.net"));
    EXPECT_THROW(Util::KulloAddress("test#-kullo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo-.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#ku--llo.net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo.-net"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo.net-"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("test#kullo.123"), std::invalid_argument);
}

K_TEST_F(KulloAddress, chars)
{
    EXPECT_NO_THROW(Util::KulloAddress("abcdefghijklmnopqrstuvwxyz#kullo.net"));
    EXPECT_NO_THROW(Util::KulloAddress("ABCDEFGHIJKLMNOPQRSTUVWXYZ#kullo.net"));
    EXPECT_NO_THROW(Util::KulloAddress("0123456789#kullo.net"));

    EXPECT_NO_THROW(Util::KulloAddress("test#abcdefghijklmnopqr.stuvwxyz"));
    EXPECT_NO_THROW(Util::KulloAddress("test#ABCDEFGHIJKLMNOPQR.STUVWXYZ"));
    EXPECT_NO_THROW(Util::KulloAddress("test#0123456789.com"));
}

K_TEST_F(KulloAddress, length)
{
    EXPECT_NO_THROW(Util::KulloAddress("a#kullo.net"));
    EXPECT_NO_THROW(Util::KulloAddress("a#b.c"));
    EXPECT_THROW(Util::KulloAddress(""), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("a#"), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress("#kullo.net"), std::invalid_argument);

    std::string maxUsername(64, 'a');
    std::string maxDomainLabel(63, 'b');
    // max. 255 chars, here: 63 + 1 + 63 + 1 + 63 + 1 + 63 = 255
    std::string maxDomain = maxDomainLabel + "." + maxDomainLabel + "." + maxDomainLabel + "." + maxDomainLabel;

    EXPECT_NO_THROW(Util::KulloAddress(maxUsername + "#" + maxDomain));
    EXPECT_THROW(Util::KulloAddress(maxUsername + "a#" + maxDomain), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress(maxUsername + "#b." + maxDomain), std::invalid_argument);
    EXPECT_THROW(Util::KulloAddress(maxUsername + "#b.b" + maxDomainLabel), std::invalid_argument);
}

K_TEST_F(KulloAddress, members)
{
    auto testAddress = std::string("test#kullo.net");
    EXPECT_THAT(Util::KulloAddress(testAddress).toString(), Eq(testAddress));
    EXPECT_THAT(Util::KulloAddress(testAddress).username(), Eq(std::string("test")));
    EXPECT_THAT(Util::KulloAddress(testAddress).domain(), Eq(std::string("kullo.net")));
    EXPECT_THAT(Util::KulloAddress(testAddress).server(), Eq(std::string("kullo.kullo.net")));
}

K_TEST_F(KulloAddress, equals)
{
    EXPECT_THAT(Util::KulloAddress("test#kullo.net"), Eq(Util::KulloAddress("test#kullo.net")));
    EXPECT_THAT(Util::KulloAddress("Test#kullo.net"), Eq(Util::KulloAddress("test#kullo.net")));
    EXPECT_THAT(Util::KulloAddress("test#Kullo.net"), Eq(Util::KulloAddress("test#kullo.net")));
    EXPECT_THAT(Util::KulloAddress("test#Kullo.NET"), Eq(Util::KulloAddress("test#kullo.net")));

    EXPECT_THAT(Util::KulloAddress("test#kullo.com"), Ne(Util::KulloAddress("test#kullo.net")));
    EXPECT_THAT(Util::KulloAddress("testk#ullo.net"), Ne(Util::KulloAddress("test#kullo.net")));
    EXPECT_THAT(Util::KulloAddress("test1#kullo.net"), Ne(Util::KulloAddress("test#kullo.net")));
    EXPECT_THAT(Util::KulloAddress("test#kullo1.net"), Ne(Util::KulloAddress("test#kullo.net")));
}
