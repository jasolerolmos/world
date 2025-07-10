#ifndef WORLD_H
#define WORLD_H

#include "PerlinNoise.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

class World {
private:
    int width, height;
    std::vector<std::vector<double>> world;
    std::vector<std::vector<double>> weather;
    PerlinNoise perlin;
    PerlinNoise perlinWeather; ///

    void generateWorld(double scale, int octaves, double persistence, double lacunarity);
    void normalizeWorld();
    cv::Vec3b colorMap(double value) const;

    cv::Vec3b weatherMap(double value1, double value2) const;

    std::string terrainType(double value) const;

    int getCodigoBioma(double value1, double value2) const;

    std::string getBiomaName(double value1, double value2) const;

public:
    World(int w, int h);

    void createWorld(double scale, int octaves, double persistence, double lacunarity);

    void saveImage(const std::string& filename) const;

    void showImage(const std::string& windowName) const;

    void setSeed(unsigned int seed);

    double getRawValue(int x, int y) const;

    std::string getTerrainName(int x, int y) const;

    int getBioma(double value1, double value2) const;

    std::string  getBiomaNameAt(int x, int y) const;

    std::vector<std::vector<int>> getBiomaGrid(std::pair<int, int> coordenadas, std::pair<int, int> winViewport) const;

    int getTerrainType(int x, int y) const;

    bool isPossiblePass(int x, int y) const;

    void cargarDesdeArchivo(const std::string& nombreArchivo);

    void guardarEnArchivo(const std::string& nombreArchivo) const;
};

#endif // WORLD_H