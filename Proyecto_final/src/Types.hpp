#pragma once

#include <string>
#include <vector>

enum class PortState {
    OPEN,
    CLOSED
};

struct PortResult {
    int port;
    PortState state;
    std::string service;
};

struct Args {
    std::string host;
    std::vector<int> ports;
};
