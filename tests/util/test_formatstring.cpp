/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include <exception>

#include <kulloclient/util/formatstring.h>

#include "tests/kullotest.h"

using namespace Kullo;
using namespace testing;

class FormatString : public KulloTest
{
};

K_TEST_F(FormatString, padLeftWithDefaultChar)
{
    EXPECT_THAT(Util::FormatString::padLeft("123", 8), Eq("     123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 5), Eq("  123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 3), Eq("123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 2), Eq("123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 0), Eq("123"));
}

K_TEST_F(FormatString, padLeftWithXChar)
{
    EXPECT_THAT(Util::FormatString::padLeft("123", 8, 'x'), Eq("xxxxx123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 5, 'x'), Eq("xx123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 3, 'x'), Eq("123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 2, 'x'), Eq("123"));
    EXPECT_THAT(Util::FormatString::padLeft("123", 0, 'x'), Eq("123"));
}

K_TEST_F(FormatString, padRightWithDefaultChar)
{
    EXPECT_THAT(Util::FormatString::padRight("123", 8), Eq("123     "));
    EXPECT_THAT(Util::FormatString::padRight("123", 5), Eq("123  "));
    EXPECT_THAT(Util::FormatString::padRight("123", 3), Eq("123"));
    EXPECT_THAT(Util::FormatString::padRight("123", 2), Eq("123"));
    EXPECT_THAT(Util::FormatString::padRight("123", 0), Eq("123"));
}

K_TEST_F(FormatString, padRightWithXChar)
{
    EXPECT_THAT(Util::FormatString::padRight("123", 8, 'x'), Eq("123xxxxx"));
    EXPECT_THAT(Util::FormatString::padRight("123", 5, 'x'), Eq("123xx"));
    EXPECT_THAT(Util::FormatString::padRight("123", 3, 'x'), Eq("123"));
    EXPECT_THAT(Util::FormatString::padRight("123", 2, 'x'), Eq("123"));
    EXPECT_THAT(Util::FormatString::padRight("123", 0, 'x'), Eq("123"));
}

K_TEST_F(FormatString, trim)
{
    std::string str;

    str = "";
    Util::FormatString::trim(str);
    EXPECT_EQ("", str);

    str = "a";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "a ";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = " a";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = " a ";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "a\n";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\na";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\na\n";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "a\t";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\ta";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\ta\t";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "a\r";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\ra";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\ra\r";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "\r \na\r \t";
    Util::FormatString::trim(str);
    EXPECT_EQ("a", str);

    str = "a\r \n\r \tb";
    Util::FormatString::trim(str);
    EXPECT_EQ("a\r \n\r \tb", str);

    str = " a\r \n\r \tb ";
    Util::FormatString::trim(str);
    EXPECT_EQ("a\r \n\r \tb", str);
}

K_TEST_F(FormatString, highlightLinksSimple)
{
    std::string str;

    str = "Some minor note without any links whatsoever.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, Not(HasSubstr("</a>")));

    str = "Some note with an HTTP link: http://example.com.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));

    str = "Some note with an HTTPs link: https://example.com.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));

    str = "http://example.com That's a link at the beginning.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));

    str = "And this one sits at the end of text: http://example.com";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
}

K_TEST_F(FormatString, highlightBorderCases)
{
    std::string str;

    str = "This link does not contain trailing dot: http://www. ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://www")"));
}

K_TEST_F(FormatString, highlightDifferentHosts)
{
    std::string str;

    // 192.168.178.1
    str = "Link http://192.168.178.1 in text.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://192.168.178.1")"));

    // www.Alliancefrançaise.nu
    str = "Link http://www.xn--alliancefranaise-npb.nu in text.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://www.xn--alliancefranaise-npb.nu")"));

    // fritz.box
    str = "Link http://fritz.box in text.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://fritz.box")"));

    // localhost
    str = "Link http://localhost in text.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://localhost")"));
}

K_TEST_F(FormatString, highlightLinksWithPort)
{
    std::string str;

    str = "Some note with an HTTPs link: https://example.com:443/subfolder";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"https://example.com:443/subfolder\""));

    str = "Some note with an HTTP link: http://example.com:8080.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"http://example.com:8080\""));
}

