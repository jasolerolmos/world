#include <iostream>
#include <string>
#include <thread>
#include <sstream> // Para stringstream
#include <vector> // Para vector
#include "Client.h"
#include <nlohmann/json.hpp>


std::pair<std::string, std::string> parseCommand(const std::string& input) {
  std::istringstream iss(input);
  std::string command;
  std::string value;
  
  // Extraer el comando (la primera palabra)
  if (!(iss >> command)) {
    return {"", ""}; // No hay comando
  }
    
  // Convertir el comando a minúsculas para comparaciones sin distinción de mayúsculas y minúsculas
  for (char &c : command) {
        c = std::tolower(c);
  }

  // Extraer el valor (lo que hay después del comando)
  std::getline(iss >> std::ws, value);

  return {command, value};
}

nlohmann::json buildJson(const std::string& command, const std::string& value) {
    nlohmann::json j;
    if (command.length() == 1) {
        
        if (command == "g") {
            j["command"] = "move";
             
            if (value == "n") {
                j["direction"] = "north";
            } else if (value == "s") {
                j["direction"] = "south";
            } else if (value == "e") {
                j["direction"] = "east";
            } else if (value == "o") {
               j["direction"] = "west";
            } else {
                 j["error"] = "invalid direction: " + value;
            }

        } else if (command == "i") {
            j["command"] = "info";
            j["target"] = "player";
        } else if (command == "t") {
            j["command"] = "info";
             j["target"] = "position";
        } else if (command == "m") {
            j["command"] = "message";
            j["text"] = value;
        } else {
             j["error"] = "unknown command: " + command;
        }

    } else if (command == "exit") {
         j["command"] = "exit";

    } else {
         j["error"] = "unknown command: " + command;
    }
    

    return j;
}


int main() {
    try {
        Client client("127.0.0.1", 5050);

        // Lanzar un hilo para recibir mensajes del servidor
        std::thread receiver_thread(&Client::receiveMessages, &client);

        std::string input;
        while (true) {
            std::cout << "> ";
            std::getline(std::cin, input); // Leer toda la línea de entrada

            std::stringstream ss(input);
            std::string comando;
            ss >> comando;

            nlohmann::json estructuraJson;
            std::string valor;

            if (comando == "g") {
                 if (ss >> valor) {
                    estructuraJson["go"] = valor;
                 } else{
                    std::cerr << "Error: Se necesita un valor para el comando 'g'.\n";
                    continue;
                 }
            } else if (comando == "i") {
                estructuraJson["info"] = "";
            } else if (comando == "m") {
                std::getline(ss >> std::ws, valor); // Leer el resto de la línea
                 if (!valor.empty()) {
                     estructuraJson["mapa"] = valor;
                 }else{
                      std::cerr << "Error: Se necesita un valor para el comando 'm'.\n";
                     continue;
                 }
            } else if (comando == "p") {
                int x, y;
                 if (ss >> x >> y) {
                    estructuraJson["punto"] = {x, y};
                     std::string request = estructuraJson.dump() + "\n";
                     std::cout << "Enviando JSON: " << request << std::endl;
                 } else {
                     std::cerr << "Error: Se necesitan dos valores enteros para el comando 'p'.\n";
                     continue;
                 }
            } else if (comando == "d") {
                 if(ss >> valor){
                     estructuraJson["donde"] = valor;
                 }else{
                     std::cerr << "Error: Se necesita un valor para el comando 'd'.\n";
                     continue;
                 }
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
        receiver_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }

    return 0;
}