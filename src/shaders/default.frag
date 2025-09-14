#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uDensity;
uniform sampler2D uPressure;
uniform usampler2D uObstacle;

uniform int uMode; // 0 = density, 1 = pressure

vec3 pressureColormap(float p) {
    // Map [-1,1] pressure to colors (blue -> white -> red)
    float t = clamp((p + 1.0) * 0.5, 0.0, 1.0);
    return mix(vec3(0.0,0.0,1.0), vec3(1.0,0.0,0.0), t);
}

void main() {
    vec2 texCoord = vec2(1.0 - vTexCoord.y, vTexCoord.x);

    // Check obstacle
    uint obstacle = texture(uObstacle, texCoord).r;
    if (obstacle != 1u) {
        FragColor = vec4(0.3, 0.3, 0.3, 1.0); // walls always gray
        return;
    }

    // Background: either density or pressure
    if (uMode == 0) {
        float density = texture(uDensity, texCoord).r;
        vec3 color = mix(vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0), density);
        FragColor = vec4(color, 1.0);
    } else if (uMode == 1) {
        float pressure = texture(uPressure, texCoord).r;
        vec3 color = pressureColormap(pressure);
        FragColor = vec4(color, 1.0);
    }
}

