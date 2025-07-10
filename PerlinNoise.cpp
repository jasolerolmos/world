#include "PerlinNoise.h"
#include <algorithm>
#include <cmath>
#include <random>

// Constructor
PerlinNoise::PerlinNoise() : seed(std::random_device{}()) {
    initializeGradients();
    initializePermutation();
}
PerlinNoise::PerlinNoise(unsigned int s) : seed(s) {
    initializeGradients();
    initializePermutation();
}

// Inicializa los gradientes unitarios
void PerlinNoise::initializeGradients() {
    gradients = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {sqrt(0.5), sqrt(0.5)}, {-sqrt(0.5), sqrt(0.5)},
        {sqrt(0.5), -sqrt(0.5)}, {-sqrt(0.5), -sqrt(0.5)}
    };
}

void PerlinNoise::setSeed(unsigned int newSeed) {
    seed = newSeed;
    initializePermutation();
}

// Inicializa la tabla de permutación
void PerlinNoise::initializePermutation() {
    permutation.resize(256);
    for (int i = 0; i < 256; ++i) permutation[i] = i;
    // Usar el generador de números aleatorios con la semilla
    std::mt19937 rng(seed);
    //std::shuffle(permutation.begin(), permutation.end(), std::mt19937{std::random_device{}()});
    std::shuffle(permutation.begin(), permutation.end(), rng);
    // Duplicar la tabla para evitar overflow
    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

// Función hash
int PerlinNoise::hash(int x, int y) const {
    return permutation[(permutation[x & 255] + y) & 255];
}

// Calcula el producto punto entre el gradiente y la distancia
double PerlinNoise::dotGridGradient(int ix, int iy, double x, double y) const {
    int gradientIndex = hash(ix, iy) % gradients.size();
    auto gradient = gradients[gradientIndex];
    double dx = x - ix;
    double dy = y - iy;
    return (dx * gradient.first + dy * gradient.second);
}

// Función de suavizado (fade)
double PerlinNoise::fade(double t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Interpolación lineal
double PerlinNoise::lerp(double a, double b, double t) const {
    return a + t * (b - a);
}

// Función para obtener el valor de ruido Perlin en (x, y)
double PerlinNoise::noise(double x, double y) const {
    int x0 = static_cast<int>(floor(x));
    int y0 = static_cast<int>(floor(y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    double sx = fade(x - x0);
    double sy = fade(y - y0);

    double n00 = dotGridGradient(x0, y0, x, y);
    double n10 = dotGridGradient(x1, y0, x, y);
    double n01 = dotGridGradient(x0, y1, x, y);
    double n11 = dotGridGradient(x1, y1, x, y);

    double ix0 = lerp(n00, n10, sx);
    double ix1 = lerp(n01, n11, sx);
    return lerp(ix0, ix1, sy);
}