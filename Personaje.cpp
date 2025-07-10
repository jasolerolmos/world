#include "Personaje.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

// Constructor
Personaje::Personaje(const std::string& nombre, int x, int y, double dinero, int experiencia)
    : nombre(nombre), coordenadas(x, y), dinero(dinero), experiencia(experiencia) {}

// Métodos para obtener las propiedades
const std::string& Personaje::getNombre() const { return nombre; }
std::pair<int, int> Personaje::getCoordenadas() const { return coordenadas; }
double Personaje::getDinero() const { return dinero; }
int Personaje::getExperiencia() const { return experiencia; }

// Métodos para modificar las propiedades
void Personaje::mover(int x, int y) {
    coordenadas = {x, y};
    std::cout << nombre << " se ha movido a (" << x << ", " << y << ").\n";
}

void Personaje::agregarDinero(double cantidad) {
    if (cantidad < 0) {
        std::cerr << "No puedes agregar una cantidad negativa de dinero.\n";
    } else {
        dinero += cantidad;
        std::cout << nombre << " ahora tiene $" << dinero << ".\n";
    }
}

void Personaje::gastarDinero(double cantidad) {
    if (cantidad > dinero) {
        std::cerr << "No hay suficiente dinero para gastar.\n";
    } else if (cantidad < 0) {
        std::cerr << "No puedes gastar una cantidad negativa de dinero.\n";
    } else {
        dinero -= cantidad;
        std::cout << nombre << " ha gastado $" << cantidad << ". Dinero restante: $" << dinero << ".\n";
    }
}

 int Personaje::swimmLevel() {
    return water;
 }

void Personaje::ganarExperiencia(int puntos) {
    if (puntos < 0) {
        std::cerr << "No puedes ganar experiencia negativa.\n";
    } else {
        experiencia += puntos;
        std::cout << nombre << " ha ganado " << puntos << " puntos de experiencia. Experiencia total: " << experiencia << ".\n";
    }
}

// Método para mostrar información del personaje
void Personaje::mostrarInfo() const {
    std::cout << "Nombre: " << nombre << "\n";
    std::cout << "Coordenadas: (" << coordenadas.first << ", " << coordenadas.second << ")\n";
    std::cout << "Dinero: $" << dinero << "\n";
    std::cout << "Experiencia: " << experiencia << "\n";
}

void Personaje::desplazar(const std::string& direccion, int casillas) {
    /*
    if (casillas < 0) {
        std::cerr << "El número de casillas debe ser positivo.\n";
        return;
    }
    */
    
    if (direccion == "n") {
        coordenadas.second -= casillas; // Desplazar hacia arriba disminuye Y
    } else if (direccion == "s") {
        coordenadas.second += casillas; // Desplazar hacia abajo aumenta Y
    } else if (direccion == "w") {
        coordenadas.first -= casillas; // Desplazar hacia la izquierda disminuye X
    } else if (direccion == "e") {
        coordenadas.first += casillas; // Desplazar hacia la derecha aumenta X
    } else {
        std::cerr << "Dirección no válida. Usa: arriba, abajo, izquierda o derecha.\n";
        return;
    }
    
    std::cout << nombre << " se ha desplazado " << casillas << " casillas hacia " << direccion << ". Nueva posición: (" 
              << coordenadas.first << ", " << coordenadas.second << ")" << std::endl;
    
}

void Personaje::guardarEnArchivo(const std::string& nombreArchivo) const {
    std::ofstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo) {
        throw std::runtime_error("No se pudo abrir el archivo para escritura.");
    }

    size_t longitudNombre = nombre.size();
    archivo.write(reinterpret_cast<const char*>(&longitudNombre), sizeof(longitudNombre));
    archivo.write(nombre.data(), longitudNombre);
    archivo.write(reinterpret_cast<const char*>(&coordenadas.first), sizeof(coordenadas.first));
    archivo.write(reinterpret_cast<const char*>(&coordenadas.second), sizeof(coordenadas.second));
    archivo.write(reinterpret_cast<const char*>(&dinero), sizeof(dinero));
    archivo.write(reinterpret_cast<const char*>(&experiencia), sizeof(experiencia));
    archivo.write(reinterpret_cast<const char*>(&water), sizeof(water));

    archivo.close();
}

void Personaje::cargarDesdeArchivo(const std::string& nombreArchivo) {
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo) {
        throw std::runtime_error("No se pudo abrir el archivo para lectura.");
    }

    size_t longitudNombre;
    archivo.read(reinterpret_cast<char*>(&longitudNombre), sizeof(longitudNombre));

    nombre.resize(longitudNombre);
    archivo.read(&nombre[0], longitudNombre);
    archivo.read(reinterpret_cast<char*>(&coordenadas.first), sizeof(coordenadas.first));
    archivo.read(reinterpret_cast<char*>(&coordenadas.second), sizeof(coordenadas.second));
    archivo.read(reinterpret_cast<char*>(&dinero), sizeof(dinero));
    archivo.read(reinterpret_cast<char*>(&experiencia), sizeof(experiencia));
    archivo.read(reinterpret_cast<char*>(&water), sizeof(water));

    archivo.close();
}

void Personaje::setViewPort(std::pair<int, int> wv) {
    winView = wv;
}

std::pair<int, int> Personaje::getViewPort() {
    return winView;
}