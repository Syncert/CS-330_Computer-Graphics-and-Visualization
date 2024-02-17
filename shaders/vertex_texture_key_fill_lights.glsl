#version 440 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal; // Normal vector for lighting
layout(location = 2) in vec2 textureCoordinate;

out vec2 vertexTextureCoordinate;
out vec3 FragPos; // Pass position to fragment shader
out vec3 Normal; // Pass normal to fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(position, 1.0f)); // Calculate world position
    Normal = mat3(transpose(inverse(model))) * normal; // Transform normal vector
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vertexTextureCoordinate = textureCoordinate;
}