K_TEST_F(FormatString, highlightLinksSpecialChars)
{
    std::string str;

    str = "Link https://www.youtube.com/watch?v=5OcTXnLVfFk ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"https://www.youtube.com/watch?v=5OcTXnLVfFk\""));

    str = "Link https://www.google.de/search?client=ubuntu&amp;q=andinlink ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"https://www.google.de/search?client=ubuntu&amp;q=andinlink\""));

    str = "Link https://en.wikipedia.org/wiki/Help:URL#Linking_to_URLs ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"https://en.wikipedia.org/wiki/Help:URL#Linking_to_URLs\""));

    str = "Link https://github.com/kullo?result=2*3 ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"https://github.com/kullo?result=2*3\""));

    // percent encoding
    str = "Link http://en.wikipedia.org/wiki/H%26M";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"http://en.wikipedia.org/wiki/H%26M\""));

    // Chrome does not escape "(" and ")" when user copies a link
    // from the address bar. Firefox does.
    str = "Different meanings of bank http://en.wikipedia.org/wiki/Bank_(disambiguation)";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"http://en.wikipedia.org/wiki/Bank_(disambiguation)\""));

    str = "Link http://ticketsystem.com/projects/1/report?filters[]=done ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr("href=\"http://ticketsystem.com/projects/1/report?filters[]=done\""));

    str = "Link https://www.google.de/maps/place/52%C2%B031%2725.2%22N+13%C2%B022%2706.0%22E/@52.523677,13.368332,173m ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="https://www.google.de/maps/place/52%C2%B031%2725.2%22N+13%C2%B022%2706.0%22E/@52.523677,13.368332,173m")"));

    // ; in path, empty fragment
    str = "Link https://foo.bar/123;456# ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="https://foo.bar/123;456#")"));

    // no path but query and fragment
    str = "Link https://foo.bar?123;456#home ok?";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="https://foo.bar?123;456#home")"));
}

K_TEST_F(FormatString, highlightLinkInBrackets)
{
    std::string str;

    str = "Auf der Seite (http://example.com) habe ich die Infos gefunden.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://example.com")"));

    str = "Auf der Seite (http://example.com/dir/root_(unix)) habe ich die Infos gefunden.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"|(href="http://example.com/dir/root_(unix)")|"));

    str = "Auf einigen Seiten (http://example.com zum Beispiel) habe ich die Infos gefunden.";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>"));
    EXPECT_THAT(str, HasSubstr(R"(href="http://example.com")"));
}

K_TEST_F(FormatString, highlightLinksComplex)
{
    std::string str;

    str = "A text with one link here: http://example.com "
         "and some other stuff here http://example.com";
    Util::FormatString::highlightLinks(str);
    EXPECT_THAT(str, HasSubstr("</a>")); // there is a link
    EXPECT_THAT(str.find("</a>"), Ne(str.rfind("</a>"))); // there are more than one links
}

K_TEST_F(FormatString, escapeMessageTextUnmodified)
{
    std::string str;

    str = "";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("", str);

    str = "A plain string";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("A plain string", str);

    str = "A special char: ö";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("A special char: ö", str);

    str = "A nice 'quote'";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("A nice 'quote'", str);
}

K_TEST_F(FormatString, escapeMessageTextModified)
{
    std::string str;

    str = "<";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("&lt;", str);

    str = ">";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("&gt;", str);

    str = "&";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("&amp;", str);

    str = "\"";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("&quot;", str);

    str = "\n";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("\n", str);

    str = "A plain\n\tstring";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("A plain\n\tstring", str);

    str = R"(die "<-" Taste über dem Enter)";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("die &quot;&lt;-&quot; Taste über dem Enter", str);

    str = R"(An <a> tag)";
    Util::FormatString::escapeMessageText(str);
    EXPECT_EQ("An &lt;a&gt; tag", str);
}

K_TEST_F(FormatString, messageTextToHtml)
{
    std::string str;

    str = "";
    Util::FormatString::messageTextToHtml(str);
    EXPECT_EQ("", str);

    str = "<b>";
    Util::FormatString::messageTextToHtml(str);
    EXPECT_EQ("&lt;b&gt;", str);

    str = "a text with link: http://example.com";
    Util::FormatString::messageTextToHtml(str);
    EXPECT_EQ(R"(a text with link: <a href="http://example.com">http://example.com</a>)", str);

    str = "html char < and link: http://example.com";
    Util::FormatString::messageTextToHtml(str);
    EXPECT_EQ(R"(html char &lt; and link: <a href="http://example.com">http://example.com</a>)", str);

    str = "html char in link: http://example.com/?var=1&var=2";
    Util::FormatString::messageTextToHtml(str);
    EXPECT_EQ(R"(html char in link: <a href="http://example.com/?var=1&amp;var=2">http://example.com/?var=1&amp;var=2</a>)", str);

    str = "html char < breaks link: http://example.com/>";
    Util::FormatString::messageTextToHtml(str);
    EXPECT_EQ(R"(html char &lt; breaks link: <a href="http://example.com/">http://example.com/</a>&gt;)", str);
}

K_TEST_F(FormatString, formatIntegerWithCommas)
{
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(0), Eq("0"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(1), Eq("1"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(10), Eq("10"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(100), Eq("100"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-1), Eq("-1"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-10), Eq("-10"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-100), Eq("-100"));

    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas( 1000), Eq("1,000"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-1000), Eq("-1,000"));

    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas( 1000000), Eq("1,000,000"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-1000000), Eq("-1,000,000"));

    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas( 1000000000), Eq("1,000,000,000"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-1000000000), Eq("-1,000,000,000"));

    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas( 22000000), Eq("22,000,000"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-22000000), Eq("-22,000,000"));

    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas( 333000000), Eq("333,000,000"));
    EXPECT_THAT(Util::FormatString::formatIntegerWithCommas(-333000000), Eq("-333,000,000"));
}
