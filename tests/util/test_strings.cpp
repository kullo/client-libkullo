/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include <exception>

#include <kulloclient/util/strings.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class Strings : public KulloTest
{
};

K_TEST_F(Strings, padLeftWithDefaultChar)
{
    EXPECT_THAT(Util::Strings::padLeft("123", 8), Eq("     123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 5), Eq("  123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 3), Eq("123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 2), Eq("123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 0), Eq("123"));
}

K_TEST_F(Strings, padLeftWithXChar)
{
    EXPECT_THAT(Util::Strings::padLeft("123", 8, 'x'), Eq("xxxxx123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 5, 'x'), Eq("xx123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 3, 'x'), Eq("123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 2, 'x'), Eq("123"));
    EXPECT_THAT(Util::Strings::padLeft("123", 0, 'x'), Eq("123"));
}

K_TEST_F(Strings, padRightWithDefaultChar)
{
    EXPECT_THAT(Util::Strings::padRight("123", 8), Eq("123     "));
    EXPECT_THAT(Util::Strings::padRight("123", 5), Eq("123  "));
    EXPECT_THAT(Util::Strings::padRight("123", 3), Eq("123"));
    EXPECT_THAT(Util::Strings::padRight("123", 2), Eq("123"));
    EXPECT_THAT(Util::Strings::padRight("123", 0), Eq("123"));
}

K_TEST_F(Strings, padRightWithXChar)
{
    EXPECT_THAT(Util::Strings::padRight("123", 8, 'x'), Eq("123xxxxx"));
    EXPECT_THAT(Util::Strings::padRight("123", 5, 'x'), Eq("123xx"));
    EXPECT_THAT(Util::Strings::padRight("123", 3, 'x'), Eq("123"));
    EXPECT_THAT(Util::Strings::padRight("123", 2, 'x'), Eq("123"));
    EXPECT_THAT(Util::Strings::padRight("123", 0, 'x'), Eq("123"));
}

K_TEST_F(Strings, trim)
{
    EXPECT_THAT(Util::Strings::trim(""),
                StrEq(""));
    EXPECT_THAT(Util::Strings::trim("a"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("a "),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim(" a"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim(" a "),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("a\n"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\na"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\na\n"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("a\t"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\ta"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\ta\t"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("a\r"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\ra"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\ra\r"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("\r \na\r \t"),
                StrEq("a"));
    EXPECT_THAT(Util::Strings::trim("a\r \n\r \tb"),
                StrEq("a\r \n\r \tb"));
    EXPECT_THAT(Util::Strings::trim(" a\r \n\r \tb "),
                StrEq("a\r \n\r \tb"));
}

K_TEST_F(Strings, highlightLinksSimple)
{
    // without link
    EXPECT_THAT(Util::Strings::highlightWebLinks("Some minor note without any links whatsoever."),
                Not(HasSubstr("</a>")));

    // with link
    EXPECT_THAT(Util::Strings::highlightWebLinks("Some note with an HTTP link: http://example.com."),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("Some note with an HTTPs link: https://example.com."),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("http://example.com That's a link at the beginning."),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("And this one sits at the end of text: http://example.com"),
                HasSubstr("</a>"));
}

K_TEST_F(Strings, highlightLinksAfterPunktuation)
{
    EXPECT_THAT(Util::Strings::highlightWebLinks("Other text: https://example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("Other text:https://example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("Other text.https://example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("Other text;https://example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("Other text,https://example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("Other text–https://example.com"),
                HasSubstr("</a>"));
}

K_TEST_F(Strings, highlightLinksSchemes)
{
    // ok
    EXPECT_THAT(Util::Strings::highlightWebLinks("http://example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightWebLinks("https://example.com"),
                HasSubstr("</a>"));

    // other schemes
    EXPECT_THAT(Util::Strings::highlightWebLinks("httpx://example.com"),
                Not(HasSubstr("</a>")));
    EXPECT_THAT(Util::Strings::highlightWebLinks("fakehttp://example.com"),
                Not(HasSubstr("</a>")));
}

K_TEST_F(Strings, highlightBorderCases)
{
    auto out = Util::Strings::highlightWebLinks("This link does not contain trailing dot: http://www. ok?");
    EXPECT_THAT(out, HasSubstr("</a>"));
    EXPECT_THAT(out, HasSubstr(R"(href="http://www")"));
}

K_TEST_F(Strings, highlightDifferentHosts)
{
    std::string result;

    // 192.168.178.1
    result = Util::Strings::highlightWebLinks("Link http://192.168.178.1 in text.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://192.168.178.1")"));

    // www.Alliancefrançaise.nu
    result = Util::Strings::highlightWebLinks("Link http://www.xn--alliancefranaise-npb.nu in text.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://www.xn--alliancefranaise-npb.nu")"));

    // fritz.box
    result = Util::Strings::highlightWebLinks("Link http://fritz.box in text.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://fritz.box")"));

    // localhost
    result = Util::Strings::highlightWebLinks("Link http://localhost in text.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://localhost")"));
}

K_TEST_F(Strings, highlightLinksWithPort)
{
    std::string result;

    result = Util::Strings::highlightWebLinks("Some note with an HTTPs link: https://example.com:443/subfolder");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"https://example.com:443/subfolder\""));

    result = Util::Strings::highlightWebLinks("Some note with an HTTP link: http://example.com:8080.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://example.com:8080\""));
}

K_TEST_F(Strings, highlightLinksPunycodeDomains)
{
    std::string result;

    result = Util::Strings::highlightWebLinks("The link http://www.obermühle-hennethal.de/ exists");
    EXPECT_THAT(result, HasSubstr("href=\"http://www.obermühle-hennethal.de/\""));
    EXPECT_THAT(result, HasSubstr("</a> "));

    result = Util::Strings::highlightWebLinks("The link http://www.schräge.de/ exists");
    EXPECT_THAT(result, HasSubstr("href=\"http://www.schräge.de/\""));
    EXPECT_THAT(result, HasSubstr("</a> "));

    result = Util::Strings::highlightWebLinks("The link http://www.börse.de/ exists");
    EXPECT_THAT(result, HasSubstr("href=\"http://www.börse.de/\""));
    EXPECT_THAT(result, HasSubstr("</a> "));

    // works with IDNA 2008 but not IDNA 2003 (use Firefox to confirm)
    // http://idnaconv.net/try-it.html?decoded=stra%C3%9Fe&idn_version=2003&encode=Encode+%3E%3E
    // http://idnaconv.net/try-it.html?decoded=stra%C3%9Fe&idn_version=2008&encode=Encode+%3E%3E
    result = Util::Strings::highlightWebLinks("The link http://straße.de/ exists");
    EXPECT_THAT(result, HasSubstr("href=\"http://straße.de/\""));
    EXPECT_THAT(result, HasSubstr("</a> "));
}

K_TEST_F(Strings, highlightLinksSpecialChars)
{
    std::string result;

    result = Util::Strings::highlightWebLinks("Link https://www.youtube.com/watch?v=5OcTXnLVfFk ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"https://www.youtube.com/watch?v=5OcTXnLVfFk\""));

    result = Util::Strings::highlightWebLinks("Link https://www.google.de/search?client=ubuntu&amp;q=andinlink ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"https://www.google.de/search?client=ubuntu&amp;q=andinlink\""));

    result = Util::Strings::highlightWebLinks("Link https://en.wikipedia.org/wiki/Help:URL#Linking_to_URLs ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"https://en.wikipedia.org/wiki/Help:URL#Linking_to_URLs\""));

    result = Util::Strings::highlightWebLinks("Link https://github.com/kullo?result=2*3 ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"https://github.com/kullo?result=2*3\""));

    result = Util::Strings::highlightWebLinks("Link http://ticketsystem.com/projects/1/report?filters[]=done ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://ticketsystem.com/projects/1/report?filters[]=done\""));

    result = Util::Strings::highlightWebLinks("Link https://www.google.de/maps/place/52%C2%B031%2725.2%22N+13%C2%B022%2706.0%22E/@52.523677,13.368332,173m ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="https://www.google.de/maps/place/52%C2%B031%2725.2%22N+13%C2%B022%2706.0%22E/@52.523677,13.368332,173m")"));

    // no path but query and fragment
    result = Util::Strings::highlightWebLinks("Link https://foo.bar?123;456#home ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="https://foo.bar?123;456#home")"));
}

K_TEST_F(Strings, highlightLinksSpecialCharsPath)
{
    std::string result;

    // percent encoding
    result = Util::Strings::highlightWebLinks("Link http://en.wikipedia.org/wiki/H%26M");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://en.wikipedia.org/wiki/H%26M\""));

    // broken percent encoding
    result = Util::Strings::highlightWebLinks("Link http://en.wikipedia.org/wiki/H%2");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://en.wikipedia.org/wiki/H\""));
    result = Util::Strings::highlightWebLinks("Link http://en.wikipedia.org/wiki/H%_2");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://en.wikipedia.org/wiki/H\""));
    result = Util::Strings::highlightWebLinks("Link http://en.wikipedia.org/wiki/H%xx");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://en.wikipedia.org/wiki/H\""));

    // Chrome does not escape "(" and ")" when user copies a link
    // from the address bar. Firefox does.
    result = Util::Strings::highlightWebLinks("Different meanings of bank http://en.wikipedia.org/wiki/Bank_(disambiguation)");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr("href=\"http://en.wikipedia.org/wiki/Bank_(disambiguation)\""));

    // valid path chars
    for (const auto &character : std::vector<std::string>{".", "-", ",", ";", "~", "_", "+", "*"})
    {
        result = Util::Strings::highlightWebLinks("A link: http://example.com/some" + character + "thing");
        EXPECT_THAT(result, HasSubstr("</a>"));
        EXPECT_THAT(result, HasSubstr("href=\"http://example.com/some" + character + "thing\""));
    }

    // invalid path chars
    for (const auto &character : std::vector<std::string>{"`", "^", ">", "<", "{", "}", "\\"})
    {
        result = Util::Strings::highlightWebLinks("A link: http://example.com/some" + character + "thing");
        EXPECT_THAT(result, HasSubstr("</a>"));
        EXPECT_THAT(result, Not(HasSubstr("href=\"http://example.com/some" + character + "thing\"")));
    }

    // Opera and Safari think it is a good idea, to export unescapted special chars in the path
    // that are not covered by the standard, e.g.
    //   https://de.wikipedia.org/wiki/Vorsätze_für_Maßeinheiten#SI-Pr.C3.A4fixe
    //   https://be.wikipedia.org/wiki/Пі
    //   https://be.wikipedia.org/wiki/Ірацыянальны_лік
    // Users blame Kullo for not highlighting those. Since we do not have full unicode support
    // we whitelist commonly used characters on a case to case basis.
    for (const auto &character : std::vector<std::string>{"ä", "ö", "ü", "Ä", "Ö", "Ü", "ß"})
    {
        result = Util::Strings::highlightWebLinks("A link: http://example.com/some" + character + "thing");
        EXPECT_THAT(result, HasSubstr("</a>"));
        EXPECT_THAT(result, HasSubstr("href=\"http://example.com/some" + character + "thing\""));
    }
}

K_TEST_F(Strings, highlightLinksSpecialCharsFragment)
{
    std::string result;

    // empty fragment
    result = Util::Strings::highlightWebLinks("Link https://foo.bar/123456# ok?");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="https://foo.bar/123456#")"));

    // No special chars in fragment
    // Safari/Opera: https://de.wikipedia.org/wiki/Vorsätze_für_Maßeinheiten#SI-Pr.C3.A4fixe
    for (const auto &character : std::vector<std::string>{"ä", "ö", "ü", "Ä", "Ö", "Ü", "ß"})
    {
        result = Util::Strings::highlightWebLinks("A link: http://example.com/some#" + character + "thing");
        EXPECT_THAT(result, HasSubstr("</a>"));
        EXPECT_THAT(result, HasSubstr("href=\"http://example.com/some#\""));
    }
}

K_TEST_F(Strings, highlightLinkInBrackets)
{
    std::string result;

    result = Util::Strings::highlightWebLinks("Auf der Seite (http://example.com) habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com")"));

    result = Util::Strings::highlightWebLinks("Auf der Seite (http://example.com/dir/root_(unix)) habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"|(href="http://example.com/dir/root_(unix)")|"));

    result = Util::Strings::highlightWebLinks("Auf der Seite (http://example.com/dir/root#page2) habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"|(href="http://example.com/dir/root#page2")|"));

    result = Util::Strings::highlightWebLinks("Auf einigen Seiten (http://example.com zum Beispiel) habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com")"));

    // brackets over punctuation
    result = Util::Strings::highlightWebLinks("Auf der Seite (http://example.com/path).");
    EXPECT_THAT(result, HasSubstr("</a>"));
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com/path")"));
}

K_TEST_F(Strings, highlightLinkBeforePunctuation)
{
    std::string result;

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com, hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com")"));
    EXPECT_THAT(result, HasSubstr("</a>, "));

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com; hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com")"));
    EXPECT_THAT(result, HasSubstr("</a>; "));

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com. Hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com")"));
    EXPECT_THAT(result, HasSubstr("</a>. "));

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com: hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com")"));
    EXPECT_THAT(result, HasSubstr("</a>: "));

    // Link including dot in path

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com/ab.cd, hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com/ab.cd")"));
    EXPECT_THAT(result, HasSubstr("</a>, "));

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com/ab.cd; hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com/ab.cd")"));
    EXPECT_THAT(result, HasSubstr("</a>; "));

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com/ab.cd. Hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com/ab.cd")"));
    EXPECT_THAT(result, HasSubstr("</a>. "));

    result = Util::Strings::highlightWebLinks("Das ist die Seite http://example.com/ab.cd: hier habe ich die Infos gefunden.");
    EXPECT_THAT(result, HasSubstr(R"(href="http://example.com/ab.cd")"));
    EXPECT_THAT(result, HasSubstr("</a>: "));
}

K_TEST_F(Strings, highlightLinksMultiple)
{
    std::string result;

    result = Util::Strings::highlightWebLinks(
                "A text with one link here: http://example.com "
                "and some other stuff here http://example.com");
    EXPECT_THAT(result, HasSubstr("</a>")); // there is a link
    EXPECT_THAT(result.find("</a>"), Ne(result.rfind("</a>"))); // there are more than one links
}

K_TEST_F(Strings, highlightKulloAddressesSimple)
{
    // without link
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Some minor note without any links whatsoever."),
                Not(HasSubstr("</a>")));

    // with link
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Some note with an address bla#example.com in between"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("bla#example.com That's an address at the beginning."),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("And this one sits at the end of text: bla#example.com"),
                HasSubstr("</a>"));

    // with uppercase
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("address: Bla#example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("address: bla#Example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("address: bla#example.Com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("address: BLA#EXAMPLE.COM"),
                HasSubstr("</a>"));
}

