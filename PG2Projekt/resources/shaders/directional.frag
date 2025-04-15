#version 460 core
// Výstup - barva v RGBA
out vec4 FragColor;

// Vstup z vertex shaderu
in VS_OUT {
    vec3 Normal;   // Normála ve world space
    vec3 FragPos;  // Pozice fragmentu ve world space 
    vec2 TexCoord; // Texturové koordináty
} fs_in;

// Vlastnosti materiálu
uniform vec3 ambientMaterial = vec3(0.2, 0.2, 0.2);     // Ambient složka materiálu
uniform vec3 diffuseMaterial = vec3(1.0, 1.0, 1.0);     // Diffuse složka materiálu
uniform vec3 specularMaterial = vec3(1.0, 1.0, 1.0);    // Specular složka materiálu
uniform float shininess = 32.0;                         // Lesklost materiálu

// Vlastnosti směrového světla - v world space
uniform vec3 lightDir = vec3(0.0, -1.0, -1.0);          // Směr světla (výchozí hodnota)
uniform vec3 lightAmbient = vec3(0.2, 0.2, 0.2);        // Ambient složka světla
uniform vec3 lightDiffuse = vec3(0.8, 0.8, 0.8);        // Diffuse složka světla
uniform vec3 lightSpecular = vec3(1.0, 1.0, 1.0);       // Specular složka světla

// Pozice kamery (pro výpočet spekulární složky) - world space
uniform vec3 viewPos = vec3(0.0, 0.0, 0.0);             // Pozice kamery ve world space

// Textura
uniform sampler2D tex0;

// Příznak průhlednosti
uniform bool transparent = false;                        // Je objekt průhledný?
uniform vec4 u_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0); // Barva a průhlednost

void main() {
    // Normalizace vektorů
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDirection = normalize(-lightDir); // Otočíme směr světla
    
    // Ambient složka
    vec3 ambient = lightAmbient * ambientMaterial;
    
    // Diffuse složka
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = lightDiffuse * (diff * diffuseMaterial);
    
    // Specular složka (Phong)
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightSpecular * (spec * specularMaterial);
    
    // Textura
    vec4 texColor = texture(tex0, fs_in.TexCoord);
    
    // Kombinace složek
    vec3 result = ambient + diffuse;
    
    // Aplikace textury, průhlednosti a spekulární složky
    if (transparent) {
        // Pro průhledné objekty použijeme u_diffuse_color
        FragColor = vec4(result, 1.0) * vec4(texColor.rgb, 1.0) * u_diffuse_color + vec4(specular, 0.0);
    } else {
        // Pro neprůhledné objekty
        FragColor = vec4(result, 1.0) * texColor + vec4(specular, 0.0);
    }
}