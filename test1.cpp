#include <iostream>
#include <string>
#include <nlohmann/json.hpp> // Biblioteca JSON

using json = nlohmann::json;

int main() {
    std::cout << "Introduce comandos en el formato:\n";
    std::cout << "  g <direccion> - Mover al personaje (direccion: n, s, e, o).\n";
    std::cout << "  i - Obtener información del personaje.\n";
    std::cout << "  t - Obtener información de la posición.\n";
    std::cout << "  m <mensaje> - Enviar un mensaje.\n";
    std::cout << "  exit - Salir del programa.\n";

    while (true) {
        std::string comando, valor;
        std::cout << "> ";
        std::cin >> comando; // Leer el comando principal

        json estructuraJson;

        if (comando == "g") { // Comando para mover
            std::cin >> valor; // Leer la dirección (n, s, e, o)
            estructuraJson["g"] = valor;

        } else if (comando == "i") { // Comando para obtener información del personaje
            estructuraJson["i"] = ""; // No requiere valor adicional

        } else if (comando == "t") { // Comando para obtener información de la posición
            estructuraJson["t"] = ""; // No requiere valor adicional

        } else if (comando == "m") { // Comando para enviar un mensaje
            std::getline(std::cin >> std::ws, valor); // Leer el mensaje completo
            estructuraJson["m"] = valor;

        } else if (comando == "exit") { // Comando para salir
            estructuraJson["exit"] = ""; // No requiere valor adicional
            std::cout << "Generando JSON:\n" << estructuraJson.dump(4) << std::endl;
            break; // Salir del bucle

        } else { // Comando no reconocido
            std::cerr << "Comando no reconocido. Intenta de nuevo.\n";
            continue; // Ir a la siguiente iteración
        }

        // Imprimir el JSON generado
        std::cout << "Generando JSON:\n" << estructuraJson.dump(4) << std::endl;
    }

    std::cout << "Programa finalizado.\n";
    return 0;
}