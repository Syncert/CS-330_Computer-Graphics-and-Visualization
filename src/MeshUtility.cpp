// MeshUtility.cpp
#include <mesh_utility.h>
#include <mesh.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vector>
#include <cmath> // For std::acos

// Define PI
constexpr double PI = 3.14159265358979323846;


//Load a texture from file
unsigned int LoadTextureFromFile(const char* path, const string& directory, bool gamma) {
    string filename = string(path);
    filename = directory + '/' + filename;


    unsigned int textureID;
    glGenTextures(1, &textureID);

    // Load and generate the texture
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format{};
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Shaker Cap
Mesh CreateCircleShakerCapMesh(const std::string& directory, const std::string& fileName, bool gamma) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    const unsigned int numSegments = 32;
    float radius = 0.5f;
    float capHeight = 0.2f;
    float angleIncrement = 360.0f / numSegments;

    // Generate the top circle vertices and the center top vertex
    for (unsigned int i = 0; i <= numSegments; ++i) {
        float angle = i < numSegments ? glm::radians(angleIncrement * i) : 0;
        float x = i < numSegments ? radius * cos(angle) : 0.0f;
        float z = i < numSegments ? radius * sin(angle) : 0.0f;

        Vertex topVertex{};
        topVertex.Position = glm::vec3(x, capHeight, z);
        topVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        topVertex.TexCoords = glm::vec2(x * 0.5f + 0.5f, z * 0.5f + 0.5f); // Mapping to texture coordinates
        vertices.push_back(topVertex);
    }

    // Generate the bottom circle vertices and the center bottom vertex
    for (unsigned int i = 0; i <= numSegments; ++i) {
        float angle = i < numSegments ? glm::radians(angleIncrement * i) : 0;
        float x = i < numSegments ? radius * cos(angle) : 0.0f;
        float z = i < numSegments ? radius * sin(angle) : 0.0f;

        Vertex bottomVertex{};
        bottomVertex.Position = glm::vec3(x, 0.0f, z);
        bottomVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        bottomVertex.TexCoords = glm::vec2(x * 0.5f + 0.5f, z * 0.5f + 0.5f); // Mapping to texture coordinates
        vertices.push_back(bottomVertex);
    }

    // Indices for the top circle (triangle fan)
    for (unsigned int i = 0; i < numSegments; ++i) {
        indices.push_back(numSegments); // Center top vertex index
        indices.push_back(i);
        indices.push_back((i + 1) % numSegments);
    }

    // Indices for the bottom circle (triangle fan)
    unsigned int bottomStartIndex = numSegments + 1; // Offset by the top circle vertices and one center top vertex
    for (unsigned int i = 0; i < numSegments; ++i) {
        indices.push_back(bottomStartIndex + numSegments); // Center bottom vertex index
        indices.push_back(bottomStartIndex + (i + 1) % numSegments);
        indices.push_back(bottomStartIndex + i);
    }

    // Generate the indices for the cylindrical part
    for (unsigned int i = 0; i < numSegments; ++i) {
        unsigned int nextIndex = (i + 1) % numSegments;

        // First triangle of the side quad
        indices.push_back(i);
        indices.push_back(nextIndex);
        indices.push_back(bottomStartIndex + nextIndex);

        // Second triangle of the side quad
        indices.push_back(i);
        indices.push_back(bottomStartIndex + nextIndex);
        indices.push_back(bottomStartIndex + i);
    }

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma);
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create and return the mesh
    return Mesh(vertices, indices, textures);
}

