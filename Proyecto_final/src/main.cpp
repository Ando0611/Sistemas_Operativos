// Para compilar: make
// Para ejecutar: ./scanner --host <ip/host> --ports <spec>
//   <spec> puede ser un rango (20-80), una lista (22,80,443) o un puerto (80).
// ./scanner --host 127.0.0.1 --ports all

#include "ScannerApp.hpp"

int main(int argc, char *argv[]) {
  ScannerApp app;
  return app.run(argc, argv);
}
