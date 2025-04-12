#version 460 core
// (interpolated) input from previous pipeline stage
in vec3 FragPos;
in vec3 Normal;
in VS_OUT {
    vec2 texcoord;
} fs_in;
// uniform variables
uniform sampler2D tex0; // texture unit from C++
uniform vec4 u_diffuse_color = vec4(1.0f); // přidaný uniform pro barvu a průhlednost
// mandatory: final output color
out vec4 FragColor;

// Definice bodového světla
struct PointLight {
    vec3 position;
    vec3 color;
    
    float constant;
    float linear;
    float quadratic;
};

// Uniformy pro tři bodová světla
uniform PointLight lights[3]; // Původní světlo + modrý a červený světelný zdroj
uniform vec3 viewPos;

// Funkce pro výpočet vlivu bodového světla
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light.color;
  	
    // Diffuse 
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    // Specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light.color;  
    
    // Útlum
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Aplikace útlumu
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

void main() {
    // Normalizace normály
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Inicializace výsledné barvy na nulu
    vec3 result = vec3(0.0);
    
    // Příspěvek každého světla
    for(int i = 0; i < 3; i++) {
        result += CalcPointLight(lights[i], norm, FragPos, viewDir);
    }
    
    // Kombinace s texturou
    vec4 texColor = texture(tex0, fs_in.texcoord);
    FragColor = vec4(result, 1.0) * u_diffuse_color * texColor;
}