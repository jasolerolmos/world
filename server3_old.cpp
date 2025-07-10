#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include "World.h" // Incluir el código del mundo procedural

using boost::asio::ip::tcp;

class Server {
private:
    boost::asio::io_service io_service;
    tcp::acceptor acceptor;
    tcp::socket socket;
    World world;

public:
    Server(short port) : acceptor(io_service, tcp::endpoint(tcp::v4(), port)), socket(io_service), world(1024, 1024) {
        world.createWorld(128.0, 6, 0.5, 2.0);  // Generar el mundo procedural
    }

    void start() {
        std::cout << "Servidor iniciado en el puerto 5050" << std::endl;

        while (true) {
            // Aceptar una conexión del cliente
            acceptor.accept(socket);
            std::cout << "Cliente conectado" << std::endl;

            try {
                // Recibir datos del cliente
                boost::asio::streambuf buf;
                boost::asio::read_until(socket, buf, "\n");

                std::istream input(&buf);
                nlohmann::json request_json;
                input >> request_json; // Parsear el JSON recibido

                // Procesar la solicitud del cliente
                if (request_json.contains("mapa")) {
                    // Obtener las coordenadas del JSON
                    auto mapa = request_json["mapa"];
                    int x = mapa[0];
                    int y = mapa[1];

                    // Obtener el tipo de terreno de esa coordenada
                    std::string tipo = world.getTerrainTypeAt(x, y);

                    // Crear la respuesta
                    nlohmann::json response_json;
                    response_json["tipo"] = tipo;

                    // Enviar la respuesta al cliente
                    std::string response = response_json.dump() + "\n";
                    boost::asio::write(socket, boost::asio::buffer(response));
                }

                std::cout << "Mensaje" << std::endl;
                
                socket.close(); // Cerrar la conexión
            }
            catch (std::exception& e) {
                std::cerr << "Error durante la comunicación con el cliente: " << e.what() << std::endl;
                socket.close();
            }
        }
    }
};

int main() {
    try {
        int width = 2048, height = 2048; 
        double scale = 200.0;
        int octaves = 10;
        double persistence = 0.5, lacunarity = 2.0;

        World world(width, height);
        world.createWorld(scale, octaves, persistence, lacunarity);
        world.saveImage("world_image.png");
        Server server(5050); // Servidor escuchando en el puerto 12345
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }

    return 0;
}