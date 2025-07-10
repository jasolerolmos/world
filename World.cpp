#include <fstream>
#include <stdexcept>
#include "World.h"
#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>

// Constructor
World::World(int w, int h) : width(w), height(h) {}

// Generación del mundo usando Perlin Noise

void World::generateWorld(double scale, int octaves, double persistence, double lacunarity) {
    world.assign(height, std::vector<double>(width, 0.0));
    weather.assign(height, std::vector<double>(width, 0.0));

    std::cout << "Creating surface..." << std::endl;
    #pragma omp parallel for collapse(2) // Paraleliza los bucles anidados
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double amplitude = 1.0;
            double frequency = 1.0;
            double noiseHeight = 0.0;
            double noiseHeightWeather = 0.0;

            for (int o = 0; o < octaves; ++o) {
                double nx = x / scale * frequency;
                double ny = y / scale * frequency;
                noiseHeight += perlin.noise(nx, ny) * amplitude;

                amplitude *= persistence;
                frequency *= lacunarity;
            }
            world[y][x] = noiseHeight;
        }
    }

    std::cout << "Creating Weather..." << std::endl;
    #pragma omp parallel for collapse(2) // Paraleliza los bucles anidados
    double scaleW = 800;
    int octavesW = 1;
    double persistenceW = persistence;
    double lacunarityW = lacunarity;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double amplitude = 1.0;
            double frequency = 1.0;
            double noiseHeight = 0.0;
            double noiseHeightWeather = 0.0;

            for (int o = 0; o < octavesW; ++o) {
                double nx = x / scaleW * frequency;
                double ny = y / scaleW * frequency;
                noiseHeightWeather += perlin.noise(nx, ny) * amplitude;

                amplitude *= persistenceW;
                frequency *= lacunarityW;
            }
            weather[y][x] = noiseHeightWeather;
        }
    }
    normalizeWorld();
}

// Normaliza los valores del mundo entre 0 y 1
void World::normalizeWorld() {
    double min_val = std::numeric_limits<double>::max();
    double max_val = std::numeric_limits<double>::lowest();
    for (const auto& row : world) {
        for (double val : row) {
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
        }
    }
    double range = max_val - min_val;
    if (range < 1e-6) range = 1.0; // Evitar divisiones por cero

    for (auto& row : world) {
        for (double& val : row) {
            val = (val - min_val) / range;
        }
    }
    
    min_val = std::numeric_limits<double>::max();
    max_val = std::numeric_limits<double>::lowest();
    for (const auto& row : weather) {
        for (double val : row) {
            min_val = std::min(min_val, val);
            max_val = std::max(max_val, val);
        }
    }
    range = max_val - min_val;
    if (range < 1e-6) range = 1.0; // Evitar divisiones por cero

    for (auto& row : weather) {
        for (double& val : row) {
            val = (val - min_val) / range;
        }
    }
}

// Mapa de colores
cv::Vec3b World::colorMap(double value) const {
    if (value < 0.4) return cv::Vec3b(255, 0, 0);          // Agua (azul)
    if (value < 0.45) return cv::Vec3b(19, 69, 139);       // Tierras bajas (marrón)
    if (value < 0.65) return cv::Vec3b(0, 128, 0);         // Tierras medias (verde)
    if (value < 0.85) return cv::Vec3b(85, 85, 85);        // Montañas bajas (gris)
    return cv::Vec3b(255, 255, 255);                      // Cumbres (blanco)
}

