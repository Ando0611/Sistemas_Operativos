// Para compilar: make
// Ejemplos de uso:
//   ./scanner --host 127.0.0.1 --ports 20-80
//   ./scanner --host scanme.nmap.org --ports 22,80,443 --timeout 800 --banner
//   ./scanner --range 192.168.1.1-192.168.1.20 --ports 22,80 --timeout 500
//   ./scanner --host 127.0.0.1 --ports all --export resultados.csv

#include "ScannerApp.hpp"

int main(int argc, char *argv[]) {
  ScannerApp app;
  return app.run(argc, argv);
}
