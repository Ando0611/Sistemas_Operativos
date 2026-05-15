#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

class PortScanner {
public:
    PortScanner(int timeoutMs, bool readBanner);
    std::vector<PortResult> scan(const std::string& host, const std::vector<int>& ports);

private:
    int timeoutMs_;
    bool readBanner_;
};
