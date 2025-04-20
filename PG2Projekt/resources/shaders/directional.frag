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

// Struktura pro Spotlight (čelová baterka)
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
    
    float cutOff;
    float outerCutOff;
};

// Uniform pro spotlight
uniform SpotLight spotLight;
uniform bool spotLightEnabled = false;  // Ovládání zapnutí/vypnutí

// Pozice kamery (pro výpočet spekulární složky) - world space
uniform vec3 viewPos = vec3(0.0, 0.0, 0.0);             // Pozice kamery ve world space

// Textura
uniform sampler2D tex0;

// Příznak průhlednosti
uniform bool transparent = false;                        // Je objekt průhledný?
uniform vec4 u_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0); // Barva a průhlednost

// Funkce pro výpočet vlivu spotlightu (čelové baterky)
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Úhel mezi aktuálním směrem a směrem světla
    float theta = dot(lightDir, normalize(-light.direction));
    
    // Výpočet intenzity na základě plynulého přechodu mezi vnitřním a vnějším úhlem
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // Ambient složka (vždy přítomná i mimo kužel)
    vec3 ambient = light.ambient * ambientMaterial;
    
    // Pokud jsme uvnitř kužele, přidáme diffuse a specular složky
    vec3 result = ambient;
    
    if (theta > light.outerCutOff) {
        // Diffuse složka
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = light.diffuse * (diff * diffuseMaterial) * intensity;
        
        // Specular složka (Phong)
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = light.specular * (spec * specularMaterial) * intensity;
        
        // Útlum na základě vzdálenosti
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        // Aplikace útlumu na všechny složky
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
        
        result = ambient + diffuse + specular;
    }
    
    return result;
}

void main() {
    // Normalizace vektorů
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    
    // Výpočet pro směrové světlo (slunce)
    vec3 lightDirection = normalize(-lightDir); // Otočíme směr světla
    
    // Ambient složka
    vec3 ambient = lightAmbient * ambientMaterial;
    
    // Diffuse složka
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = lightDiffuse * (diff * diffuseMaterial);
    
    // Specular složka (Phong)
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightSpecular * (spec * specularMaterial);
    
    // Kombinace složek směrového světla
    vec3 result = ambient + diffuse + specular;
    
    // Přidání vlivu čelové baterky, pokud je zapnutá
    if (spotLightEnabled) {
        result += CalcSpotLight(spotLight, normal, fs_in.FragPos, viewDir);
    }
    
    // Textura
    vec4 texColor = texture(tex0, fs_in.TexCoord);
    
    // Aplikace textury a průhlednosti
    if (transparent) {
        // Pro průhledné objekty použijeme u_diffuse_color
        FragColor = vec4(result, 1.0) * vec4(texColor.rgb, 1.0) * u_diffuse_color;
    } else {
        // Pro neprůhledné objekty
        FragColor = vec4(result, 1.0) * texColor;
    }
}