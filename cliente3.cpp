#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <atomic>
#include "Client.h"

void processMessage (const std::string& msg) {
    std::cout << "Recibido: " << msg << std::endl;
}

int main() {
    try {
        Client client("127.0.0.1", 5050);  // Dirección y puerto del servidor

        // Lanzar un hilo para recibir mensajes del servidor
        //std::thread receiver_thread(&Client::receiveMessages, &client);
        std::thread receiver_thread(&Client::receiveMessages, &client, [&](const std::string& msg){
                std::cout << "Procesando mensaje: " << msg << " en el cliente " << &client << "\n";
        });
        
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
                std::string request = estructuraJson.dump() + "\n";
                std::cout << "Enviando JSON: " << request << std::endl;
            } else if (comando == "d") {
                estructuraJson["donde"] = valor;
            } else if (comando == "exit") {
                estructuraJson["exit"] = "";
                std::cout << "Generando JSON:\n" << estructuraJson.dump(4) << std::endl;
                break;
            } else {
                std::cerr << "Comando no reconocido. Intenta de nuevo.\n";
                continue;
            }

            std::string request = comando + " " + valor + "\n";

            client.sendJSON(estructuraJson);
        }
        client.closeConnection();
        //receiver_thread.join(); // Esperar a que el hilo termine
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }

    return 0;
}