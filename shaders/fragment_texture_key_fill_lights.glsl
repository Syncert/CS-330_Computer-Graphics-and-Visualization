#version 440 core

in vec2 vertexTextureCoordinate;
in vec3 Normal;
in vec3 FragPos;

out vec4 fragmentColor;

uniform sampler2D uTexture;
uniform vec3 viewPos; // Camera's position

// Structs for light properties
struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Light keyLight;
uniform Light fillLight;

void main() {
    // Ambient component
    float ambientStrength = 0.3;
    vec3 ambientKey = ambientStrength * keyLight.color * keyLight.intensity;
    vec3 ambientFill = ambientStrength * fillLight.color * fillLight.intensity;
    vec3 ambient = ambientKey + ambientFill;

    // Diffuse component for key light
    vec3 norm = normalize(Normal);
    vec3 lightDirKey = normalize(keyLight.position - FragPos);
    float diffKey = max(dot(norm, lightDirKey), 0.0);
    vec3 diffuseKey = diffKey * keyLight.color * keyLight.intensity;

    // Diffuse component for fill light
    vec3 lightDirFill = normalize(fillLight.position - FragPos);
    float diffFill = max(dot(norm, lightDirFill), 0.0);
    vec3 diffuseFill = diffFill * fillLight.color * fillLight.intensity;

    // Specular component for key light
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDirKey = reflect(-lightDirKey, norm);
    float specKey = pow(max(dot(viewDir, reflectDirKey), 0.0), 32);
    vec3 specularKey = specularStrength * specKey * keyLight.color * keyLight.intensity;

    // Specular component for fill light
    vec3 reflectDirFill = reflect(-lightDirFill, norm);
    float specFill = pow(max(dot(viewDir, reflectDirFill), 0.0), 32);
    vec3 specularFill = specularStrength * specFill * fillLight.color * fillLight.intensity;

    // Combine lighting components from both lights
    vec3 result = (ambient + diffuseKey + specularKey + diffuseFill + specularFill) * texture(uTexture, vertexTextureCoordinate).rgb;
    fragmentColor = vec4(result, 1.0);
}