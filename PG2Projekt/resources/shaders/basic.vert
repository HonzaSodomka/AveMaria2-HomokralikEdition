#version 460 core
in vec3 attribute_Position;
out vec3 vertexPosition;
uniform float time; // �as pro animaci

void main() {
    // Zmen�en� a posunut�
    vec3 transformed = attribute_Position * 0.18; // Zv�t�en� na 15%
    transformed.y += 0.0;  // ��dn� posunut� dol�
    transformed.x += 0.0;  // ��dn� posunut� do stran
    
    // Rotace kolem osy Y
    float angle = time * 0.5;
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);
    
    // Aplikace rotace
    float x = transformed.x * cosAngle - transformed.z * sinAngle;
    float z = transformed.x * sinAngle + transformed.z * cosAngle;
    transformed.x = x;
    transformed.z = z;
    
    // P�ed�n� pozice do fragment shaderu
    vertexPosition = transformed;
    
    gl_Position = vec4(transformed, 1.0);
}