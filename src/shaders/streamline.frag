#version 330 core
out vec4 FragColor;
uniform vec3 uColor = vec3(1.0, 0.0, 0.0); // red by default
void main() {
    FragColor = vec4(uColor, 1.0);
}