//Shaker Body
Mesh CreatePrismShakerBodyMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures; // If you have textures

    // Define the octagon radius and the prism height
    float radius = 0.5f; // The radius of the octagon
    float prismHeight = 1.0f; // The height of the prism
    float angleIncrement = 360.0f / 8.0f; // Octagon, so 360 degrees divided by 8 vertices

    //Vertices//

    // Calculate the texture coordinates with repeating
    float textureUIncrement = textureRepeatFactor / 8.0f; // Repeat the texture across the prism's body
    float textureVHeight = textureRepeatFactor; // Repeat the texture vertically according to the height

    for (int i = 0; i < 8; ++i) {
        float angle = glm::radians(angleIncrement * i);
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float u = textureUIncrement * i; // Adjusted for texture repeat

        // Side vertices (between bottom and top base vertices)
        for (float y = 0; y <= prismHeight; y += prismHeight) {
            Vertex sideVertex{};
            sideVertex.Position = glm::vec3(x, y, z);
            sideVertex.Normal = glm::normalize(glm::vec3(x, 0.0f, z)); // Normal should point outwards from the center
            sideVertex.TexCoords = glm::vec2(u, y / prismHeight * textureVHeight); // Adjusted for texture repeat
            vertices.push_back(sideVertex);
        }
    }

    // Add the central bottom vertex
    Vertex bottomCenterVertex{};
    bottomCenterVertex.Position = glm::vec3(0.0f, 0.0f, 0.0f);
    bottomCenterVertex.Normal = glm::vec3(0.0f, -1.0f, 0.0f);
    bottomCenterVertex.TexCoords = glm::vec2(0.5f, 0.5f); // Center of texture
    vertices.push_back(bottomCenterVertex);

    // Add the central top vertex
    Vertex topCenterVertex{};
    topCenterVertex.Position = glm::vec3(0.0f, prismHeight, 0.0f);
    topCenterVertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    topCenterVertex.TexCoords = glm::vec2(0.5f, 0.5f); // Center of texture
    vertices.push_back(topCenterVertex);

    //Indices//

    // Bottom base and top base indices
    for (int i = 0; i < 8; ++i)
    {
        // Side faces bottom
        indices.push_back(i * 2);
        indices.push_back(((i + 1) % 8) * 2);
        indices.push_back(((i + 1) % 8) * 2 + 1);
        //side faces top
        indices.push_back(i * 2);
        indices.push_back(((i + 1) % 8) * 2 + 1);
        indices.push_back(i * 2 + 1);
    }


    // Correct the index for the central bottom and top vertices
    GLuint centralBottomVertexIndex = 16; // Since there are 16 side vertices before adding the central vertices
    GLuint centralTopVertexIndex = 17; // The top central vertex is added right after the bottom central vertex

    // Indices for the bottom base triangles
    for (int i = 0; i < 8; ++i)
    {
        indices.push_back(i * 2);
        indices.push_back(((i + 1) % 8) * 2);
        indices.push_back(centralBottomVertexIndex);
    }

    // Indices for the top base triangles
    for (int i = 0; i < 8; ++i)
    {
        indices.push_back(centralTopVertexIndex);
        indices.push_back(1 + i * 2); // Top vertices are at odd indices
        indices.push_back(1 + ((i + 1) % 8) * 2);
    }

    //Textures//

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    return Mesh(vertices, indices, textures);
}

