/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Kullo {
namespace Util {

class MasterKey
{
public:
    explicit MasterKey(const std::string &pemKey);
    explicit MasterKey(
            const std::vector<std::string> &dataBlocks,
            std::uint32_t version = 1);
    explicit MasterKey(
            const std::vector<unsigned char> &key,
            std::uint32_t version = 1);

    static bool isValid(const std::string &pemKey);
    static bool isValid(const std::vector<std::string> &dataBlocks);
    static MasterKey makeRandom();

    static bool isPlausibleSingleBlock(const std::string &block);

    std::uint32_t version();
    std::vector<std::string> dataBlocks() const;
    std::vector<unsigned char> toVector() const;
    std::string toPem() const;
    bool operator==(const MasterKey &other) const;
    bool operator!=(const MasterKey &other) const;

private:
    static std::vector<unsigned char> blockListToRawData(
            const std::vector<std::string> &blockList);
    static void checkKey(const std::vector<std::string> &blockList);
    static bool checkLuhn(const std::string &block);
    static bool checkRange(const std::string &block);
    static std::string luhnMod10GenerateCheckCharacter(const std::string &digits);

    std::vector<unsigned char> data_;
    std::uint32_t version_;
};

std::ostream &operator<<(std::ostream &out, const MasterKey &key);

}
}
