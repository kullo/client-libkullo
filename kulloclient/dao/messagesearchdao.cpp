/* Copyright 2013–2017 Kullo GmbH. All rights reserved. */
#include "kulloclient/dao/messagesearchdao.h"

#include <boost/algorithm/string/replace.hpp>

#include "kulloclient/util/assert.h"

namespace Kullo {
namespace Dao {

namespace {
unsigned int utf8Read(const unsigned char **pz);
bool unicodeIsalnum(unsigned int c);
bool unicodeIsdiacritic(unsigned int c);
}

std::unique_ptr<MessageSearchResult> MessageSearchDao::search(
        const std::string &searchText,
        id_type convId,
        const boost::optional<Api::SenderPredicate> &sender,
        int32_t limitResults,
        const std::string &boundary,
        Db::SharedSessionPtr session)
{
    kulloAssert(session);

    const std::string startTag = "(" + boundary + ")";
    const std::string endTag = "(/" + boundary + ")";

    std::string sql =
            "SELECT rowid AS message_id, conversation_id, sender, received, "
            "snippet(messages_fts, 4, :start_tag, :end_tag, '[…]', 20) AS snippet "
            "FROM messages_fts "
            "WHERE messages_fts MATCH :matcher ";

    if (convId >= 0)
    {
        sql += "AND conversation_id = :conv_id ";
    }

    if (sender)
    {
        std::string op;
        switch (sender->predicateOperator) {
        case Api::SearchPredicateOperator::Is:
            op = "=";
            break;
        case Api::SearchPredicateOperator::IsNot:
            op = "!=";
            break;
        }
        sql += "AND sender " + op + " :sender_address ";
    }

    // sorting by message_id is equivalent to received but much faster for big result sets
    sql += "ORDER BY message_id DESC LIMIT :limit ";

    auto stmt = session->prepare(sql);

    // foo bar -> "foo" * AND "bar" *
    std::string matcher;
    bool firstToken = true;
    auto tokens = tokenize(searchText);
    if (tokens.empty())
    {
        matcher = "\"\"";
    }
    else
    {
        for (const auto &token : tokens)
        {
            if (firstToken)
            {
                firstToken = false;
            }
            else
            {
                matcher += " AND ";
            }
            matcher += "\"" + token + "\" *";
        }
    }

    stmt.bind(":start_tag", startTag);
    stmt.bind(":end_tag", endTag);
    stmt.bind(":matcher", matcher);
    stmt.bind(":limit", limitResults);

    if (convId > 0)
    {
        stmt.bind(":conv_id", convId);
    }
    if (sender)
    {
        stmt.bind(":sender_address", sender->senderAddress);
    }

    return make_unique<MessageSearchResult>(std::move(stmt), session);
}

std::unique_ptr<MessageSearchDaoRow> MessageSearchDao::loadFromDb(
        const SmartSqlite::Row &row, Db::SharedSessionPtr session)
{
    auto out = Kullo::make_unique<MessageSearchDaoRow>();
    out->msgId = row.get<id_type>("message_id");
    out->convId = row.get<id_type>("conversation_id");
    out->senderAddress = row.get<std::string>("sender");
    out->dateReceived = row.get<std::string>("received");
    out->snippet = row.get<std::string>("snippet");
    return out;
}

std::vector<std::string> MessageSearchDao::tokenize(const std::string &searchText)
{
    std::vector<std::string> tokens;

    auto input = reinterpret_cast<const unsigned char*>(searchText.c_str());
    unsigned int codePoint;
    const char *tokenBegin = nullptr;
    const char *tokenEnd = nullptr;

    while (true)
    {
        // skip over non-token chars to get start of token
        do
        {
            tokenBegin = reinterpret_cast<const char*>(input);
            if (*input == 0)
            {
                return tokens;
            }
            codePoint = utf8Read(&input);
        }
        while (!(unicodeIsalnum(codePoint) || unicodeIsdiacritic(codePoint)));

        // skip over token chars to get end of token
        do
        {
            tokenEnd = reinterpret_cast<const char*>(input);
            if (*input == 0)
            {
                tokens.emplace_back(tokenBegin, tokenEnd - tokenBegin);
                return tokens;
            }
            codePoint = utf8Read(&input);
        }
        while (unicodeIsalnum(codePoint) || unicodeIsdiacritic(codePoint));

        tokens.emplace_back(tokenBegin, tokenEnd - tokenBegin);
    }
}

namespace {

// START code borrowed from SQLite3 and slightly modified

/*
** This lookup table is used to help decode the first byte of
** a multi-byte UTF8 character.
*/
const unsigned char utf8Trans1[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

/*
** Translate a single UTF-8 character.  Return the unicode value.
**
** For this routine, we assume the UTF8 string is always zero-terminated.
**
** Write a pointer to the next unread byte back into *pzNext.
**
** Notes On Invalid UTF-8:
**
**  *  This routine never allows a 7-bit character (0x00 through 0x7f) to
**     be encoded as a multi-byte character.  Any multi-byte character that
**     attempts to encode a value between 0x00 and 0x7f is rendered as 0xfffd.
**
**  *  This routine never allows a UTF16 surrogate value to be encoded.
**     If a multi-byte character attempts to encode a value between
**     0xd800 and 0xe000 then it is rendered as 0xfffd.
**
**  *  Bytes in the range of 0x80 through 0xbf which occur as the first
**     byte of a character are interpreted as single-byte characters
**     and rendered as themselves even though they are technically
**     invalid characters.
**
**  *  This routine accepts over-length UTF8 encodings
**     for unicode values 0x80 and greater.  It does not change over-length
**     encodings to 0xfffd as some systems recommend.
*/
unsigned int utf8Read(
  const unsigned char **pz    /* Pointer to string from which to read char */
){
  unsigned int c;

  c = *((*pz)++);
  if( c>=0xc0 ){
    c = utf8Trans1[c-0xc0];
    while( (*(*pz) & 0xc0)==0x80 ){
      c = (c<<6) + (0x3f & *((*pz)++));
    }
    if( c<0x80
        || (c&0xFFFFF800)==0xD800
        || (c&0xFFFFFFFE)==0xFFFE ){  c = 0xFFFD; }
  }
  return c;
}

/*
** Return true if the argument corresponds to a unicode codepoint
** classified as either a letter or a number. Otherwise false.
**
** The results are undefined if the value passed to this function
** is less than zero.
*/
bool unicodeIsalnum(unsigned int c){
  /* Each unsigned integer in the following array corresponds to a contiguous
  ** range of unicode codepoints that are not either letters or numbers (i.e.
  ** codepoints for which this function should return 0).
  **
  ** The most significant 22 bits in each 32-bit value contain the first
  ** codepoint in the range. The least significant 10 bits are used to store
  ** the size of the range (always at least 1). In other words, the value
  ** ((C<<22) + N) represents a range of N codepoints starting with codepoint
  ** C. It is not possible to represent a range larger than 1023 codepoints
  ** using this format.
  */
  static const unsigned int aEntry[] = {
    0x00000030, 0x0000E807, 0x00016C06, 0x0001EC2F, 0x0002AC07,
    0x0002D001, 0x0002D803, 0x0002EC01, 0x0002FC01, 0x00035C01,
    0x0003DC01, 0x000B0804, 0x000B480E, 0x000B9407, 0x000BB401,
    0x000BBC81, 0x000DD401, 0x000DF801, 0x000E1002, 0x000E1C01,
    0x000FD801, 0x00120808, 0x00156806, 0x00162402, 0x00163C01,
    0x00164437, 0x0017CC02, 0x00180005, 0x00181816, 0x00187802,
    0x00192C15, 0x0019A804, 0x0019C001, 0x001B5001, 0x001B580F,
    0x001B9C07, 0x001BF402, 0x001C000E, 0x001C3C01, 0x001C4401,
    0x001CC01B, 0x001E980B, 0x001FAC09, 0x001FD804, 0x00205804,
    0x00206C09, 0x00209403, 0x0020A405, 0x0020C00F, 0x00216403,
    0x00217801, 0x0023901B, 0x00240004, 0x0024E803, 0x0024F812,
    0x00254407, 0x00258804, 0x0025C001, 0x00260403, 0x0026F001,
    0x0026F807, 0x00271C02, 0x00272C03, 0x00275C01, 0x00278802,
    0x0027C802, 0x0027E802, 0x00280403, 0x0028F001, 0x0028F805,
    0x00291C02, 0x00292C03, 0x00294401, 0x0029C002, 0x0029D401,
    0x002A0403, 0x002AF001, 0x002AF808, 0x002B1C03, 0x002B2C03,
    0x002B8802, 0x002BC002, 0x002C0403, 0x002CF001, 0x002CF807,
    0x002D1C02, 0x002D2C03, 0x002D5802, 0x002D8802, 0x002DC001,
    0x002E0801, 0x002EF805, 0x002F1803, 0x002F2804, 0x002F5C01,
    0x002FCC08, 0x00300403, 0x0030F807, 0x00311803, 0x00312804,
    0x00315402, 0x00318802, 0x0031FC01, 0x00320802, 0x0032F001,
    0x0032F807, 0x00331803, 0x00332804, 0x00335402, 0x00338802,
    0x00340802, 0x0034F807, 0x00351803, 0x00352804, 0x00355C01,
    0x00358802, 0x0035E401, 0x00360802, 0x00372801, 0x00373C06,
    0x00375801, 0x00376008, 0x0037C803, 0x0038C401, 0x0038D007,
    0x0038FC01, 0x00391C09, 0x00396802, 0x003AC401, 0x003AD006,
    0x003AEC02, 0x003B2006, 0x003C041F, 0x003CD00C, 0x003DC417,
    0x003E340B, 0x003E6424, 0x003EF80F, 0x003F380D, 0x0040AC14,
    0x00412806, 0x00415804, 0x00417803, 0x00418803, 0x00419C07,
    0x0041C404, 0x0042080C, 0x00423C01, 0x00426806, 0x0043EC01,
    0x004D740C, 0x004E400A, 0x00500001, 0x0059B402, 0x005A0001,
    0x005A6C02, 0x005BAC03, 0x005C4803, 0x005CC805, 0x005D4802,
    0x005DC802, 0x005ED023, 0x005F6004, 0x005F7401, 0x0060000F,
    0x0062A401, 0x0064800C, 0x0064C00C, 0x00650001, 0x00651002,
    0x0066C011, 0x00672002, 0x00677822, 0x00685C05, 0x00687802,
    0x0069540A, 0x0069801D, 0x0069FC01, 0x006A8007, 0x006AA006,
    0x006C0005, 0x006CD011, 0x006D6823, 0x006E0003, 0x006E840D,
    0x006F980E, 0x006FF004, 0x00709014, 0x0070EC05, 0x0071F802,
    0x00730008, 0x00734019, 0x0073B401, 0x0073C803, 0x00770027,
    0x0077F004, 0x007EF401, 0x007EFC03, 0x007F3403, 0x007F7403,
    0x007FB403, 0x007FF402, 0x00800065, 0x0081A806, 0x0081E805,
    0x00822805, 0x0082801A, 0x00834021, 0x00840002, 0x00840C04,
    0x00842002, 0x00845001, 0x00845803, 0x00847806, 0x00849401,
    0x00849C01, 0x0084A401, 0x0084B801, 0x0084E802, 0x00850005,
    0x00852804, 0x00853C01, 0x00864264, 0x00900027, 0x0091000B,
    0x0092704E, 0x00940200, 0x009C0475, 0x009E53B9, 0x00AD400A,
    0x00B39406, 0x00B3BC03, 0x00B3E404, 0x00B3F802, 0x00B5C001,
    0x00B5FC01, 0x00B7804F, 0x00B8C00C, 0x00BA001A, 0x00BA6C59,
    0x00BC00D6, 0x00BFC00C, 0x00C00005, 0x00C02019, 0x00C0A807,
    0x00C0D802, 0x00C0F403, 0x00C26404, 0x00C28001, 0x00C3EC01,
    0x00C64002, 0x00C6580A, 0x00C70024, 0x00C8001F, 0x00C8A81E,
    0x00C94001, 0x00C98020, 0x00CA2827, 0x00CB003F, 0x00CC0100,
    0x01370040, 0x02924037, 0x0293F802, 0x02983403, 0x0299BC10,
    0x029A7C01, 0x029BC008, 0x029C0017, 0x029C8002, 0x029E2402,
    0x02A00801, 0x02A01801, 0x02A02C01, 0x02A08C09, 0x02A0D804,
    0x02A1D004, 0x02A20002, 0x02A2D011, 0x02A33802, 0x02A38012,
    0x02A3E003, 0x02A4980A, 0x02A51C0D, 0x02A57C01, 0x02A60004,
    0x02A6CC1B, 0x02A77802, 0x02A8A40E, 0x02A90C01, 0x02A93002,
    0x02A97004, 0x02A9DC03, 0x02A9EC01, 0x02AAC001, 0x02AAC803,
    0x02AADC02, 0x02AAF802, 0x02AB0401, 0x02AB7802, 0x02ABAC07,
    0x02ABD402, 0x02AF8C0B, 0x03600001, 0x036DFC02, 0x036FFC02,
    0x037FFC01, 0x03EC7801, 0x03ECA401, 0x03EEC810, 0x03F4F802,
    0x03F7F002, 0x03F8001A, 0x03F88007, 0x03F8C023, 0x03F95013,
    0x03F9A004, 0x03FBFC01, 0x03FC040F, 0x03FC6807, 0x03FCEC06,
    0x03FD6C0B, 0x03FF8007, 0x03FFA007, 0x03FFE405, 0x04040003,
    0x0404DC09, 0x0405E411, 0x0406400C, 0x0407402E, 0x040E7C01,
    0x040F4001, 0x04215C01, 0x04247C01, 0x0424FC01, 0x04280403,
    0x04281402, 0x04283004, 0x0428E003, 0x0428FC01, 0x04294009,
    0x0429FC01, 0x042CE407, 0x04400003, 0x0440E016, 0x04420003,
    0x0442C012, 0x04440003, 0x04449C0E, 0x04450004, 0x04460003,
    0x0446CC0E, 0x04471404, 0x045AAC0D, 0x0491C004, 0x05BD442E,
    0x05BE3C04, 0x074000F6, 0x07440027, 0x0744A4B5, 0x07480046,
    0x074C0057, 0x075B0401, 0x075B6C01, 0x075BEC01, 0x075C5401,
    0x075CD401, 0x075D3C01, 0x075DBC01, 0x075E2401, 0x075EA401,
    0x075F0C01, 0x07BBC002, 0x07C0002C, 0x07C0C064, 0x07C2800F,
    0x07C2C40E, 0x07C3040F, 0x07C3440F, 0x07C4401F, 0x07C4C03C,
    0x07C5C02B, 0x07C7981D, 0x07C8402B, 0x07C90009, 0x07C94002,
    0x07CC0021, 0x07CCC006, 0x07CCDC46, 0x07CE0014, 0x07CE8025,
    0x07CF1805, 0x07CF8011, 0x07D0003F, 0x07D10001, 0x07D108B6,
    0x07D3E404, 0x07D4003E, 0x07D50004, 0x07D54018, 0x07D7EC46,
    0x07D9140B, 0x07DA0046, 0x07DC0074, 0x38000401, 0x38008060,
    0x380400F0,
  };
  static const unsigned int aAscii[4] = {
    0xFFFFFFFF, 0xFC00FFFF, 0xF8000001, 0xF8000001,
  };

  if( c<128 ){
    return ( (aAscii[c >> 5] & (1 << (c & 0x001F)))==0 );
  }else if( c<(1<<22) ){
    unsigned int key = (c<<10) | 0x000003FF;
    int iRes = 0;
    int iHi = sizeof(aEntry)/sizeof(aEntry[0]) - 1;
    int iLo = 0;
    while( iHi>=iLo ){
      int iTest = (iHi + iLo) / 2;
      if( key >= aEntry[iTest] ){
        iRes = iTest;
        iLo = iTest+1;
      }else{
        iHi = iTest-1;
      }
    }
    assert( aEntry[0]<key );
    assert( key>=aEntry[iRes] );
    return (c >= ((aEntry[iRes]>>10) + (aEntry[iRes]&0x3FF)));
  }
  return true;
}

/*
** Return true if the argument interpreted as a unicode codepoint
** is a diacritical modifier character.
*/
bool unicodeIsdiacritic(unsigned int c){
  unsigned int mask0 = 0x08029FDF;
  unsigned int mask1 = 0x000361F8;
  if( c<768 || c>817 ) return false;
  return (c < 768+32) ?
      (mask0 & (1 << (c-768))) :
      (mask1 & (1 << (c-768-32)));
}

// END code borrowed from SQLite3 and slightly modified

}

}
}
