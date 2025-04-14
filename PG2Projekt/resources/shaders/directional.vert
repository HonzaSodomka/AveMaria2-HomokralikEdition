#version 460 core
// Vertex attributes
in vec3 aPos;
in vec3 aNorm;
in vec2 aTex;

// Matrices 
uniform mat4 uP_m = mat4(1.0f); // Projekční matice
uniform mat4 uV_m = mat4(1.0f); // View matice (kamera)
uniform mat4 uM_m = mat4(1.0f); // Model matice (pozice, rotace, měřítko objektu)

// Směrové světlo - vlastnosti
uniform vec3 lightDir = vec3(0.0, -1.0, -1.0); // Výchozí hodnota - světový prostor

// Výstupy do fragment shaderu
out VS_OUT {
    vec3 Normal;   // Normála ve world space
    vec3 FragPos;  // Pozice fragmentu ve world space
    vec2 TexCoord; // Texturové koordináty
} vs_out;

void main(void) {
    // Pozice vrcholu ve world space
    vec4 worldPos = uM_m * vec4(aPos, 1.0);
    
    // Předání pozice fragmentu ve world space
    vs_out.FragPos = worldPos.xyz;
    
    // Výpočet normály ve world space (ne view space)
    vs_out.Normal = mat3(transpose(inverse(uM_m))) * aNorm;
    
    // Předání texturových koordinátů
    vs_out.TexCoord = aTex;
    
    // Výpočet clip-space pozice každého vrcholu
    gl_Position = uP_m * uV_m * worldPos;
}