//Plane Mesh for Table
Mesh CreatePlaneMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Define vertices for the plane with adjusted texture coordinates for repetition
    vertices = {
        // Positions          // Normals        // Texture Coords (adjusted for repetition)
        {{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, textureRepeatFactor}}, // Top-left
        {{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {textureRepeatFactor, textureRepeatFactor}}, // Top-right
        {{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {textureRepeatFactor, 0.0f}}, // Bottom-right
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
    };

    // Define indices for the plane
    indices = {
        0, 1, 3, // First triangle
        1, 2, 3  // Second triangle
    };

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create Mesh
    return Mesh(vertices, indices, textures);
}

// Cylinder Mesh
Mesh CreateCylinderMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    float radius = 0.5f;
    float halfHeight = 0.5f;
    int sectorCount = 36; // number of slices
    float sectorStep = 2 * PI / sectorCount;
    float sectorAngle;  // radian

    // Vertices
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0, z)); // Normalize the side normals

        // Apply texture repeat factor for all but the last set of vertices
        float u = (i < sectorCount) ? static_cast<float>(i) / sectorCount * textureRepeatFactor : 0.0f;
        float vTop = 1.0f; // Texture coordinate V for the top vertex
        float vBottom = 0.0f; // Texture coordinate V for the bottom vertex

        // For the last vertex, explicitly set texture coordinates to sample from (0.0f, 0.0f)
        if (i == sectorCount) {
            vertices.push_back(Vertex{ {x, halfHeight, z}, normal, {0.0f, 0.0f} });
            vertices.push_back(Vertex{ {x, -halfHeight, z}, normal, {0.0f, 0.0f} });
        }
        else {
            vertices.push_back(Vertex{ {x, halfHeight, z}, normal, {u, vTop} });
            vertices.push_back(Vertex{ {x, -halfHeight, z}, normal, {u, vBottom} });
        }
    }

    // Indices
    int k1, k2;
    for (int i = 0; i < sectorCount; ++i) {
        k1 = i * 2;     // indices for sectors + 1
        k2 = k1 + 1;    // indices for sectors + 2

        // two triangles per sector excluding first and last points
        if (i < (sectorCount - 1)) {
            indices.push_back(k1);
            indices.push_back(k1 + 2);
            indices.push_back(k2);

            indices.push_back(k2);
            indices.push_back(k1 + 2);
            indices.push_back(k2 + 2);
        }
        // last triangle
        else {
            indices.push_back(k1);
            indices.push_back(0);
            indices.push_back(k2);

            indices.push_back(k2);
            indices.push_back(0);
            indices.push_back(1);
        }
    }

    // Use a single texture coordinate for all cap vertices to sample from the same texture point
    const glm::vec2 centerTexCoord(0.5f, 0.5f); // Texture coordinate for center (single point sampling)

    // Top Cap
    int baseCenterIndex = vertices.size();
    vertices.push_back(Vertex{ {0, halfHeight, 0}, {0, 1, 0}, centerTexCoord }); // Center vertex

    // Top cap perimeter vertices (all share the same texture coordinate for uniform color)
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        vertices.push_back(Vertex{ {x, halfHeight, z}, {0, 1, 0}, centerTexCoord });
    }

    // Bottom Cap
    int bottomCenterIndex = vertices.size();
    vertices.push_back(Vertex{ {0, -halfHeight, 0}, {0, -1, 0}, centerTexCoord }); // Center vertex

    // Bottom cap perimeter vertices (all share the same texture coordinate for uniform color)
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        vertices.push_back(Vertex{ {x, -halfHeight, z}, {0, -1, 0}, centerTexCoord });
    }

    // Top Cap
    baseCenterIndex = vertices.size();
    vertices.push_back(Vertex{ {0, halfHeight, 0}, {0, 1, 0}, centerTexCoord }); // Center vertex

    // Top cap perimeter vertices (all share the same texture coordinate for uniform color)
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        vertices.push_back(Vertex{ {x, halfHeight, z}, {0, 1, 0}, centerTexCoord });
    }

    // Bottom Cap
    bottomCenterIndex = vertices.size();
    vertices.push_back(Vertex{ {0, -halfHeight, 0}, {0, -1, 0}, centerTexCoord }); // Center vertex

    // Bottom cap perimeter vertices (all share the same texture coordinate for uniform color)
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        vertices.push_back(Vertex{ {x, -halfHeight, z}, {0, -1, 0}, centerTexCoord });
    }

    // Top Cap Indices
    for (int i = 0; i < sectorCount; ++i) {
        int k1 = baseCenterIndex + 1 + i;
        int k2 = k1 + 1;
        indices.push_back(baseCenterIndex);
        indices.push_back(k1);
        indices.push_back(k2);
    }
    indices[indices.size() - 1] = baseCenterIndex + 1; // Last triangle wraps back to first perimeter vertex

    // Bottom Cap Indices
    for (int i = 0; i < sectorCount; ++i) {
        int k1 = bottomCenterIndex + 1 + i;
        int k2 = k1 + 1;
        indices.push_back(bottomCenterIndex);
        indices.push_back(k2);
        indices.push_back(k1);
    }
    indices[indices.size() - 2] = bottomCenterIndex + 1; // Last triangle wraps back to first perimeter vertex

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create and return the Mesh object
    return Mesh(vertices, indices, textures);
}

