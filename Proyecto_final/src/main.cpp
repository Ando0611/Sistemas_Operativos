// Para poder usar este código se debe de ejecutar en la terminal usando el
// siguiente comando : g++ -std=c++17 src/main.cpp -o scanner

// Para poder ejecutar el programa se debe usar el siguiente comando :
// ./scanner --host <IP_ADDRESS> --ports <INICIO>-<FIN>

// En caso de mostrar que no se cuentan con los permisos requeridos se debe
// ejecutar "chmod +x scanner" y volver a ejecutar el programa.

#include <arpa/inet.h>  // sockaddr_in, htons
#include <cstdlib>      // exit()
#include <cstring>      // Funciones C de strings
#include <iostream>     // Entrada y salida estándar
#include <netdb.h>      // getaddrinfo()
#include <string>       // Manejo de strings
#include <sys/socket.h> // socket(), connect()
#include <unistd.h>     // close()

// Función que muestra cómo usar el programa
void usage() {
  std::cout << "Usage: ./scanner --host <ip/host> --ports <start-end>\n";
  exit(1);
}

int main(int argc, char *argv[]) {

  // Verificamos que se pasen exactamente 4 argumentos
  // Ejemplo esperado:
  // ./scanner --host 127.0.0.1 --ports 20-80
  if (argc != 5)
    usage();

  std::string host;  // Guardará la IP o hostname
  std::string range; // Guardará el rango de puertos

  // Recorremos los argumentos para encontrar --host y --ports
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--host" && i + 1 < argc)
      host = argv[++i]; // Guardamos la IP/hostname

    else if (arg == "--ports" && i + 1 < argc)
      range = argv[++i]; // Guardamos el rango
  }

  // Si falta algún argumento obligatorio, mostramos ayuda
  if (host.empty() || range.empty())
    usage();

  // Separar el rango "inicio-fin"
  size_t dash = range.find('-');
  if (dash == std::string::npos)
    usage();

  int start = std::stoi(range.substr(0, dash));
  int end = std::stoi(range.substr(dash + 1));

  // Resolver el hostname/IP una sola vez
  struct addrinfo hints{}, *res = nullptr;
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // Convertimos hostname a dirección IP
  if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) {
    std::cerr << "Error: cannot resolve host\n";
    return 1;
  }

  // Preparar estructura sockaddr_in
  struct sockaddr_in target{};
  target.sin_family = AF_INET;
  target.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;

  freeaddrinfo(res); // Liberar memoria de getaddrinfo

  // Escaneo secuencial (SIN concurrencia)
  for (int port = start; port <= end; port++) {

    // Crear socket TCP
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
      std::cerr << host << ":" << port << " ERROR\n";
      continue;
    }

    // Configurar el puerto actual
    target.sin_port = htons(port);

    // Intentar conexión (Connect Scan básico)
    int result = connect(sock, (struct sockaddr *)&target, sizeof(target));

    // Si connect devuelve 0 → puerto abierto
    if (result == 0)
      std::cout << host << ":" << port << " OPEN\n";
    else
      std::cout << host << ":" << port << " CLOSED\n";

    // Cerrar socket después de cada intento
    close(sock);
  }

  return 0;
}