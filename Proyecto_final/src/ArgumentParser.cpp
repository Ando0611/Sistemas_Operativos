#include "ArgumentParser.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

void ArgumentParser::usage() {
    std::cerr << "Uso: ./scanner --host <ip/host> --ports <spec>\n";
    std::cerr << "  <spec>: 'all' (todos los puertos 1-65535),\n";
    std::cerr << "          rango (ej. 20-80), lista (ej. 22,80,443), o puerto (ej. 80)\n";
    std::cerr << "          no se pueden mezclar '-' y ','\n";
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

Args ArgumentParser::parse(int argc, char* argv[]) {
    if (argc != 5) usage();

    Args args;
    std::string spec;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            args.host = argv[++i];
        } else if (arg == "--ports" && i + 1 < argc) {
            spec = argv[++i];
        }
    }

    if (args.host.empty() || spec.empty()) usage();

    args.ports = parsePortSpec(spec);
    return args;
}
