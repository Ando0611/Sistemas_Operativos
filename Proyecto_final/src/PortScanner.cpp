#include "PortScanner.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace {

constexpr size_t kMaxWorkers = 256;
constexpr size_t kBannerBufSize = 256;
constexpr size_t kBannerMaxChars = 80;

std::mutex g_cerrMutex;

std::string readBanner(int sock, int port, int timeoutMs) {
    // Aplicar timeout de lectura para no bloquear si el servicio no envia nada
    struct timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // HTTP no envia banner espontaneo; un HEAD provoca una respuesta
    if (port == 80 || port == 8000 || port == 8080) {
        const char* probe = "HEAD / HTTP/1.0\r\n\r\n";
        send(sock, probe, std::strlen(probe), 0);
    }

    char buf[kBannerBufSize];
    ssize_t n = recv(sock, buf, sizeof(buf) - 1, 0);
    if (n <= 0) return "";

    std::string out;
    out.reserve(static_cast<size_t>(n));
    for (ssize_t i = 0; i < n; ++i) {
        unsigned char c = static_cast<unsigned char>(buf[i]);
        if (c == '\r' || c == '\n') break;
        if (c < 32 || c > 126) continue;
        out.push_back(static_cast<char>(c));
        if (out.size() >= kBannerMaxChars) break;
    }
    return out;
}

void scanSinglePort(struct sockaddr_in baseTarget,
                    int port,
                    int timeoutMs,
                    bool wantBanner,
                    PortResult& result) {
    result.port = port;
    result.service = "";
    result.banner = "";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        {
            std::lock_guard<std::mutex> lock(g_cerrMutex);
            std::cerr << "Error: no se pudo crear socket para puerto " << port << "\n";
        }
        result.state = PortState::CLOSED;
        return;
    }

    // Socket en modo no bloqueante para acotar connect() con select()
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    baseTarget.sin_port = htons(static_cast<uint16_t>(port));
    int res = connect(sock, reinterpret_cast<struct sockaddr*>(&baseTarget), sizeof(baseTarget));

    if (res == 0) {
        result.state = PortState::OPEN;
    } else if (errno == EINPROGRESS) {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;

        int sel = select(sock + 1, nullptr, &wfds, nullptr, &tv);
        if (sel == 0) {
            // Sin respuesta dentro del timeout: probable firewall
            result.state = PortState::FILTERED;
        } else if (sel > 0) {
            int soErr = 0;
            socklen_t len = sizeof(soErr);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &soErr, &len);
            if (soErr == 0) {
                result.state = PortState::OPEN;
            } else if (soErr == ECONNREFUSED) {
                result.state = PortState::CLOSED;
            } else {
                result.state = PortState::FILTERED;
            }
        } else {
            result.state = PortState::FILTERED;
        }
    } else if (errno == ECONNREFUSED) {
        result.state = PortState::CLOSED;
    } else {
        result.state = PortState::FILTERED;
    }

    if (result.state == PortState::OPEN && wantBanner) {
        // Restaurar modo bloqueante para que recv() respete SO_RCVTIMEO
        fcntl(sock, F_SETFL, flags);
        int bannerTimeout = std::min(timeoutMs, 1500);
        result.banner = readBanner(sock, port, bannerTimeout);
    }

    close(sock);
}

} // namespace

PortScanner::PortScanner(int timeoutMs, bool readBanner)
    : timeoutMs_(timeoutMs), readBanner_(readBanner) {}

std::vector<PortResult> PortScanner::scan(const std::string& host, const std::vector<int>& ports) {
    struct addrinfo hints{};
    struct addrinfo* res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || res == nullptr) {
        std::cerr << "Error: no se pudo resolver host '" << host << "'\n";
        return {};
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

    int timeoutMs = timeoutMs_;
    bool wantBanner = readBanner_;

    for (size_t w = 0; w < workerCount; ++w) {
        workers.emplace_back([&, baseTarget, timeoutMs, wantBanner]() {
            for (;;) {
                size_t i = next.fetch_add(1, std::memory_order_relaxed);
                if (i >= ports.size()) break;
                scanSinglePort(baseTarget, ports[i], timeoutMs, wantBanner, results[i]);
            }
        });
    }

    for (auto& t : workers) {
        t.join();
    }

    return results;
}
