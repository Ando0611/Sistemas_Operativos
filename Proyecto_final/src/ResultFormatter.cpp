#include "ResultFormatter.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>

std::string ResultFormatter::serviceFor(int port) {
    static const std::map<int, std::string> table = {
        {21, "FTP"},       {22, "SSH"},       {23, "Telnet"},
        {25, "SMTP"},      {53, "DNS"},       {80, "HTTP"},
        {110, "POP3"},     {143, "IMAP"},     {443, "HTTPS"},
        {445, "SMB"},      {3306, "MySQL"},   {3389, "RDP"},
        {5432, "PostgreSQL"}, {6379, "Redis"}, {8080, "HTTP-alt"},
        {8443, "HTTPS-alt"}, {27017, "MongoDB"}
    };
    auto it = table.find(port);
    return it != table.end() ? it->second : "desconocido";
}

const char* ResultFormatter::stateToString(PortState state) {
    switch (state) {
        case PortState::OPEN:     return "OPEN";
        case PortState::CLOSED:   return "CLOSED";
        case PortState::FILTERED: return "FILTERED";
    }
    return "?";
}

void ResultFormatter::print(const std::string& host,
                             const std::vector<PortResult>& results,
                             double elapsedSeconds) {
    std::cout << "Escaneando " << host << " (" << results.size() << " puertos)...\n\n";

    int openCount = 0;
    int closedCount = 0;
    int filteredCount = 0;
    bool headerPrinted = false;

    for (const auto& r : results) {
        if (r.state == PortState::OPEN) {
            if (!headerPrinted) {
                std::cout << std::left
                          << std::setw(8) << "PUERTO"
                          << std::setw(10) << "ESTADO"
                          << std::setw(14) << "SERVICIO"
                          << "BANNER\n";
                headerPrinted = true;
            }
            std::cout << std::left
                      << std::setw(8) << r.port
                      << std::setw(10) << "OPEN"
                      << std::setw(14) << serviceFor(r.port)
                      << r.banner << "\n";
            ++openCount;
        } else if (r.state == PortState::CLOSED) {
            ++closedCount;
        } else {
            ++filteredCount;
        }
    }

    if (!headerPrinted) {
        std::cout << "(sin puertos abiertos)\n";
    }

    std::cout << "\n---------------------------------------\n";
    std::cout << "Total: " << results.size() << " puertos"
              << " | Abiertos: " << openCount
              << " | Cerrados: " << closedCount
              << " | Filtrados: " << filteredCount << "\n";
    std::cout << "Tiempo: " << std::fixed << std::setprecision(2)
              << elapsedSeconds << "s\n";
}

void ResultFormatter::exportCsv(const std::string& csvPath,
                                 const std::vector<HostResults>& byHost) {
    std::ofstream out(csvPath);
    if (!out) {
        std::cerr << "Error: no se pudo escribir '" << csvPath << "'\n";
        return;
    }

    out << "host,puerto,estado,servicio,banner\n";
    for (const auto& h : byHost) {
        for (const auto& r : h.results) {
            out << h.host << "," << r.port << "," << stateToString(r.state)
                << "," << serviceFor(r.port) << ",";
            if (!r.banner.empty()) {
                out << "\"";
                for (char c : r.banner) {
                    if (c == '"') out << "\"\"";
                    else out << c;
                }
                out << "\"";
            }
            out << "\n";
        }
    }

    std::cout << "Resultados exportados a " << csvPath << "\n";
}
