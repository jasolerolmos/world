#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include "World.h" // Incluir el código del mundo procedural
#include "Personaje.h"
#include <random>
#include <filesystem>

using boost::asio::ip::tcp;

bool empiezaCon(const std::string& cadena, const std::string& prefijo) {
    // Verifica si el prefijo se encuentra al inicio de la cadena
    return cadena.find(prefijo) == 0;
}

std::vector<std::vector<int>> getMapaToShow(World world, std::pair<int, int> coordenadas, std::pair<int, int> winViewport) {
    std::vector<std::vector<int>> grid = world.getBiomaGrid(coordenadas, winViewport);
    int rows = grid.size();
    int cols = grid[0].size();

    // Coordenadas del punto central
    int central_row = rows / 2;
    int central_col = cols / 2;

    
    if (coordenadas.second < central_row) {
        central_row = coordenadas.second;
    }

    if (coordenadas.first < central_col) {
        central_col = coordenadas.first;
    }
        
    grid[central_row][central_col] = 99;

    return grid;
}

bool canSwimm(World world, Personaje personaje, std::string direccion) {

    personaje.desplazar(direccion, 1);
    std::pair<int, int> coordenadas = personaje.getCoordenadas();
    double rawValue = world.getRawValue(coordenadas.first, coordenadas.second);
    if ( rawValue < 0.2 ) {
        if (personaje.swimmLevel()<2) 
            return false;
    } else  if (rawValue < 0.4 ) {
        if (personaje.swimmLevel()<1) 
            return false;
    }

    return true;
}

// Clase para manejar el servidor
class Server {
private:
    boost::asio::io_service io_service;
    tcp::acceptor acceptor;
    World world;
    std::vector<std::shared_ptr<tcp::socket>> clients;
    std::mutex clients_mutex;