//BACKUP CYLINDER MESH BEFORE REMOVING SOME TEXTURE MAPPING
/*
// Cylinder Mesh
Mesh CreateCylinderMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    float radius = 0.5f;
    float halfHeight = 0.5f;
    int sectorCount = 36; // number of slices
    float sectorStep = 2 * PI / sectorCount;
    float sectorAngle;  // radian

    // Vertices
    for (int i = 0; i <= sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        glm::vec3 normal = glm::normalize(glm::vec3(x, 0.0, z)); // Normalize the side normals
        vertices.push_back(Vertex{ {x, halfHeight, z}, normal, {static_cast<float>(i) / sectorCount * textureRepeatFactor, 1.0f} });
        vertices.push_back(Vertex{ {x, -halfHeight, z}, normal, {static_cast<float>(i) / sectorCount * textureRepeatFactor, 0.0f} });
    }

    // Indices
    int k1, k2;
    for (int i = 0; i < sectorCount; ++i) {
        k1 = i * 2;     // indices for sectors + 1
        k2 = k1 + 1;    // indices for sectors + 2

        // two triangles per sector excluding first and last points
        if (i < (sectorCount - 1)) {
            indices.push_back(k1);
            indices.push_back(k1 + 2);
            indices.push_back(k2);

            indices.push_back(k2);
            indices.push_back(k1 + 2);
            indices.push_back(k2 + 2);
        }
        // last triangle
        else {
            indices.push_back(k1);
            indices.push_back(0);
            indices.push_back(k2);

            indices.push_back(k2);
            indices.push_back(0);
            indices.push_back(1);
        }
    }

    // Top Cap
    int baseCenterIndex = vertices.size();
    float y = halfHeight;
    vertices.push_back(Vertex{ {0, y, 0}, {0, 1, 0}, {0.5f, 0.5f} }); // Top center vertex

    // Top circle perimeter vertices
    for (int i = 0; i < sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        vertices.push_back(Vertex{ {x, y, z}, {0, 1, 0}, {cos(sectorAngle) * 0.5f + 0.5f, sin(sectorAngle) * 0.5f + 0.5f} });
    }

    // Bottom Cap
    int bottomCenterIndex = vertices.size();
    y = -halfHeight;
    vertices.push_back(Vertex{ {0, y, 0}, {0, -1, 0}, {0.5f, 0.5f} }); // Bottom center vertex

    // Bottom circle perimeter vertices
    for (int i = 0; i < sectorCount; ++i) {
        sectorAngle = i * sectorStep;
        float x = radius * cos(sectorAngle);
        float z = radius * sin(sectorAngle);
        vertices.push_back(Vertex{ {x, y, z}, {0, -1, 0}, {cos(sectorAngle) * 0.5f + 0.5f, sin(sectorAngle) * 0.5f + 0.5f} });
    }

    // Top Cap Indices
    for (int i = 0; i < sectorCount; ++i) {
        int k1 = baseCenterIndex + 1 + i;
        int k2 = k1 + 1;
        indices.push_back(baseCenterIndex);
        indices.push_back(k1);
        indices.push_back(k2);
    }
    indices[indices.size() - 1] = baseCenterIndex + 1; // Last triangle wraps back to first perimeter vertex

    // Bottom Cap Indices
    for (int i = 0; i < sectorCount; ++i) {
        int k1 = bottomCenterIndex + 1 + i;
        int k2 = k1 + 1;
        indices.push_back(bottomCenterIndex);
        indices.push_back(k2);
        indices.push_back(k1);
    }
    indices[indices.size() - 2] = bottomCenterIndex + 1; // Last triangle wraps back to first perimeter vertex

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create and return the Mesh object
    return Mesh(vertices, indices, textures);
}
*/

