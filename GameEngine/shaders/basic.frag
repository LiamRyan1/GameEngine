#version 330 core

out vec4 FragColor;
uniform vec3 objectColor;

void main()
{
    // Check if objectColor is black (edges)
    if (objectColor.r < 0.1 && objectColor.g < 0.1 && objectColor.b < 0.1) {
        // Draw black edges
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // Draw rainbow for faces
        float hue = mod(gl_FragCoord.x / 100.0, 1.0);
        vec3 rainbow = vec3(
            sin(hue * 6.28),
            sin(hue * 6.28 + 2.09),
            sin(hue * 6.28 + 4.18)
        ) * 0.5 + 0.5;
        FragColor = vec4(rainbow, 1.0);
    }
}