    void handleClient(std::shared_ptr<tcp::socket> client_socket) {
        std::random_device rd;
        std::mt19937 gen(rd()); // Mersenne Twister como generador de números
        std::uniform_int_distribution<> distrib(0, 1024);
        
        //Personaje personaje("Héroe", distrib(gen), distrib(gen), 100.0, 0);
        Personaje personaje("Héroe", 85, 105, 100.0, 0);

        try {
            while (true) {
                boost::asio::streambuf buf;
                boost::asio::read_until(*client_socket, buf, "\n"); // Leer hasta un salto de línea
                std::istream input(&buf);
                
                nlohmann::json request_json;
                input >> request_json;
                std::cout << "Mensaje recibido (sin 'mapa'): " << request_json.dump() << std::endl;

                nlohmann::json response_json;

                if (request_json.contains("viewPort")) {
                    std::pair<int, int> viewPort(request_json["viewPort"][0], request_json["viewPort"][1]);
                    personaje.setViewPort(viewPort);
                }

                if (request_json.contains("heroe")) {
                    std::string persojaneFile = request_json["heroe"].get<std::string>() + ".dat";
                    if (std::filesystem::exists(persojaneFile)) {
                        personaje.cargarDesdeArchivo(persojaneFile);
                    } 
                }

                if (request_json.contains("info")) {
                    personaje.mostrarInfo();
                }

                if (request_json.contains("tl")) {
                    auto nuevaPosicion = request_json["tl"];
                    personaje.mover(nuevaPosicion[0], nuevaPosicion[1]);

                    std::pair<int, int> coordenadas = personaje.getCoordenadas(); 
                    std::string tipo = world.getTerrainName(coordenadas.first, coordenadas.second);

                    response_json["terreno"] = tipo;
                    response_json["centro"] = {coordenadas.first, coordenadas.second};
                    response_json["mapa"] = getJSONMap(world, coordenadas, personaje.getViewPort());

                    //std::cout << personaje.getViewPort().first << "-" << personaje.getViewPort().second << std::endl;
                }
                
                if (request_json.contains("go")) {
                    auto direccion = request_json["go"];

                    if ( canSwimm(world, personaje, direccion) ) {
                        personaje.desplazar(direccion, 1);
                    }

                    std::pair<int, int> coordenadas = personaje.getCoordenadas();
                    std::string tipo = world.getTerrainName(coordenadas.first, coordenadas.second);

                    response_json["terreno"] = tipo;
                    response_json["centro"] = {coordenadas.first, coordenadas.second};
                    response_json["mapa"] = getJSONMap(world, coordenadas, personaje.getViewPort());

                    //std::cout << "ViewPort: " << personaje.getViewPort().first << "-" << personaje.getViewPort().second << std::endl;
                    //std::cout << "Coordenadas: " << coordenadas.first << "-" << coordenadas.second << std::endl;
                }

                if (request_json.contains("donde")) {
                    std::pair<int, int> coordenadas = personaje.getCoordenadas(); 
                    std::string tipo = world.getTerrainName(coordenadas.first, coordenadas.second);

                    response_json["terreno"] = tipo;
                    response_json["mapa"] = getJSONMap(world, coordenadas, personaje.getViewPort());
                    response_json["centro"] = {coordenadas.first, coordenadas.second};
                    
                    //std::cout << coordenadas.first << " " << coordenadas.second << std::endl;
                    //std::cout << personaje.getViewPort().first << "-" << personaje.getViewPort().second << std::endl;
                    //std::cout << response_json.dump() << std::endl;
                }

                if (request_json.contains("punto")) {
                    // Obtener las coordenadas del JSON
                    auto mapa = request_json["punto"];
                    int x = mapa[0];
                    int y = mapa[1];
                    std::string tipo = world.getTerrainName(x, y);

                    // Incluir el tipo en la respuesta
                    response_json["terreno"] = tipo;
                } 
                
                if (request_json.contains("msg")) {
                    // Si el mensaje contiene el campo "msg", retransmitirlo a los demás clientes
                    std::string message = request_json["msg"];
                    broadcastMessage(request_json, client_socket);

                } 
                
                /*
                else {
                    // Si no tiene la variable "m", mostramos el JSON completo en pantalla
                    std::cout << "Mensaje recibido (sin 'mapa'): " << request_json.dump() << std::endl;
                }
                */

                // Responder con "cmd": "OK" para indicar que la solicitud fue procesada
                response_json["cmd"] = "OK";

                personaje.guardarEnArchivo("personaje.dat");
                // Enviar la respuesta al cliente
                std::string response = response_json.dump() + "\n";
                boost::asio::write(*client_socket, boost::asio::buffer(response));

                std::cout << "Respuesta enviada al cliente: " << response_json.dump() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error al manejar cliente: " << e.what() << std::endl;
        }

        // Si salimos del bucle, el cliente ha cerrado la conexión
        removeClient(client_socket);
    }

    void broadcastMessage(const nlohmann::json& message, std::shared_ptr<tcp::socket> sender) {
        std::lock_guard<std::mutex> lock(clients_mutex); // Bloqueamos el acceso a la lista de clientes
        for (const auto& client : clients) {
            if (client != sender && client->is_open()) {
                try {
                    std::string msg = message.dump() + "\n";
                    boost::asio::write(*client, boost::asio::buffer(msg));
                } catch (const std::exception& e) {
                    std::cerr << "Error al enviar mensaje a un cliente: " << e.what() << std::endl;
                }
            }
        }
    }

    void removeClient(std::shared_ptr<tcp::socket> client_socket) {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
        std::cout << "Cliente desconectado. Clientes restantes: " << clients.size() << std::endl;
    }

public:
    Server(short port, int width, int height, double scale, int octaves, double persistence, double lacunarity)
        : acceptor(io_service, tcp::endpoint(tcp::v4(), port)), world(width, height) {
        world.setSeed(120);
        world.createWorld(scale, octaves, persistence, lacunarity);
        std::string archivoMundo = "mundo.dat";
        world.guardarEnArchivo(archivoMundo);
        world.saveImage("world_image.png");
    }

    void createWorld(float x, float y, double scale, int octaves, double persistence, double lacunarity) {
    }

    void start() {
        std::cout << "Servidor iniciado en el puerto 5050" << std::endl;

        while (true) {
            // Aceptar una nueva conexión
            auto client_socket = std::make_shared<tcp::socket>(io_service);
            acceptor.accept(*client_socket);

            // Agregar el cliente a la lista
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.push_back(client_socket);
            }

            std::cout << "Nuevo cliente conectado. Total de clientes: " << clients.size() << std::endl;

            // Crear un hilo independiente para manejar al cliente
            std::thread(&Server::handleClient, this, client_socket).detach();
        }
    }

    nlohmann::json biomaGridToJson(const std::vector<std::vector<int>>& biomaGrid) {
        nlohmann::json jsonGrid;
        
        for (size_t i = 0; i < biomaGrid.size(); ++i) {
            nlohmann::json jsonRow;
            for (int biomeValue : biomaGrid[i]) {
                jsonRow.push_back(biomeValue);
            }
            jsonGrid.push_back(jsonRow);
        }

        nlohmann::json result;
        result["mapa"] = jsonGrid; // Añadimos el objeto jsonGrid a un objeto JSON llamado "mapa"
        
        return result;
    }

    nlohmann::json getJSONMap(World world, std::pair<int, int> coordenadas, std::pair<int, int> winViewport) {
        std::vector<std::vector<int>> grid = getMapaToShow(world, coordenadas, winViewport);                    
        nlohmann::json jsonOutput = biomaGridToJson(grid);
        return jsonOutput["mapa"];
    }
};

int main() {
    try {
        
        int width = 8192, height = 8192;
        double scale = 200.0;
        int octaves = 10;
        double persistence = 0.5, lacunarity = 2.0;
                
        // Servidor escuchando en el puerto 5050
        Server server(5050, 1024, 1024, scale, octaves, persistence, lacunarity);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }

    return 0;
}