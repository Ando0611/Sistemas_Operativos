#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

class ResultFormatter {
public:
    static void print(const std::string& host,
                      const std::vector<PortResult>& results,
                      double elapsedSeconds);

    static void exportCsv(const std::string& csvPath,
                          const std::vector<HostResults>& byHost);

private:
    static std::string serviceFor(int port);
    static const char* stateToString(PortState state);
};
