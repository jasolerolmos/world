#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <vector>
#include <random>
#include <cmath>

class PerlinNoise {
private:
    std::vector<int> permutation;
    std::vector<std::pair<double, double>> gradients;

    // Gradientes unitarios
    void initializeGradients();

    // Tabla de permutación aleatoria
    void initializePermutation();

    int hash(int x, int y) const;

    double dotGridGradient(int ix, int iy, double x, double y) const;

    double fade(double t) const;

    double lerp(double a, double b, double t) const;

    unsigned int seed; // Nueva variable para almacenar la semilla

public:
    PerlinNoise();

    PerlinNoise(unsigned int seed);     // Constructor con semilla


    double noise(double x, double y) const;

    void setSeed(unsigned int newSeed);
};

#endif // PERLINNOISE_H