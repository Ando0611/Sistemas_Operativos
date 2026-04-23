#pragma once

#include "Types.hpp"

#include <string>
#include <vector>

class PortScanner {
public:
    std::vector<PortResult> scan(const std::string& host, const std::vector<int>& ports);
};