// Mapa de colores
cv::Vec3b World::weatherMap(double value1, double value2) const {
    switch (getCodigoBioma(value1, value2)) {
        case 1:
            return cv::Vec3b(255, 0, 0);
        case 2:
            return cv::Vec3b(255, 104, 0);
        case 11:
            return cv::Vec3b(80, 132, 108);
        case 12:
            return cv::Vec3b(48, 174, 106);
        case 13:
            return cv::Vec3b(141, 217, 240);
        case 21:
            return cv::Vec3b(51, 125, 91);
        case 22:
            return cv::Vec3b(62, 156, 110);
        case 23:
            return cv::Vec3b(47, 174, 106);
        case 24:
            return cv::Vec3b(134, 188, 202);
        case 31:
            return cv::Vec3b(81, 132, 108);
        case 32:
            return cv::Vec3b(106, 156, 147);
        case 33:
            return cv::Vec3b(153, 173, 178);
        case 41:
            return cv::Vec3b(175, 218, 184);
        case 42:
            return cv::Vec3b(111, 111, 122);
        case 43:
            return cv::Vec3b(255, 255, 255);
    }
    return cv::Vec3b(0, 0, 0);

    /*
    if (value1 < 0.2) {
        return cv::Vec3b(255, 0, 0);
    }
    if (value1 < 0.4) {
        return cv::Vec3b(255, 104, 0);
    }
    if (value1 < 0.45) {
        //return cv::Vec3b(19, 69, 139);  // Tierras bajas (marrón)
        if (value2 < 0.6) {
            return cv::Vec3b(80, 132, 108);
        } else if (value2 < 0.75) {
            return cv::Vec3b(48, 174, 106);
        } else {
            return cv::Vec3b(141, 217, 240);
        } 
    }
    if (value1 < 0.70) { 
        //return cv::Vec3b(0, 128, 0);  // Tierras medias (verde)
        if (value2 < 0.25) {
            return cv::Vec3b(51, 125, 91);
        } else if (value2 < 0.50) {
            return cv::Vec3b(62, 156, 110);
        } else if (value2 < 0.75) {
            return cv::Vec3b(47, 174, 106);
        } else {
            return cv::Vec3b(134, 188, 202);
        } 
    }
    if (value1 < 0.78) { // Montañas bajas (gris)
        //return cv::Vec3b(85, 85, 85); 
        if (value2 < 0.6) {
            return cv::Vec3b(81, 132, 108);
        } else if (value2 < 0.75) {
            return cv::Vec3b(106, 156, 147);
        } else {
            return cv::Vec3b(153, 173, 178);
        } 
    }

    if (value1 < 0.9) { // Montañas bajas (gris)
        if (value2 < 0.5) {
            return cv::Vec3b(175, 218, 184);
        } else return cv::Vec3b(111, 111, 122);
    }
    
    return cv::Vec3b(255, 255, 255);                      // Cumbres (blanco)
    */
}

// Mapa de colores
std::string World::getBiomaName(double value1, double value2) const {
    switch (getCodigoBioma(value1, value2)) {
        case 1:
            return "Agua";
        case 2:
            return "Aguas Profundas";
        case 11:
            return "Bosque Tropical";
        case 12:
            return "Pradera";
        case 13:
            return "Desierto Tropical";
        case 21:
            return "Bosque Templado Humedo";
        case 22:
            return "Bosque Templado Seco";
        case 23:
            return "Pradera";
        case 24:
            return "Desierto Templado";
        case 31:
            return "Taiga";
        case 32:
            return "Matorrales";
        case 33:
            return "Desierto Frío";
        case 41:
            return "Nieve";
        case 42:
            return "Tundra";
        case 43:
            return "Yermo";
        case 44:
            return "Abismo";
    }
    return "";
}

// Mapa de colores
int World::getCodigoBioma(double value1, double value2) const {
    if (value1 < 0.2) {
        return 1;
    }
    if (value1 < 0.4) {
        return 2;
    }
    if (value1 < 0.45) {
        if (value2 < 0.6) {
            return 11;
        } else if (value2 < 0.75) {
            return 12;
        } else {
            return 13;
        } 
    }
    if (value1 < 0.70) { 
        if (value2 < 0.25) {
            return 21;
        } else if (value2 < 0.50) {
            return 22;
        } else if (value2 < 0.75) {
            return 23;
        } else {
            return 24;
        } 
    }
    if (value1 < 0.78) { 
        if (value2 < 0.6) {
            return 31;
        } else if (value2 < 0.75) {
            return 32;
        } else {
            return 33;
        } 
    }

    if (value1 < 0.9) {
        if (value2 < 0.5) {
            return 41;
        } else return 42;
    }
    
    return 43;
}

