#version 330 core

out vec4 FragColor;

in vec3 FragPos;    // From vertex shader
in vec3 Normal;     // From vertex shader
in vec2 TexCoord;   // Texture coordinates
in vec4 FragPosLightSpace; // For vertex shader

uniform vec3 objectColor;
uniform vec3 lightDir;      // Light direction
uniform vec3 viewPos;       // Camera position
uniform vec3 lightColor;    // Light color

uniform sampler2D textureSampler;  // Texture sampler
uniform sampler2D shadowMap; // shadow depth texture
uniform bool useTexture;           // Flag to enable/disable texture


uniform bool uIsSelected;
uniform vec3 uHighlightColor;
uniform float uHighlightStrength;

// Shadow calculation function
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Outside shadow map = no shadow
    if(projCoords.z > 1.0)
        return 0.0;
    
    // Get closest depth from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get current fragment depth
    float currentDepth = projCoords.z;
    
    // Bias to prevent shadow acne
    float bias = 0.005;
    
    // PCF (Percentage Closer Filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

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
    float ambientStrength = 0.3;
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

    // Calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);
    
    // Apply shadow (only affects diffuse and specular, not ambient)
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
    vec3 result = lighting * baseColor;

    // Apply selection tint
    if (uIsSelected) {
    result = mix(result, uHighlightColor, uHighlightStrength);
    }
    FragColor = vec4(result, 1.0);



}