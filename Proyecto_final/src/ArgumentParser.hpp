#pragma once

#include "Types.hpp"

class ArgumentParser {
public:
    static Args parse(int argc, char* argv[]);

private:
    static std::vector<int> parsePortSpec(const std::string& spec);
    static void usage();
};