std::string World::terrainType(double value) const {
    if (value < 0.4) return "Agua";
    if (value < 0.45) return "Tierras bajas";
    if (value < 0.70) return "Tierras medias";
    if (value < 0.78) return "Montañas bajas";
    return "Cumbres";
}

// Crear el mundo
void World::createWorld(double scale, int octaves, double persistence, double lacunarity) {
    if (scale <= 0 || octaves <= 0 || persistence < 0 || lacunarity <= 0) {
        throw std::invalid_argument("Parámetros inválidos para generar el mundo");
    }

    std::cout << "Creating World..." << std::endl;
    generateWorld(scale, octaves, persistence, lacunarity);
}

void World::setSeed(unsigned int seed) {
    perlin.setSeed(seed);
}

std::string World::getBiomaNameAt(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;
    return getBiomaName(world[y][x], weather[y][x]);
}

std::string World::getTerrainName(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return "Fuera de límites";
    return terrainType(world[y][x]);
}

int World::getTerrainType(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return -1;
    double value = world[y][x];
    if (value < 0.4) return 0;
    if (value < 0.45) return 10;
    if (value < 0.70) return 20;
    if (value < 0.78) return 30;
    return 40;
}

double World::getRawValue(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) return -1;
    return world[y][x];
}

bool World::isPossiblePass(int x, int y) const {
    return true;
}

void World::showImage(const std::string& windowName) const {
    cv::Mat image(height, width, CV_8UC3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image.at<cv::Vec3b>(y, x) = colorMap(world[y][x]);
        }
    }
    cv::imshow(windowName, image);
    cv::waitKey(0); // Espera a que el usuario cierre la ventana
}

// Guardar la imagen en un archivo
void World::saveImage(const std::string& filename) const {
    cv::Mat image(height, width, CV_8UC3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image.at<cv::Vec3b>(y, x) = weatherMap(world[y][x], weather[x][y]);
            //image.at<cv::Vec3b>(y, x) = colorMap(weather[y][x]);
        }
    }
    cv::imwrite(filename, image);
    std::cout << "Imagen guardada como: " << filename << std::endl;
}

/*
std::vector<std::vector<int>> World::getBiomaGrid(std::pair<int, int> coordenadas, std::pair<int, int> winViewport) const {
    // Dimensiones de la cuadrícula 11x11
    int centerX = coordenadas.first;
    int centerY = coordenadas.second;
    const int gridSizeX = winViewport.first;
    const int gridSizeY = winViewport.second;
    
    if ( centerX < 0 ) centerX = 0;
    if ( centerY < 0 ) centerY = 0;
    if ( centerX >= width ) centerX = width - 1;
    if ( centerY >= height ) centerX = height - 1;

    const int halfGridX = gridSizeX / 2;
    const int halfGridY = gridSizeY / 2;

    if (centerX < halfGridX) centerX = halfGridX;
    if (centerY < halfGridY) centerY = halfGridY;

    std::cout << "Centro: " << centerX << "," << centerY << std::endl;
    std::cout << "Desde: " << (centerX-halfGridX) << "," << (centerY-halfGridY) << std::endl;
    std::cout << "Hasta: " << (centerX+halfGridX) << "," << (centerY+halfGridY) << std::endl;
    std::cout << "gridSizeX: " << gridSizeX << " gridSizeY " << gridSizeY << std::endl;
    
    // Resultado
    std::vector<std::vector<int>> biomaGrid(gridSizeY, std::vector<int>(gridSizeX, 0));

    std::cout << "Rellenando";

    for (int dy = -halfGridY; dy <= halfGridY; ++dy) {
        for (int dx = -halfGridX; dx <= halfGridX; ++dx) {
            int x = centerX + dx;
            int y = centerY + dy;
            //std::cout << "=> " << x << "," << y << std::endl;
            if (x >= 0 && x < width && y >= 0 && y < height) {
                // Coordenadas válidas
                biomaGrid[dy + halfGridY][dx + halfGridX] = getCodigoBioma(world[y][x], weather[y][x]);
            } else {
                // Fuera de límites
                biomaGrid[dy + halfGridY][dx + halfGridX] = -1; // -1 indica fuera de límites
            }
        }
        std::cout << ".";
    }
    std::cout << std::endl;

    std::cout << "Terminado" << std::endl;

    return biomaGrid;
}

*/

