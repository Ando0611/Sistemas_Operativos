#include "ArgumentParser.hpp"

#include <arpa/inet.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

constexpr uint32_t kMaxRangeSize = 1024;

bool parseIpv4(const std::string& s, uint32_t& out) {
    struct in_addr addr;
    if (inet_pton(AF_INET, s.c_str(), &addr) != 1) return false;
    out = ntohl(addr.s_addr);
    return true;
}

std::string ipv4ToString(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = htonl(ip);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));
    return std::string(buf);
}

} // namespace

void ArgumentParser::usage() {
    std::cerr << "Uso: ./scanner (--host <ip/host> | --range <ip1-ip2>) --ports <spec>\n";
    std::cerr << "             [--timeout <ms>] [--banner] [--export <archivo.csv>]\n";
    std::cerr << "  <spec> de puertos: 'all', rango (20-80), lista (22,80,443) o uno solo (80)\n";
    std::cerr << "  --range:          rango IPv4, ej. 192.168.1.1-192.168.1.20 (max 1024 IPs)\n";
    std::cerr << "  --timeout:        ms maximo de espera por puerto (default 1000)\n";
    std::cerr << "  --banner:         intenta leer banner inicial en puertos OPEN\n";
    std::cerr << "  --export:         guarda resultados en CSV\n";
    std::exit(1);
}

std::vector<int> ArgumentParser::parsePortSpec(const std::string& spec) {
    if (spec == "all") {
        std::vector<int> ports;
        ports.reserve(65535);
        for (int p = 1; p <= 65535; ++p) ports.push_back(p);
        return ports;
    }

    bool hasDash = spec.find('-') != std::string::npos;
    bool hasComma = spec.find(',') != std::string::npos;

    if (hasDash && hasComma) {
        std::cerr << "Error: no se pueden mezclar rango y lista\n";
        usage();
    }

    std::vector<int> ports;

    try {
        if (hasDash) {
            size_t dash = spec.find('-');
            int start = std::stoi(spec.substr(0, dash));
            int end = std::stoi(spec.substr(dash + 1));
            if (start < 1 || end < start || end > 65535) {
                std::cerr << "Error: rango invalido (1-65535, inicio <= fin)\n";
                usage();
            }
            for (int p = start; p <= end; ++p) ports.push_back(p);
        } else if (hasComma) {
            std::stringstream ss(spec);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (item.empty()) throw std::invalid_argument("empty");
                int p = std::stoi(item);
                if (p < 1 || p > 65535) {
                    std::cerr << "Error: puerto fuera de rango (1-65535)\n";
                    usage();
                }
                ports.push_back(p);
            }
        } else {
            int p = std::stoi(spec);
            if (p < 1 || p > 65535) {
                std::cerr << "Error: puerto fuera de rango (1-65535)\n";
                usage();
            }
            ports.push_back(p);
        }
    } catch (const std::exception&) {
        std::cerr << "Error: spec de puertos invalido\n";
        usage();
    }

    if (ports.empty()) {
        std::cerr << "Error: no se especificaron puertos\n";
        usage();
    }

    return ports;
}

std::vector<std::string> ArgumentParser::parseRangeSpec(const std::string& spec) {
    size_t dash = spec.find('-');
    if (dash == std::string::npos) {
        std::cerr << "Error: --range requiere formato ip1-ip2\n";
        usage();
    }

    std::string startStr = spec.substr(0, dash);
    std::string endStr = spec.substr(dash + 1);

    uint32_t start = 0, end = 0;
    if (!parseIpv4(startStr, start) || !parseIpv4(endStr, end)) {
        std::cerr << "Error: IPv4 invalida en --range\n";
        usage();
    }
    if (start > end) {
        std::cerr << "Error: en --range, ip1 debe ser <= ip2\n";
        usage();
    }
    if (end - start + 1 > kMaxRangeSize) {
        std::cerr << "Error: --range excede el maximo de " << kMaxRangeSize << " IPs\n";
        usage();
    }

    std::vector<std::string> hosts;
    hosts.reserve(end - start + 1);
    for (uint32_t ip = start; ip <= end; ++ip) {
        hosts.push_back(ipv4ToString(ip));
    }
    return hosts;
}

int ArgumentParser::parseTimeout(const std::string& spec) {
    try {
        int ms = std::stoi(spec);
        if (ms < 1 || ms > 60000) {
            std::cerr << "Error: --timeout fuera de rango (1-60000 ms)\n";
            usage();
        }
        return ms;
    } catch (const std::exception&) {
        std::cerr << "Error: --timeout invalido\n";
        usage();
    }
    return 1000;
}

Args ArgumentParser::parse(int argc, char* argv[]) {
    Args args;
    args.timeoutMs = 1000;
    args.readBanner = false;

    std::string hostSpec;
    std::string rangeSpec;
    std::string portSpec;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            hostSpec = argv[++i];
        } else if (arg == "--range" && i + 1 < argc) {
            rangeSpec = argv[++i];
        } else if (arg == "--ports" && i + 1 < argc) {
            portSpec = argv[++i];
        } else if (arg == "--timeout" && i + 1 < argc) {
            args.timeoutMs = parseTimeout(argv[++i]);
        } else if (arg == "--banner") {
            args.readBanner = true;
        } else if (arg == "--export" && i + 1 < argc) {
            args.csvPath = argv[++i];
        } else {
            std::cerr << "Error: argumento desconocido '" << arg << "'\n";
            usage();
        }
    }

    if (portSpec.empty()) {
        std::cerr << "Error: falta --ports\n";
        usage();
    }
    if (hostSpec.empty() == rangeSpec.empty()) {
        std::cerr << "Error: especifica --host o --range (uno solo)\n";
        usage();
    }

    if (!hostSpec.empty()) {
        args.hosts.push_back(hostSpec);
    } else {
        args.hosts = parseRangeSpec(rangeSpec);
    }

    args.ports = parsePortSpec(portSpec);
    return args;
}
