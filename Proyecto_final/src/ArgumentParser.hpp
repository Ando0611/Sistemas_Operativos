#pragma once

#include "Types.hpp"

#include <string>

class ArgumentParser {
public:
    static Args parse(int argc, char* argv[]);

private:
    static std::vector<int> parsePortSpec(const std::string& spec);
    static std::vector<std::string> parseRangeSpec(const std::string& spec);
    static int parseTimeout(const std::string& spec);
    static void usage();
};