K_TEST_F(Strings, highlightKulloAddressesAfterPunktuation)
{
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Other text: test#example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Other text:test#example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Other text;test#example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Other text,test#example.com"),
                HasSubstr("</a>"));
    EXPECT_THAT(Util::Strings::highlightKulloAdresses("Other text–test#example.com"),
                HasSubstr("</a>"));
}

K_TEST_F(Strings, highlightKulloAddressesLinkContent)
{
    {
        auto out = Util::Strings::highlightKulloAdresses("Some note with an address bla#example.com in between");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out, HasSubstr("href=\"kulloInternal:bla#example.com\""));
    }

    {
        auto out = Util::Strings::highlightKulloAdresses("Some note with an address bla#example.com");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out, HasSubstr("href=\"kulloInternal:bla#example.com\""));
    }

    {
        auto out = Util::Strings::highlightKulloAdresses("Some note with an address bla#example.com.");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out, HasSubstr("href=\"kulloInternal:bla#example.com\""));
    }

    {
        auto out = Util::Strings::highlightKulloAdresses("Some note with an (address bla#example.com)");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out, HasSubstr("href=\"kulloInternal:bla#example.com\""));
    }

    {
        auto out = Util::Strings::highlightKulloAdresses("Some note with an address &quot;bla#example.com&quot;");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out, HasSubstr("href=\"kulloInternal:bla#example.com\""));
    }
}

