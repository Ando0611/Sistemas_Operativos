#include <iostream>
#include <unistd.h>   
#include <fcntl.h>    
#include <cstring>    

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <nombre_archivo>\n";
        return 1;
    }

    const char* nombreArchivo = argv[1];

    int fd = open(nombreArchivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::perror("Error al abrir el archivo");
        return 1;
    }

    const char* texto = "Tarea 1. Andoreni Claudio Flores Alcocer.\n";
    size_t longitud = std::strlen(texto);

    ssize_t bytesEscritos = write(fd, texto, longitud);
    if (bytesEscritos == -1) {
        std::perror("Error al escribir en el archivo");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        std::perror("Error al cerrar el archivo");
        return 1;
    }

    std::cout << "Archivo modificado correctamente.\n";
    return 0;
}
