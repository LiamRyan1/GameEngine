#version 330 core

layout (location = 0) in vec3 aPos;      // Position
layout (location = 1) in vec3 aNormal;   // Normal
layout (location = 2) in vec2 aTexCoord; // Texture Coordinate


// Uniform matrices for transformations
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // Matrix to transform to light space for shadow mapping


out vec3 FragPos;   // Position in world space
out vec3 Normal;    // Normal in world space
out vec2 TexCoord;   // Texture coordinates
out vec4 FragPosLightSpace; // Position in light space for shadow mapping

void main()
{
     // Transform position to world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal to world space (use normal matrix to preserve perpendicularity)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); // Calculate position in light space for shadow mapping

    // Final position in clip space
    gl_Position = projection * view * vec4(FragPos, 1.0);
}