K_TEST_F(Strings, htmlEscapeUnmodified)
{
    EXPECT_THAT(Util::Strings::htmlEscape(""),
                StrEq(""));
    EXPECT_THAT(Util::Strings::htmlEscape("A plain string"),
                StrEq("A plain string"));
    EXPECT_THAT(Util::Strings::htmlEscape("A special char: ö"),
                StrEq("A special char: ö"));
    EXPECT_THAT(Util::Strings::htmlEscape("A nice 'quote'"),
                StrEq("A nice 'quote'"));
    EXPECT_THAT(Util::Strings::htmlEscape("\n"),
                StrEq("\n"));
    EXPECT_THAT(Util::Strings::htmlEscape("A plain\n\tstring"),
                StrEq("A plain\n\tstring"));
}

K_TEST_F(Strings, htmlEscapeModified)
{
    EXPECT_THAT(Util::Strings::htmlEscape("<"),
                StrEq("&lt;"));
    EXPECT_THAT(Util::Strings::htmlEscape(">"),
                StrEq("&gt;"));
    EXPECT_THAT(Util::Strings::htmlEscape("&"),
                StrEq("&amp;"));
    EXPECT_THAT(Util::Strings::htmlEscape("\""),
                StrEq("&quot;"));
    EXPECT_THAT(Util::Strings::htmlEscape(R"(die "<-" Taste über dem Enter)"),
                StrEq("die &quot;&lt;-&quot; Taste über dem Enter"));
    EXPECT_THAT(Util::Strings::htmlEscape(R"(An <a> tag)"),
                StrEq("An &lt;a&gt; tag"));
}

