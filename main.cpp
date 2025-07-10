#include "World.h"

int main() {
    int width = 2048, height = 2048; 
    double scale = 200.0;
    int octaves = 10;
    double persistence = 0.5, lacunarity = 2.0;

    World world(width, height);
    world.createWorld(scale, octaves, persistence, lacunarity);
    world.saveImage("world_image.png");

    return 0;
}