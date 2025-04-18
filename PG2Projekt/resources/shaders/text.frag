#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    // Získání hodnoty intenzity z textury (RED kanál)
    float sampled = texture(text, TexCoords).r;
    
    // Smíchání intenzity s barvou textu a použití intensity jako alpha kanálu
    color = vec4(textColor, 1.0) * sampled;
    
    // Pro lepší viditelnost můžeme přidat "obrys" kolem textu
    if (sampled > 0.05 && sampled < 0.9) {
        // Mírné zvýraznění okrajů pro lepší čitelnost
        color = vec4(textColor, sampled * 1.2);
    }
}