K_TEST_F(Strings, messageTextToHtml)
{
    EXPECT_THAT(Util::Strings::messageTextToHtml(""),
                StrEq(""));
    EXPECT_THAT(Util::Strings::messageTextToHtml("<b>"),
                StrEq("&lt;b&gt;"));
    EXPECT_THAT(Util::Strings::messageTextToHtml("a text with link: http://example.com"),
                StrEq(R"(a text with link: <a href="http://example.com">http://example.com</a>)"));
    EXPECT_THAT(Util::Strings::messageTextToHtml("html char < and link: http://example.com"),
                StrEq(R"(html char &lt; and link: <a href="http://example.com">http://example.com</a>)"));
    EXPECT_THAT(Util::Strings::messageTextToHtml("html char in link: http://example.com/?var=1&var=2"),
                StrEq(R"(html char in link: <a href="http://example.com/?var=1&amp;var=2">http://example.com/?var=1&amp;var=2</a>)"));
    EXPECT_THAT(Util::Strings::messageTextToHtml("html char < breaks link: http://example.com/>"),
                StrEq(R"(html char &lt; breaks link: <a href="http://example.com/">http://example.com/</a>&gt;)"));
}

K_TEST_F(Strings, messageTextToHtmlKulloAddressInWeblink)
{
    {
        auto out = Util::Strings::messageTextToHtml("http://www.example.com/item/jaja;bob#example.com");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out.find("</a>"), Eq(out.rfind("</a>"))); // no more than one link
        EXPECT_THAT(out, HasSubstr("href=\"http://www.example.com/item/jaja;bob#example.com\""));
    }
    {
        auto out = Util::Strings::messageTextToHtml("http://www.example.com/item/jaja;bob#example.com;moreLink");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out.find("</a>"), Eq(out.rfind("</a>"))); // no more than one link
        EXPECT_THAT(out, HasSubstr("href=\"http://www.example.com/item/jaja;bob#example.com;moreLink\""));
    }
    {
        auto out = Util::Strings::messageTextToHtml("begin http://www.example.com/item/jaja;bob#example.com end");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out.find("</a>"), Eq(out.rfind("</a>"))); // no more than one link
        EXPECT_THAT(out, HasSubstr("href=\"http://www.example.com/item/jaja;bob#example.com\""));
    }
    {
        auto out = Util::Strings::messageTextToHtml("begin http://www.example.com/item/jaja;bob#example.com;moreLink end");
        EXPECT_THAT(out, HasSubstr("</a>"));
        EXPECT_THAT(out.find("</a>"), Eq(out.rfind("</a>"))); // no more than one link
        EXPECT_THAT(out, HasSubstr("href=\"http://www.example.com/item/jaja;bob#example.com;moreLink\""));
    }
}

K_TEST_F(Strings, formatIntegerWithCommas)
{
    EXPECT_THAT(Util::Strings::formatReadable(0), Eq("0"));
    EXPECT_THAT(Util::Strings::formatReadable(1), Eq("1"));
    EXPECT_THAT(Util::Strings::formatReadable(10), Eq("10"));
    EXPECT_THAT(Util::Strings::formatReadable(100), Eq("100"));
    EXPECT_THAT(Util::Strings::formatReadable(-1), Eq("-1"));
    EXPECT_THAT(Util::Strings::formatReadable(-10), Eq("-10"));
    EXPECT_THAT(Util::Strings::formatReadable(-100), Eq("-100"));

    EXPECT_THAT(Util::Strings::formatReadable( 1000), Eq("1,000"));
    EXPECT_THAT(Util::Strings::formatReadable(-1000), Eq("-1,000"));

    EXPECT_THAT(Util::Strings::formatReadable( 1000000), Eq("1,000,000"));
    EXPECT_THAT(Util::Strings::formatReadable(-1000000), Eq("-1,000,000"));

    EXPECT_THAT(Util::Strings::formatReadable( 1000000000), Eq("1,000,000,000"));
    EXPECT_THAT(Util::Strings::formatReadable(-1000000000), Eq("-1,000,000,000"));

    EXPECT_THAT(Util::Strings::formatReadable( 22000000), Eq("22,000,000"));
    EXPECT_THAT(Util::Strings::formatReadable(-22000000), Eq("-22,000,000"));

    EXPECT_THAT(Util::Strings::formatReadable( 333000000), Eq("333,000,000"));
    EXPECT_THAT(Util::Strings::formatReadable(-333000000), Eq("-333,000,000"));
}
