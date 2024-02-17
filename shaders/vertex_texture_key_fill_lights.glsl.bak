#version 440 core

layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;    // Color data from Vertex Attrib Pointer 1
layout(location = 2) in vec2 texCoords; // Texture coordinate data from Vertex Attrib Pointer 2

out vec4 vertexColor; // variable to transfer color data to the fragment shader
out vec2 vertexTextureCoordinate; // variable to transfer texture coordinate data to the fragment shader

// Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
    vertexTextureCoordinate = texCoords; // pass texture coordinates to fragment shader
}