#version 460 core
in vec3 attribute_Position;
out vec3 vertexPosition;
uniform float time; // Èas pro animaci

void main() {
    // Zmenšení a posunutí
    vec3 transformed = attribute_Position * 0.18; // Zvìtšení na 15%
    transformed.y += 0.0;  // Žádné posunutí dolù
    transformed.x += 0.0;  // Žádné posunutí do stran
    
    // Rotace kolem osy Y
    float angle = time * 0.5;
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    
    // Aplikace rotace
    float x = transformed.x * cosAngle - transformed.z * sinAngle;
    float z = transformed.x * sinAngle + transformed.z * cosAngle;
    transformed.x = x;
    transformed.z = z;
    
    // Pøedání pozice do fragment shaderu
    vertexPosition = transformed;
    
    gl_Position = vec4(transformed, 1.0);
}