std::vector<std::vector<int>> World::getBiomaGrid(std::pair<int, int> coordenadas, std::pair<int, int> winViewport) const {
    int centerX = coordenadas.first;
    int centerY = coordenadas.second;
    const int gridSizeX = winViewport.first;
    const int gridSizeY = winViewport.second;

    // Asegurarse de que las coordenadas están dentro de los límites
    centerX = std::max(0, std::min(centerX, width - 1));
    centerY = std::max(0, std::min(centerY, height - 1));

    const int halfGridX = gridSizeX / 2;
    const int halfGridY = gridSizeY / 2;

    centerX = std::max(halfGridX, std::min(centerX, width - halfGridX - 1));
    centerY = std::max(halfGridY, std::min(centerY, height - halfGridY - 1));

    std::vector<std::vector<int>> biomaGrid(gridSizeY, std::vector<int>(gridSizeX, 0));

    for (int dy = -halfGridY; dy <= halfGridY; ++dy) {
        for (int dx = -halfGridX; dx <= halfGridX; ++dx) {
            int x = centerX + dx;
            int y = centerY + dy;

            if (dy + halfGridY >= 0 && dy + halfGridY < gridSizeY &&
                dx + halfGridX >= 0 && dx + halfGridX < gridSizeX) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    biomaGrid[dy + halfGridY][dx + halfGridX] = getCodigoBioma(world[y][x], weather[y][x]);
                } else {
                    biomaGrid[dy + halfGridY][dx + halfGridX] = -1; // Fuera de límites
                }
            }
        }
    }

    return biomaGrid;
}

void World::guardarEnArchivo(const std::string& nombreArchivo) const {
    std::ofstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo) {
        throw std::runtime_error("No se pudo abrir el archivo para escritura.");
    }

    // Guardar dimensiones del mundo
    archivo.write(reinterpret_cast<const char*>(&width), sizeof(width));
    archivo.write(reinterpret_cast<const char*>(&height), sizeof(height));

    // Guardar la matriz 'world'
    for (const auto& fila : world) {
        archivo.write(reinterpret_cast<const char*>(fila.data()), fila.size() * sizeof(double));
    }

    // Guardar la matriz 'weather'
    for (const auto& fila : weather) {
        archivo.write(reinterpret_cast<const char*>(fila.data()), fila.size() * sizeof(double));
    }

    archivo.close();
}

void World::cargarDesdeArchivo(const std::string& nombreArchivo) {
    std::ifstream archivo(nombreArchivo, std::ios::binary);
    if (!archivo) {
        throw std::runtime_error("No se pudo abrir el archivo para lectura.");
    }

    // Leer dimensiones del mundo
    archivo.read(reinterpret_cast<char*>(&width), sizeof(width));
    archivo.read(reinterpret_cast<char*>(&height), sizeof(height));

    // Redimensionar las matrices
    world.resize(height, std::vector<double>(width));
    weather.resize(height, std::vector<double>(width));

    // Leer la matriz 'world'
    for (auto& fila : world) {
        archivo.read(reinterpret_cast<char*>(fila.data()), fila.size() * sizeof(double));
    }

    // Leer la matriz 'weather'
    for (auto& fila : weather) {
        archivo.read(reinterpret_cast<char*>(fila.data()), fila.size() * sizeof(double));
    }

    archivo.close();
}