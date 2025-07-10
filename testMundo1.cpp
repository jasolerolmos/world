#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <cstdlib> // Para std::atoi
#include <opencv2/opencv.hpp>
#include "World.h"

int main(int argc, char* argv[]) {
    // Verificar si se pasó un argumento adicional

    int width = 4096, height = 4096;
    double scale = 200.0;
    int octaves = 19;
    double persistence = 0.5, lacunarity = 2.0;

    World world(width, height);
    unsigned int seed = 0;

    srand(time(0));

    if (argc > 1) {
        std::string arg = argv[1]; // Obtener el primer argumento
        try {
            // Convertir el argumento a un entero
            seed = std::stoul(arg);

            // Verificar si es positivo
            if (seed > 0) {
                std::cout << "El argumento recibido es un entero positivo: " << seed << std::endl;
            } else {
                std::cerr << "El argumento recibido no es un entero positivo." << std::endl;
            }
        } catch (const std::invalid_argument&) {
            std::cerr << "El argumento recibido no es un entero válido." << std::endl;
        } catch (const std::out_of_range&) {
            std::cerr << "El argumento recibido está fuera del rango permitido para enteros." << std::endl;
        }
    } else {
        std::cout << "No se recibió ningún argumento. Usando valores por defecto." << std::endl;


        // Generar un número aleatorio unsigned int
        seed |= static_cast<unsigned int>(rand()) << 16;
        seed |= static_cast<unsigned int>(rand());

        std::cout << "Número aleatorio unsigned int: " << seed << std::endl;
    }

    world.setSeed(seed); // Semilla para reproducibilidad
    world.createWorld(scale, octaves, persistence, lacunarity);
    world.saveImage("world_test.png");

    return 0;
}