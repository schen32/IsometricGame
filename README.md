# Isometric Terrain Generation
Watch the Demo: https://youtu.be/0UuRBZ-clkg

## Terrain Generation using Perlin Noise
![image](https://github.com/user-attachments/assets/0f442323-53eb-43f2-b656-acf5cf4a9a98)

Perlin Noise is used to generate a height map for the terrain. Different types of tiles are placed depending on the height at a specific location to represent different materials in the real world. For example, every tile below sea level is a water tile, slightly above that are sand tile, then grass tiles for the majority of the terrain, and finally snow tiles at the top of mountains.

## Isometric Perspective Projection
![image](https://github.com/user-attachments/assets/d05a0aca-60cf-4487-bdf5-3dcd8394d0b3)

The world is viewed from an isometric perspective, where the 3-dimensional position of tiles is mapped to the screen without any perspective distortion (objects getting bigger/smaller as they move closer/farther away from the viewer). Isometric projection is an effective way to simulate 3d worlds without having to work in true 3d perspective.