//Cube Mesh
Mesh CreateCubeMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Define vertices for the cube with adjusted texture coordinates for repetition
    Vertex cubeVertices[] = {
        // Positions          // Normals          // Texture Coords
        // Bottom face 0 1 2 3
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, textureRepeatFactor}},

        // Top face 4 5 6 7
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, textureRepeatFactor}},

        // Front face 8 9 10 11
        {{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, textureRepeatFactor}},

        // Back face 12 13 14 15
        {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, textureRepeatFactor}},

        // Left face 16 17 18 19
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},

        // Right face 20 21 22 23
        {{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},
        };

    //populate vertices vector
    int numVertices = sizeof(cubeVertices) / sizeof(Vertex); // Calculate the number of vertices
    for (int i = 0; i < numVertices; ++i) {
        vertices.push_back(cubeVertices[i]);
    }

    // Define indices for the cube
    unsigned int cubeIndices[] = {
        // Front face
        8, 9, 10,
        10, 11, 8,
        // Back face
        14, 13, 12,
        12, 15, 14,
        // Left face
        16, 17, 18,
        18, 19, 16,
        // Right face
        20, 23, 22,
        22, 21, 20,
        // Top face
        4, 5, 6,
        6, 7, 4,
        // Bottom face
        0, 3, 2,
        2, 1, 0
    };

    // Populate vertices vector
    int numIndices = sizeof(cubeIndices) / sizeof(unsigned int); // Calculate the number of vertices
    for (int i = 0; i < numIndices; ++i) {
        indices.push_back(cubeIndices[i]);
    }

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create and return the Mesh object
    return Mesh(vertices, indices, textures);
}


// Cube Pepper Mesh
Mesh CreateCubePepperMesh(const std::string & directory, const std::string & fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Define vertices for the cube with adjusted texture coordinates for repetition
    Vertex cubeVertices[] = {
    // Positions          // Normals          // Texture Coords
       /*IF YOU WANT TEXTURES ON ALL FACES, WE DO NOT
    // Bottom face 0 1 2 3
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}}, 
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, textureRepeatFactor}},

    // Top face 4 5 6 7
    {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, textureRepeatFactor}},

    // Front face 8 9 10 11
    {{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {textureRepeatFactor, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, textureRepeatFactor}},

    // Back face 12 13 14 15
    {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {textureRepeatFactor, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, textureRepeatFactor}},

    // Left face 16 17 18 19
    {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},

    // Right face 20 21 22 23
    {{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
    {{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},
    };
    */

    // Bottom face 0 1 2 3
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},

    // Top face 4 5 6 7
    {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},

    // Front face 8 9 10 11
    {{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},

    // Back face 12 13 14 15
    {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},

    // Left face 16 17 18 19
    {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
    {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f,textureRepeatFactor}},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},

    // Right face 20 21 22 23
    {{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
    {{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
    {{0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},
        };
    //populate vertices vector
    int numVertices = sizeof(cubeVertices) / sizeof(Vertex); // Calculate the number of vertices
    for (int i = 0; i < numVertices; ++i) {
        vertices.push_back(cubeVertices[i]);
    }

    // Define indices for the cube
    unsigned int cubeIndices[] = {
        // Front face
        8, 9, 10,
        10, 11, 8,
        // Back face
        14, 13, 12,
        12, 15, 14,
        // Left face
        16, 17, 18,
        18, 19, 16,
        // Right face
        20, 23, 22,
        22, 21, 20,
        // Top face
        4, 5, 6,
        6, 7, 4,
        // Bottom face
        0, 3, 2,
        2, 1, 0
    };

    // Populate vertices vector
    int numIndices = sizeof(cubeIndices) / sizeof(unsigned int); // Calculate the number of vertices
    for (int i = 0; i < numIndices; ++i) {
        indices.push_back(cubeIndices[i]);
    }

// Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create and return the Mesh object
    return Mesh(vertices, indices, textures);
}

//Debug Cube Mesh
Mesh CreateDebugCubeMesh() {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    float textureRepeatFactor = 1.0f;

    // Define vertices for the cube with adjusted texture coordinates for repetition
    Vertex cubeVertices[] = {
        // Positions          // Normals          // Texture Coords

        // Bottom face 0 1 2 3
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f,  0.0f}, {0.0f, textureRepeatFactor}},

        // Top face 4 5 6 7
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  1.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f,  1.0f,  0.0f}, {0.0f, textureRepeatFactor}},

        // Front face 8 9 10 11
        {{-0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f,  0.0f,  1.0f}, {0.0f, textureRepeatFactor}},

        // Back face 12 13 14 15
        {{-0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {textureRepeatFactor, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f,  0.0f, -1.0f}, {0.0f, textureRepeatFactor}},

        // Left face 16 17 18 19
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},

        // Right face 20 21 22 23
        {{0.5f, -0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, 0.0f}},
        {{0.5f,  0.5f,  0.5f}, {1.0f,  0.0f,  0.0f}, {textureRepeatFactor, textureRepeatFactor}},
        {{0.5f,  0.5f, -0.5f}, {1.0f,  0.0f,  0.0f}, {0.0f, textureRepeatFactor}},
    };

    //populate vertices vector
    int numVertices = sizeof(cubeVertices) / sizeof(Vertex); // Calculate the number of vertices
    for (int i = 0; i < numVertices; ++i) {
        vertices.push_back(cubeVertices[i]);
    }

    // Define indices for the cube
    unsigned int cubeIndices[] = {
        // Front face
        8, 9, 10,
        10, 11, 8,
        // Back face
        14, 13, 12,
        12, 15, 14,
        // Left face
        16, 17, 18,
        18, 19, 16,
        // Right face
        20, 23, 22,
        22, 21, 20,
        // Top face
        4, 5, 6,
        6, 7, 4,
        // Bottom face
        0, 3, 2,
        2, 1, 0
    };

    // Populate vertices vector
    int numIndices = sizeof(cubeIndices) / sizeof(unsigned int); // Calculate the number of vertices
    for (int i = 0; i < numIndices; ++i) {
        indices.push_back(cubeIndices[i]);
    }

    // Create and return the Mesh object
    return Mesh(vertices, indices, textures);
}

