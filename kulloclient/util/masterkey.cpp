/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#include "kulloclient/util/masterkey.h"

#include <algorithm>
#include <sstream>
#include <boost/regex.hpp>

#include "kulloclient/crypto/symmetrickeygenerator.h"
#include "kulloclient/util/formatstring.h"
#include "kulloclient/util/misc.h"

namespace Kullo {
namespace Util {

namespace {
const auto MASTER_KEY_HEADER_REGEX = boost::regex("-*BEGIN KULLO PRIVATE MASTER KEY-*");
const auto MASTER_KEY_FOOTER_REGEX = boost::regex("-*END KULLO PRIVATE MASTER KEY-*");
const auto SINGLE_BLOCK_BASIC_REGEX = boost::regex("[0-9]{6}");
const auto MAX_BLOCK_VALUE = int{65535};
}

/*
 * Example key:
 *
 * -----BEGIN KULLO PRIVATE MASTER KEY-----
 * Version: 1 (256 bit)
 *
 * 009167 186544 520379 167502
 * 092114 170530 357103 015669
 * 422915 084699 505677 309062
 * 407924 293407 624445 483701
 * -----END KULLO PRIVATE MASTER KEY-----
 *
 * Note: key is the user input and might not be perfectly clean.
 *
 */
MasterKey::MasterKey(const std::string &pemKey)
{
    std::string key  = pemKey;
    key.erase(std::remove(key.begin(), key.end(), '\r'), key.end());
    key.erase(std::remove(key.begin(), key.end(), '\t'), key.end());
    key = boost::regex_replace(key, MASTER_KEY_HEADER_REGEX, "");
    key = boost::regex_replace(key, MASTER_KEY_FOOTER_REGEX, "");
    FormatString::trim(key);

    size_t nnPos = key.find("\n\n");
    if (nnPos != std::string::npos) // there is a header
    {
        std::string header(key.begin(), key.begin()+nnPos);
        FormatString::trim(header);

        std::stringstream headerStream(header);
        std::string line;

        if (headerStream)
        {
            while(std::getline(headerStream, line, '\n'))
            {
                FormatString::trim(line);

                size_t colonPos = line.find(':');
                if (colonPos == std::string::npos)
                {
                    throw std::invalid_argument(
                                "Invalid MasterKey header line: "
                                "No colon found.");
                }

                std::string name(line.begin(), line.begin()+colonPos);
                std::string value(line.begin()+colonPos+1, line.end());
                FormatString::trim(value);

                if (FormatString::toLower(name) == "version")
                {
                    std::istringstream(value) >> version_;
                }
                else
                {
                    throw std::invalid_argument(
                                std::string("MasterKey(): "
                                            "Invalid header name: '")
                                + name + "'");
                }
            }
        }
    }

    std::string body;
    if (nnPos != std::string::npos) body = std::string(key.begin()+nnPos+2, key.end());
    else                            body = std::string(key.begin(), key.end());
    std::stringstream bodyStream(body);
    std::string line;

    if (!bodyStream) throw std::invalid_argument("MasterKey(): Invalid body");

    std::vector<std::string> blockList;
    while(std::getline(bodyStream, line, '\n'))
    {
        FormatString::trim(line);
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());

        std::string block1(line.begin(),    line.begin()+ 6);
        std::string block2(line.begin()+ 6, line.begin()+12);
        std::string block3(line.begin()+12, line.begin()+18);
        std::string block4(line.begin()+18, line.begin()+24);

        blockList.push_back(block1);
        blockList.push_back(block2);
        blockList.push_back(block3);
        blockList.push_back(block4);
    }

