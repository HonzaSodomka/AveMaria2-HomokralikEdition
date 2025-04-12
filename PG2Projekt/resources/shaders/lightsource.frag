#version 460 core
out vec4 FragColor;

// Vlastnosti světla
uniform vec3 lightColor;

void main()
{
    // Světelný zdroj zobrazíme jako jasný objekt s barvou světla
    FragColor = vec4(lightColor, 1.0);
}