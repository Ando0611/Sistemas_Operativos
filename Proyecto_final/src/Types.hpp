#pragma once

#include <string>
#include <vector>

enum class PortState {
    OPEN,
    CLOSED,
    FILTERED
};

struct PortResult {
    int port;
    PortState state;
    std::string service;
    std::string banner;
};

struct Args {
    std::vector<std::string> hosts;
    std::vector<int> ports;
    int timeoutMs;
    bool readBanner;
    std::string csvPath;
};

struct HostResults {
    std::string host;
    std::vector<PortResult> results;
};
