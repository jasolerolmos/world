#include "Personaje.h"

int main() {
    // Crear un personaje
    Personaje personaje("Héroe", 0, 0, 100.0, 0);

    // Mostrar información inicial
    personaje.mostrarInfo();

    // Mover al personaje
    personaje.mover(5, 10);

    // Agregar y gastar dinero
    personaje.agregarDinero(50.0);
    personaje.gastarDinero(30.0);

    // Ganar experiencia
    personaje.ganarExperiencia(100);
    
    personaje.desplazar("arriba", 5);  // Se desplaza 5 casillas hacia arriba
    personaje.desplazar("derecha", 3); // Se desplaza 3 casillas hacia la derecha

    // Mostrar información actualizada
    personaje.mostrarInfo();

    return 0;
}