#include <ncurses.h>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <thread>
#include <atomic>

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
    // Inicializa NCurses
    initscr();            // Inicia la pantalla
    noecho();             // No muestra entrada del teclado automáticamente
    cbreak();             // Habilita la entrada inmediata
    curs_set(1);          // Muestra el cursor
    start_color();        // Habilita colores

    // Configuración de colores
    init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Color para el área superior
    init_pair(2, COLOR_WHITE, COLOR_GREEN);  // Color para el área central
    init_pair(3, COLOR_WHITE, COLOR_RED);    // Color para el área inferior

    // Tamaño de la pantalla
    int rows, cols;
    getmaxyx(stdscr, rows, cols); // Obtiene las dimensiones de la pantalla

    // Definir las alturas de las tres áreas
    int topHeight = 3;              // Altura del área superior
    int bottomHeight = 3;           // Altura del área inferior
    int middleHeight = rows - topHeight - bottomHeight; // El resto es para el área central

    // Crear las ventanas para cada área
    WINDOW* topWin = newwin(topHeight, cols, 0, 0);
    WINDOW* middleWin = newwin(middleHeight, cols, topHeight, 0);
    WINDOW* bottomWin = newwin(bottomHeight, cols, topHeight + middleHeight, 0);

    // Configurar y mostrar el área superior
    wbkgd(topWin, COLOR_PAIR(1)); // Fondo azul
    box(topWin, 0, 0);           // Borde alrededor de la ventana
    mvwprintw(topWin, 1, (cols - 16) / 2, "Sección Superior");
    wrefresh(topWin);

    // Configurar y mostrar el área central
    wbkgd(middleWin, COLOR_PAIR(2)); // Fondo verde
    box(middleWin, 0, 0);           // Borde alrededor de la ventana
    wrefresh(middleWin);

    // Configurar el área inferior
    wbkgd(bottomWin, COLOR_PAIR(3)); // Fondo rojo
    box(bottomWin, 0, 0);           // Borde alrededor de la ventana
    mvwprintw(bottomWin, 0, 2, "Introduce texto aquí: ");
    wrefresh(bottomWin);

    // Habilitar escritura en la sección inferior
    int ch;
    std::string input;
    int cursor_x = 21; // Cursor después de "Introduce texto aquí: "

    keypad(bottomWin, true); // Habilitar entradas especiales en la ventana inferior

    while (true) {
        // Posicionar el cursor en la ventana inferior para entrada
        wmove(bottomWin, 1, cursor_x);
        wrefresh(bottomWin);

        /*******************************************/
        // Lee el carácter ingresado
        ch = wgetch(bottomWin);

        /*******************************************/

        if (ch == KEY_BACKSPACE || ch == 127) { // Manejo de tecla de retroceso
            if (!input.empty() && cursor_x > 21) {
                input.pop_back();
                cursor_x--;
                mvwprintw(bottomWin, 1, cursor_x, " "); // Borra el carácter en pantalla
                wmove(bottomWin, 1, cursor_x);
            }
        } else if (ch == '\n') { // Enter para procesar la entrada
            if (input == "quit") {
                break; // Salir del bucle si el usuario escribe "quit"
            }
            // Se ha pulsado enter, y se ejecuta el comando


            // Mostrar el texto ingresado en la sección central
            static int middle_y = 1; // Posición en la ventana central
            if (middle_y >= middleHeight - 1) { // Si se llena el área central
                werase(middleWin);             // Limpia la ventana central
                box(middleWin, 0, 0);          // Redibuja el marco
                middle_y = 1;
            }
            
            //mvwprintw(middleWin, middle_y++, 2, "%s", input.c_str());
            wrefresh(middleWin);

            // Limpiar la entrada para nueva escritura
            input.clear();
            for (int i = 21; i < cols - 1; ++i) { // Borra la línea de entrada
                mvwprintw(bottomWin, 1, i, " ");
            }
            cursor_x = 21; // Restablecer la posición del cursor
        } else if (isprint(ch)) { // Si el carácter es imprimible
            input.push_back(ch);
            mvwaddch(bottomWin, 1, cursor_x++, ch);
        }

        wrefresh(bottomWin);
    }

    // Muestra mensaje de despedida en la sección inferior
    werase(bottomWin);
    box(bottomWin, 0, 0);
    mvwprintw(bottomWin, 1, 2, "¡Adiós! Has salido del programa.");
    wrefresh(bottomWin);

    // Esperar entrada para salir
    getch();

    // Finalizar NCurses
    delwin(topWin);
    delwin(middleWin);
    delwin(bottomWin);
    endwin();

    return 0;
}