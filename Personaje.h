#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <string>
#include <utility> // Para std::pair

class Personaje {
private:
    std::string nombre;          // Nombre del personaje
    std::pair<int, int> coordenadas; // Coordenadas en (x, y)
    double dinero;               // Dinero disponible
    int experiencia;             // Experiencia acumulada
    int water = 0;
    std::pair<int, int> winView; // Coordenadas en (x, y)

public:
    // Constructor
    Personaje(const std::string& nombre, int x, int y, double dinero, int experiencia);

    // Métodos para obtener las propiedades
    const std::string& getNombre() const;
    std::pair<int, int> getCoordenadas() const;
    double getDinero() const;
    int getExperiencia() const;

    void setViewPort(std::pair<int, int> wv);
    std::pair<int, int> getViewPort();
    // Métodos para modificar las propiedades
    void mover(int x, int y);
    void agregarDinero(double cantidad);
    void gastarDinero(double cantidad);
    void ganarExperiencia(int puntos);

    int swimmLevel();

    // Método para mostrar información del personaje
    void mostrarInfo() const;

    void desplazar(const std::string& direccion, int casillas);
    
    void guardarEnArchivo(const std::string& nombreArchivo) const;

    void cargarDesdeArchivo(const std::string& nombreArchivo);

};

#endif // PERSONAJE_H