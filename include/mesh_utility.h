#ifndef MESH_UTILITY_H
#define MESH_UTILITY_H

#include "mesh.h"

// Functions to create specific mesh shapes
Mesh CreateCircleShakerCapMesh(const std::string& directory, const std::string& fileName, bool gamma);
Mesh CreatePrismShakerBodyMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor);
Mesh CreatePlaneMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor);
Mesh CreateCylinderMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor);
Mesh CreateCubeMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor);
Mesh CreateCubePepperMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor);
Mesh CreateSphereMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor);

//debug mesh
Mesh CreateDebugCubeMesh();

//Function to load texture 
// Function to load a texture from a file
unsigned int LoadTextureFromFile(const char* path, const std::string& directory, bool gamma = false);

#endif