//Sphere Mesh

Mesh CreateSphereMesh(const std::string& directory, const std::string& fileName, bool gamma, float textureRepeatFactor) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    float radius = 0.5f; // Sphere radius
    unsigned int stacks = 36; // Horizontal slices
    unsigned int slices = 18; // Vertical slices
    float pi = 3.14159265359f;

    // Vertices
    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = pi / 2 - i * pi / stacks; // Starting from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle); // r * cos(u)
        float z = radius * sinf(stackAngle); // r * sin(u)

        // Add vertices of each stack
        for (unsigned int j = 0; j <= slices; ++j) {
            float sliceAngle = j * 2 * pi / slices; // Slices angle theta

            float x = xy * cosf(sliceAngle); // r * cos(u) * cos(v)
            float y = xy * sinf(sliceAngle); // r * cos(u) * sin(v)

            float s = (float)j / slices;
            float t = (float)i / stacks;

            Vertex vertex;
            vertex.Position = glm::vec3(x, y, z);
            vertex.Normal = glm::normalize(vertex.Position);
            vertex.TexCoords = glm::vec2(s * textureRepeatFactor, t * textureRepeatFactor);
            vertices.push_back(vertex);
        }
    }

    // Indices
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (slices + 1); // Beginning of current stack
        unsigned int k2 = k1 + slices + 1;  // Beginning of next stack

        for (unsigned int j = 0; j < slices; ++j, ++k1, ++k2) {
            // Two triangles per slice, except for the first and last stacks
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    // Load texture(s)
    Texture texture;
    texture.id = LoadTextureFromFile(fileName.c_str(), directory, gamma); // Assuming gamma is false in this example
    texture.type = "texture_diffuse";
    texture.path = directory + '/' + fileName;
    textures.push_back(texture);

    // Create and return the Mesh object
    return Mesh(vertices, indices, textures);
}

