#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D uDensity;

void main() {
    // Flip horizontal axis for left → right
    float density = texture(uDensity, vec2(1.0 - vTexCoord.y, vTexCoord.x)).r;
    FragColor = vec4(mix(vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0), density), 1.0);
}

