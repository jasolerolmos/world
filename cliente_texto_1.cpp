#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <atomic>
#include "Client.h"


void receiveMessages(Client& client) {
    try {
        while (true) {
            std::string message = client.receive();
            if (message.empty()) {
                break; // Conexión cerrada o error
            }
            std::cout << "Mensaje recibido: " << message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error al recibir mensaje: " << e.what() << std::endl;
    }
}


int main() {
    try {
        Client client("127.0.0.1", 5050);  // Dirección y puerto del servidor

        // Lanzar un hilo para recibir mensajes del servidor
        std::thread receiver_thread(receiveMessages, std::ref(client));

        std::string message;
        while (true) {
            std::string comando, valor;
            std::cout << "> ";
            std::cin >> comando;
            nlohmann::json estructuraJson;

            if (comando == "g") {
                std::cin >> valor;
                estructuraJson["go"] = valor;
            } else if (comando == "i") {
                estructuraJson["info"] = "";
            } else if (comando == "m") {
                std::getline(std::cin >> std::ws, valor);
                estructuraJson["mapa"] = valor;
            } else if (comando == "p") {
                int x, y;
                std::cin >> x >> y;
                estructuraJson["punto"] = {x, y};
                std::cout << "Enviando JSON: " << estructuraJson.dump() << std::endl;
            } else if (comando == "d") {
                std::cin >> valor;
                estructuraJson["donde"] = valor;
            } else if (comando == "exit") {
                estructuraJson["exit"] = "";
                std::cout << "Generando JSON:\n" << estructuraJson.dump(4) << std::endl;
                break;
            } else {
                std::cerr << "Comando no reconocido. Intenta de nuevo.\n";
                continue;
            }


            client.sendJSON(estructuraJson);
        }
        client.closeConnection();
        receiver_thread.join(); // Esperar a que el hilo termine
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }

    return 0;
}