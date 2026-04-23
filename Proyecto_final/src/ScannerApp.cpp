#include "ScannerApp.hpp"

#include "ArgumentParser.hpp"
#include "PortScanner.hpp"
#include "ResultFormatter.hpp"

#include <chrono>

int ScannerApp::run(int argc, char* argv[]) {
    Args args = ArgumentParser::parse(argc, argv);

    auto start = std::chrono::steady_clock::now();

    PortScanner scanner;
    std::vector<PortResult> results = scanner.scan(args.host, args.ports);

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    ResultFormatter::print(args.host, results, elapsed);

    return 0;
}
