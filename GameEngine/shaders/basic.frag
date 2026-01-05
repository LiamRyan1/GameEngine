#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 objectColor;
uniform sampler2D texture1;
uniform bool useTexture;

void main()
{
    if (useTexture) {
        FragColor = texture(texture1, TexCoord);
    }else {
        FragColor = vec4(objectColor, 1.0);
    }
}