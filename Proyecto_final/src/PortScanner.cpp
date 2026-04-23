#include "PortScanner.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace {

constexpr size_t kMaxWorkers = 256;

std::mutex g_cerrMutex;

void scanSinglePort(struct sockaddr_in baseTarget, int port, PortResult& result) {
    result.port = port;
    result.service = "";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        {
            std::lock_guard<std::mutex> lock(g_cerrMutex);
            std::cerr << "Error: no se pudo crear socket para puerto " << port << "\n";
        }
        result.state = PortState::CLOSED;
        return;
    }

    baseTarget.sin_port = htons(static_cast<uint16_t>(port));
    int res = connect(sock, reinterpret_cast<struct sockaddr*>(&baseTarget), sizeof(baseTarget));
    result.state = (res == 0) ? PortState::OPEN : PortState::CLOSED;

    close(sock);
}

} // namespace

std::vector<PortResult> PortScanner::scan(const std::string& host, const std::vector<int>& ports) {
    struct addrinfo hints{};
    struct addrinfo* res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || res == nullptr) {
        std::cerr << "Error: no se pudo resolver host '" << host << "'\n";
        std::exit(1);
    }

    struct sockaddr_in baseTarget{};
    baseTarget.sin_family = AF_INET;
    baseTarget.sin_addr = reinterpret_cast<struct sockaddr_in*>(res->ai_addr)->sin_addr;
    freeaddrinfo(res);

    std::vector<PortResult> results(ports.size());
    std::atomic<size_t> next{0};

    size_t workerCount = std::min(kMaxWorkers, ports.size());
    std::vector<std::thread> workers;
    workers.reserve(workerCount);

    for (size_t w = 0; w < workerCount; ++w) {
        workers.emplace_back([&, baseTarget]() {
            for (;;) {
                size_t i = next.fetch_add(1, std::memory_order_relaxed);
                if (i >= ports.size()) break;
                scanSinglePort(baseTarget, ports[i], results[i]);
            }
        });
    }

    for (auto& t : workers) {
        t.join();
    }

    return results;
}
