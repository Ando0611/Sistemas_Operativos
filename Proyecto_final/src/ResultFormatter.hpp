#pragma once

#include "Types.hpp"

#include <string>

class ResultFormatter {
public:
    static void print(const std::string& host,
                      const std::vector<PortResult>& results,
                      double elapsedSeconds);

private:
    static std::string serviceFor(int port);
};
