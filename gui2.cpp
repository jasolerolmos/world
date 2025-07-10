#include <iostream>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <atomic>
#include "Client.h"
#include <ncurses.h>
#include <string>
#include <vector> 


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
                j["go"] = "n";
            } else if (value == "s") {
                j["go"] = "s";
            } else if (value == "e") {
                j["go"] = "e";
            } else if (value == "o") {
               j["go"] = "w";
            } else {
                 j["error"] = "invalid go: " + value;
            }

        } else if (command == "i") {
            j["command"] = "info";
            j["target"] = "player";
        } else if (command == "d") {
            j["command"] = "info";
             j["donde"] = "";
        } else if (command == "p") {
            j["command"] = "message";
            int x = 0, y = 0;

            size_t pos = value.find(','); // Encuentra la posición de la coma
            if (pos != std::string::npos) {
                x = std::stoi(value.substr(0, pos)); // Parte antes de la coma
                y = std::stoi(value.substr(pos + 1)); // Parte después de la coma

                j["punto"] = {x, y};;
            } else {
                j["donde"] = "";
            }
        } else if (command == "t" ) {
            j["command"] = "teleport"; 
            int x = 0, y = 0;

            size_t pos = value.find(' '); // Encuentra la posición de la coma
            if (pos != std::string::npos) {
                x = std::stoi(value.substr(0, pos)); // Parte antes de la coma
                y = std::stoi(value.substr(pos + 1)); // Parte después de la coma
            }
            j["tl"] = {x, y};

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

// Función para inicializar y configurar NCurses
void initNcurses() {
    initscr();            // Inicia la pantalla
    noecho();             // No muestra entrada del teclado automáticamente
    cbreak();             // Habilita la entrada inmediata
    curs_set(1);          // Muestra el cursor
    start_color();        // Habilita colores

    // Configuración de colores
    init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Color para el área superior
    init_pair(2, COLOR_WHITE, COLOR_BLACK);  // Color para el área central
    init_pair(3, COLOR_WHITE, COLOR_RED);    // Color para el área inferior
}

void handleInput(WINDOW* bottomWin, WINDOW* middleWin, int cols, int rows, std::vector<std::string>& messages) {
    std::string input;
    int cursor_x = 21; // Cursor después de "Introduce texto aquí: "
    int middle_y = 1; // Posición en la ventana central
    int ch;
    std::mutex win_mutex; // Mutex para sincronizar acceso a middleWin
    std::cout << "Rows: " << rows << std::endl;
    try {
        Client client("127.0.0.1", 5050);
        std::pair<int, int> vw = {(cols/5)-1, rows - 9};
        client.setWinView(vw);

        // Hilo para recibir mensajes del servidor
        std::thread receiver_thread([&]() {
            client.receiveMessages([&](const std::string& message) {
                std::lock_guard<std::mutex> lock(win_mutex);
                if (middle_y >= getmaxy(middleWin) - 1) {   
                    werase(middleWin);
                    box(middleWin, 0, 0);
                    middle_y = 1;
                }

                werase(middleWin);
                wrefresh(middleWin);

                std::istringstream stream(message);
                std::string line;

                while (std::getline(stream, line)) {
                    //std::cout << "Línea: " << line << std::endl; // Procesar cada línea
                    mvwprintw(middleWin, middle_y++, 2, "%s", line.c_str());
                }   
                middle_y = 1;
                
                wrefresh(middleWin);
            });
        });
        
        auto [command, value] = parseCommand("info info");
        nlohmann::json sendInfo;
        sendInfo["heroe"] = "personaje";
        sendInfo["viewPort"] = {client.getWinView().first, client.getWinView().second};
        sendInfo["donde"] = "";
        client.sendJSON(sendInfo);

        

        while (true) {
            wmove(bottomWin, 1, cursor_x);
            wrefresh(bottomWin);
            ch = wgetch(bottomWin);

            if (ch == KEY_BACKSPACE || ch == 127) {
                if (!input.empty() && cursor_x > 21) {
                    input.pop_back();
                    cursor_x--;
                    mvwprintw(bottomWin, 1, cursor_x, " ");
                    wmove(bottomWin, 1, cursor_x);
                }
            } else if (ch == '\n') {
                if (input == "quit") {
                    break;
                }
                auto [command, value] = parseCommand(input);
                nlohmann::json estructuraJson = buildJson(command, value);
                client.sendJSON(estructuraJson);
                
                std::lock_guard<std::mutex> lock(win_mutex); // Bloquea el mutex
                if (middle_y >= getmaxy(middleWin) - 1) { // Si se llena el área central
                    werase(middleWin);                    // Limpia la ventana central
                    box(middleWin, 0, 0);                 // Redibuja el marco
                    middle_y = 1;                         // Reinicia el contador de líneas
                }
                //std::string output = "[Cliente]: " + input;
                //mvwprintw(middleWin, middle_y++, 2, "%s", output.c_str());
                //wrefresh(middleWin);

                messages.push_back(input);
                input.clear();
                for (int i = 21; i < cols - 1; ++i) { // Borra la línea de entrada
                    mvwprintw(bottomWin, 1, i, " ");
                }
                cursor_x = 21; // Restablecer la posición del cursor
            }  else if (ch == KEY_UP) {
                input = "g n";
                auto [command, value] = parseCommand(input);
                nlohmann::json estructuraJson = buildJson(command, value);
                client.sendJSON(estructuraJson);
                cursor_x = 21; // Restablecer la posición del cursor
                input.clear();
            } else if (ch == KEY_DOWN) {
                input = "g s";
                auto [command, value] = parseCommand(input);
                nlohmann::json estructuraJson = buildJson(command, value);
                client.sendJSON(estructuraJson);
                cursor_x = 21; // Restablecer la posición del cursor
                input.clear();
            } else if (ch == KEY_RIGHT) {
                input = "g e";
                auto [command, value] = parseCommand(input);
                nlohmann::json estructuraJson = buildJson(command, value);
                client.sendJSON(estructuraJson);
                cursor_x = 21; // Restablecer la posición del cursor
                input.clear();
            } else if (ch == KEY_LEFT) {
                input = "g o";
                auto [command, value] = parseCommand(input);
                nlohmann::json estructuraJson = buildJson(command, value);
                client.sendJSON(estructuraJson);
                cursor_x = 21; // Restablecer la posición del cursor
                input.clear();
            } else if (isprint(ch)) {
                input.push_back(ch);
                mvwaddch(bottomWin, 1, cursor_x++, ch);
            }
            wrefresh(bottomWin);
        }

        client.closeConnection();
        receiver_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }
}


// Función para crear las ventanas y dibujar sus bordes y títulos
std::tuple<WINDOW*, WINDOW*, WINDOW*> setupWindows(int rows, int cols) {
    int topHeight = 3;
    int bottomHeight = 3;
    int middleHeight = rows - topHeight - bottomHeight;

    WINDOW* topWin = newwin(topHeight, cols, 0, 0);
    WINDOW* middleWin = newwin(middleHeight, cols, topHeight, 0);
    WINDOW* bottomWin = newwin(bottomHeight, cols, topHeight + middleHeight, 0);

    // Configurar el área superior
    wbkgd(topWin, COLOR_PAIR(1));
    box(topWin, 0, 0);
    mvwprintw(topWin, 1, (cols - 16) / 2, "Sección Superior");
    wrefresh(topWin);

    // Configurar el área central
    wbkgd(middleWin, COLOR_PAIR(2));
    mvwprintw(middleWin, 0, 2, "Introduce texto aquí: ");
    box(middleWin, 0, 0);
    wrefresh(middleWin);
    
     // Configurar el área inferior
    wbkgd(bottomWin, COLOR_PAIR(3));
    box(bottomWin, 0, 0);
    mvwprintw(bottomWin, 0, 2, "Introduce texto aquí: ");
    keypad(bottomWin, true);
    wrefresh(bottomWin);

    return std::make_tuple(topWin, middleWin, bottomWin);
}

/*
void handleInput(WINDOW* bottomWin, WINDOW* middleWin, int cols, std::vector<std::string>& messages) {
        
    std::string input;
    int cursor_x = 21; // Cursor después de "Introduce texto aquí: "
    int middle_y = 1; // Posición en la ventana central
    int ch;
    try {
        Client client("127.0.0.1", 5050); 
        std::thread receiver_thread(&Client::receiveMessages, &client);
        std::string message;

        while (true) {
            // Posicionar el cursor en la ventana inferior para entrada
            wmove(bottomWin, 1, cursor_x);
            wrefresh(bottomWin);
            ch = wgetch(bottomWin);

            if (ch == KEY_BACKSPACE || ch == 127) {
                if (!input.empty() && cursor_x > 21) {
                    input.pop_back();
                    cursor_x--;
                    mvwprintw(bottomWin, 1, cursor_x, " ");
                    wmove(bottomWin, 1, cursor_x);
                }
            } else if (ch == '\n') {
            if (input == "quit") {
                    break; 
                }
                // Mostrar el texto ingresado en la sección central
                if (middle_y >= getmaxy(middleWin) - 1) { // Si se llena el área central
                    werase(middleWin);                // Limpia la ventana central
                    box(middleWin, 0, 0);            // Redibuja el marco
                    middle_y = 1;                      // Reinicia el contador de líneas
                }

                auto [command, value] = parseCommand(input);
                nlohmann::json estructuraJson = buildJson(command, value);
                std::string output =  "JSON: " + estructuraJson.dump(4) + "\n";
                //std::cout << "JSON: " << estructuraJson.dump(4) << "\n";
                mvwprintw(middleWin, middle_y++, 2, "%s", output.c_str());

                //std::string request = comando + " " + valor + "\n";
                client.sendJSON(estructuraJson);
                
                wrefresh(middleWin);
                
                messages.push_back(input);
                // Limpiar la entrada para nueva escritura
                input.clear();
                for (int i = 21; i < cols - 1; ++i) { // Borra la línea de entrada
                    mvwprintw(bottomWin, 1, i, " ");
                }
                cursor_x = 21; // Restablecer la posición del cursor
            } else if (isprint(ch)) {
                input.push_back(ch);
                mvwaddch(bottomWin, 1, cursor_x++, ch);
            }
            wrefresh(bottomWin);
        }
        client.closeConnection();
        receiver_thread.join();
    } catch (const std::exception& e) {
        std::cerr << "Excepción: " << e.what() << std::endl;
    }
}
*/
void endProgram(WINDOW* topWin, WINDOW* middleWin, WINDOW* bottomWin) {
    werase(bottomWin);
    box(bottomWin, 0, 0);
    mvwprintw(bottomWin, 1, 2, "¡Adiós! Has salido del programa.");
    wrefresh(bottomWin);
    getch(); 

    delwin(topWin);
    delwin(middleWin);
    delwin(bottomWin);
    endwin();
}

int main() {
    initNcurses();
    int rows, cols;
    getmaxyx(stdscr, rows, cols); // Obtiene las dimensiones de la pantalla
     auto [topWin, middleWin, bottomWin] = setupWindows(rows, cols);
     std::vector<std::string> messages;

     handleInput(bottomWin,middleWin, cols, rows, messages);

     endProgram(topWin,middleWin,bottomWin);
    return 0;
}