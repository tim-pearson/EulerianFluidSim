#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D uDensity;

void main() {
    float density = texture(uDensity, vTexCoord).r;
    FragColor = vec4(mix(vec3(0.0,0.0,1.0), vec3(1.0,0.0,0.0), density), 1.0);
}

