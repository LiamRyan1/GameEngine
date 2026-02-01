#version 330 core

out vec4 FragColor;

in vec3 FragPos;    // From vertex shader
in vec3 Normal;     // From vertex shader
in vec2 TexCoord;   // Texture coordinates

uniform vec3 objectColor;
uniform vec3 lightDir;      // Light direction
uniform vec3 viewPos;       // Camera position
uniform vec3 lightColor;    // Light color

uniform sampler2D textureSampler;  // Texture sampler
uniform bool useTexture;           // Flag to enable/disable texture

uniform bool uIsSelected;
uniform vec3 uHighlightColor;
uniform float uHighlightStrength;


void main()
{
    // Check if drawing edges (black)
    if (objectColor.r < 0.1 && objectColor.g < 0.1 && objectColor.b < 0.1) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Get base color (either from texture or objectColor uniform)
    vec3 baseColor;
    if (useTexture) {
        baseColor = texture(textureSampler, TexCoord).rgb;
    } else {
        baseColor = objectColor;
    }
    
    // Ambient lighting (base light level)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting (angle between normal and light)
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular lighting (shiny highlights)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Combine all lighting components
    vec3 result = (ambient + diffuse + specular) * baseColor;

    // Apply selection tint
    if (uIsSelected) {
    result = mix(result, uHighlightColor, uHighlightStrength);
    }
    FragColor = vec4(result, 1.0);



}