    checkKey(blockList);
    data_ = blockListToRawData(blockList);
}

MasterKey::MasterKey(const std::vector<std::string> &dataBlocks, std::uint32_t version)
    : version_(version)
{
    checkKey(dataBlocks);
    data_ = blockListToRawData(dataBlocks);
}

MasterKey::MasterKey(const std::vector<unsigned char> &key, std::uint32_t version)
    : data_(key),
      version_(version)
{
    if (data_.size()*8 != 256)
    {
        throw std::invalid_argument("MasterKey(): Input does not have 256 bits.");
    }
}

bool MasterKey::isValid(const std::string &pemKey)
{
    try
    {
        MasterKey{pemKey};
    }
    catch(std::invalid_argument)
    {
        return false;
    }
    return true;
}

bool MasterKey::isValid(const std::vector<std::string> &dataBlocks)
{
    try
    {
        MasterKey{dataBlocks};
    }
    catch(std::invalid_argument)
    {
        return false;
    }
    return true;
}

MasterKey MasterKey::makeRandom()
{
    return MasterKey(Crypto::SymmetricKeyGenerator().makeMasterKey().toVector());
}

bool MasterKey::isPlausibleSingleBlock(const std::string &block)
{
    boost::match_results<std::string::const_iterator> results;
    if (!boost::regex_match(block, results, SINGLE_BLOCK_BASIC_REGEX))
    {
        return false;
    }

    if (!checkLuhn(block))
    {
        return false;
    }

    if (!checkRange(block))
    {
        return false;
    }

    return true;
}

std::uint32_t MasterKey::version()
{
    return version_;
}

std::vector<std::string> MasterKey::dataBlocks() const
{
    std::vector<std::string> dataBlocks;
    for (int i = 0; i < 16; ++i)
    {
        unsigned int n = 0;
        n = (data_[2*i] << 8) | data_[2*i+1];

        std::string blockNumber = Util::FormatString::padLeft(std::to_string(n), 5, '0');
        std::string blockCheck = luhnMod10GenerateCheckCharacter(blockNumber);
        dataBlocks.push_back(blockNumber + blockCheck);
    }
    return dataBlocks;
}

std::vector<unsigned char> MasterKey::toVector() const
{
    return data_;
}

std::string MasterKey::toPem() const
{
    auto blocks = dataBlocks();

    std::stringstream out;
    out << "-----BEGIN KULLO PRIVATE MASTER KEY-----\n";
    out << "Version: 1 (256 bit)\n";
    out << "\n";
    out << blocks[ 0] << " " << blocks[ 1] << " " << blocks[ 2] << " " << blocks[ 3] << "\n";
    out << blocks[ 4] << " " << blocks[ 5] << " " << blocks[ 6] << " " << blocks[ 7] << "\n";
    out << blocks[ 8] << " " << blocks[ 9] << " " << blocks[10] << " " << blocks[11] << "\n";
    out << blocks[12] << " " << blocks[13] << " " << blocks[14] << " " << blocks[15] << "\n";
    out << "-----END KULLO PRIVATE MASTER KEY-----\n";

    return out.str();
}

bool MasterKey::operator==(const MasterKey &other) const
{
    if (this->version_ != other.version_) return false;
    if (this->data_    != other.data_) return false;
    return true;
}

bool MasterKey::operator!=(const MasterKey &other) const
{
    return !(*this == other);
}

std::vector<unsigned char> MasterKey::blockListToRawData(
        const std::vector<std::string> &blockList)
{
    std::vector<unsigned char> rawData;
    for(std::string block : blockList)
    {
        std::string value = block.substr(0, 5);
        std::uint16_t bin;
        std::istringstream(value) >> bin;
        rawData.push_back(bin >> 8);
        rawData.push_back(bin & 255);
    }
    return rawData;
}

void MasterKey::checkKey(const std::vector<std::string> &blockList)
{
    if (blockList.size() != 16)
    {
        throw std::invalid_argument("MasterKey(): Input does not have 16 blocks.");
    }

    for (std::string block : blockList)
    {
        if (block.size() != 6)
        {
            throw std::invalid_argument("MasterKey(): Block digit size is not 6: " + block);
        }

        if (!checkLuhn(block))
        {
            throw std::invalid_argument("MasterKey(): Block luhn check failed: " + block);
        }

        std::string value      = block.substr(0, 5);
        std::string checkDigit = block.substr(5, 1);

        std::uint32_t valueInt;
        try {
            std::istringstream(value) >> valueInt;
        }
        catch (...)
        {
            throw std::invalid_argument("MasterKey(): Value could not be converted to integer: " + value);
        }
        if (valueInt > MAX_BLOCK_VALUE)
        {
            throw std::invalid_argument("MasterKey(): Value of block must be less or equal 65535: " + value);
        }

        std::uint32_t checkDigitInt;
        try {
            std::istringstream(checkDigit) >> checkDigitInt;
        }
        catch (...)
        {
            throw std::invalid_argument("MasterKey(): Check digit could not be converted to integer: " + checkDigit);
        }
    }
}

bool MasterKey::checkLuhn(const std::string &block)
{
    const unsigned int N = 10;
    unsigned int factor = 1;
    unsigned int sum = 0;

    for (int i = block.length()-1; i >= 0; i--)
    {
        unsigned int codePoint;
        std::istringstream(block.substr(i, 1)) >> codePoint;
        unsigned int addend = factor * codePoint;
        factor = (factor == 2) ? 1 : 2;
        addend = (addend / N) + (addend % N);
        sum += addend;
    }

    unsigned int remainder = sum % N;
    return (remainder == 0);
}

bool MasterKey::checkRange(const std::string &block)
{
    const std::string value = block.substr(0, 5);

    std::uint32_t valueInt;
    try {
        std::istringstream(value) >> valueInt;
    }
    catch (...) {
        kulloAssertionFailed("Couldn't parse block.");
    }

    return valueInt <= MAX_BLOCK_VALUE;
}

std::string MasterKey::luhnMod10GenerateCheckCharacter(const std::string &digits)
{
    const unsigned int N = 10;
    unsigned int factor = 2;
    unsigned int theSum = 0;
    int i = digits.length() - 1;
    while (i >= 0)
    {
        unsigned int codePoint;
        std::istringstream(digits.substr(i, 1)) >> codePoint;
        unsigned int x = factor * codePoint;
        factor = (factor == 2) ? 1 : 2;
        x = (x / N) + (x % N);
        theSum += x;
        i--;
    }
    unsigned int remainder = theSum % N;
    return std::to_string((N - remainder) % N);
}

std::ostream &operator<<(std::ostream &out, const MasterKey &key)
{
    out << key.toPem();
    return out;
}

}
}
