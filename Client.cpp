#include "Client.h"

using boost::asio::ip::tcp;

// Constructor
Client::Client(const std::string& host, short port) 
    : socket(io_service), running(true) {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, std::to_string(port));
    boost::asio::connect(socket, resolver.resolve(query));
}

// Implementación de sendJSON
void Client::sendJSON(const nlohmann::json request_json) {
    std::string request = request_json.dump() + "\n";
    std::string enviado = "Enviando JSON: " + request + "\n";
    boost::asio::write(socket, boost::asio::buffer(request));
}

void Client::setWinView(std::pair<int, int> wv) {
    winView = wv;
}
std::pair<int, int> Client::getWinView() {
    return winView;
}
// Implementación de sendRequest
void Client::sendRequest(const std::string& message) {
    nlohmann::json request_json;
    //std::cout << "sendRequest: " << message << std::endl;

    if (message.find("g") == 0) {
        std::string direccion;
        std::istringstream iss(message.substr(2));
        iss >> direccion;
        //std::cout << direccion << std::endl;
        request_json["go"] = direccion;
        request_json["info"] = "info";
    } else if (message.find("i") == 0) {
        request_json["info"] = "info";
    } else if (message.find("t") == 0) {
        std::istringstream iss(message.substr(2));
        int x, y;
        if (iss >> x >> y) {
            request_json["punto"] = {x, y};
        } else {
            std::cerr << "Error: coordenadas no válidas" << std::endl;
            return;
        }
    } else {
        request_json["msg"] = message;
    }
    sendJSON(request_json);
}

void Client::receiveMessages(std::function<void(const std::string&)> callback) {
    try {
        while (running) {
            boost::asio::streambuf buf;
            boost::asio::read_until(socket, buf, "\n");
            std::istream input(&buf);
            nlohmann::json response_json;
            input >> response_json;
            std::string message = "";

            if (response_json.contains("mapa")) {
                auto nuevaPosicion = response_json["centro"];

                int x = nuevaPosicion[0];
                int y = nuevaPosicion[1];
            
                message = "[Servidor]: Nos manda el mapa";
                std::ostringstream output; // String stream para construir el texto
                
                // Número de columnas
                int num_columns = response_json["mapa"][0].size();
                int num_rows = response_json["mapa"].size();

                int line_number = y-(num_rows/2);
                int col_number = x-(num_columns/2);

                if (line_number < 0 ) line_number = 0;
                if (col_number < 0 ) col_number = 0;

                // Imprimir cabecera con el número de columna
                output << "    "; // Espaciado para alinear con los números de fila
                for (int col = 0; col < num_columns; ++col) {
                    output << std::setw(5) << std::setfill(' ') << (col_number + col); // Columna numerada desde 1
                }
                output << "\n";
 
                for (const auto& row : response_json["mapa"]) {
                    output << std::setw(4) << std::setfill('0') << line_number++ << " ";
                    for (int code : row) {
                        if (code == -1)
                            output << code; // Fuera de límites
                        else {
                            //output << "  " << (code/10) << "  ";
                            
                            if (code == 99) {
                                //output << "@";
                                output << "  " << "@" << "  ";
                            } else if (code < 10) {
                                output << "  " << static_cast<char>(176) << "  ";
                            } else if (code < 20) {
                                //output << "-";
                                output << "  " << "-" << "  ";
                            } else if (code < 30) {
                                //output << "+";
                                output << "  " << "+" << "  ";
                            } else if (code < 40) {
                                //output << "#";
                                output << "  " << "#" << "  ";
                            } else if (code < 40) {
                                //output << "";
                                output << "  " << "Q" << "  ";
                            } else {
                                //output << "";
                                output << "  " << "&" << "  ";
                            }
                            
                        }
                    }
                    output << "\n"; // Nueva línea después de cada fila
                }

                message = output.str(); 
            } else {
                message = "[Servidor]: " + response_json.dump();
            }
            callback(message); // Llamar al callback con el mensaje recibido
        }
    } catch (const std::exception& e) {
        if (running) {
            std::cerr << "\nError al recibir mensajes: " << e.what() << std::endl;
        }
    }
}

void Client::closeConnection() {
    running = false;
    socket.close();
}