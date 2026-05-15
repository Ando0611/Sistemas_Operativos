#include "ScannerApp.hpp"

#include "ArgumentParser.hpp"
#include "PortScanner.hpp"
#include "ResultFormatter.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>

int ScannerApp::run(int argc, char* argv[]) {
    Args args = ArgumentParser::parse(argc, argv);

    auto start = std::chrono::steady_clock::now();

    PortScanner scanner(args.timeoutMs, args.readBanner);
    std::vector<HostResults> byHost;
    byHost.reserve(args.hosts.size());

    for (const auto& host : args.hosts) {
        auto hostStart = std::chrono::steady_clock::now();
        std::vector<PortResult> results = scanner.scan(host, args.ports);
        auto hostEnd = std::chrono::steady_clock::now();
        double hostElapsed = std::chrono::duration<double>(hostEnd - hostStart).count();

        if (!results.empty()) {
            ResultFormatter::print(host, results, hostElapsed);
            std::cout << "\n";
        }
        byHost.push_back({host, std::move(results)});
    }

    auto end = std::chrono::steady_clock::now();
    double totalElapsed = std::chrono::duration<double>(end - start).count();

    if (args.hosts.size() > 1) {
        std::cout << "Tiempo total (" << args.hosts.size() << " hosts): "
                  << std::fixed << std::setprecision(2) << totalElapsed << "s\n";
    }

    if (!args.csvPath.empty()) {
        ResultFormatter::exportCsv(args.csvPath, byHost);
    }

    return 0;
}
