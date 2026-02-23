// Para correr el código se debe de usar el siguiente comando :
// g++ P1.cpp -o programa -pthread -> Compilar
// ./Programa -> Ejecutarlo
// cat output.txt -> Ver el contenido del archivo

#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

mutex m;

void escribir(ofstream &archivo, int id) {
  lock_guard<mutex> lock(m);
  archivo << "Mensaje del Thread " << id << endl;
}

int main() {
  ofstream archivo("output.txt");

  if (!archivo.is_open()) {
    cout << "Error al abrir el archivo." << endl;
    return 1;
  }

  thread t1(escribir, ref(archivo), 1);
  thread t2(escribir, ref(archivo), 2);
  thread t3(escribir, ref(archivo), 3);
  thread t4(escribir, ref(archivo), 4);
  thread t5(escribir, ref(archivo), 5);

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  archivo.close();
  return 0;
}