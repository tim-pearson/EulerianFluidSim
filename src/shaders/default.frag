#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uDensity;
uniform sampler2D uPressure;
uniform isampler2D uObstacle;

// Map pressure [-1,1] to blue -> white -> red
vec3 pressureColormap(float p) {
    float t = clamp((p + 1.0) * 0.5, 0.0, 1.0);
    return mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), t);
}

void main() {
    vec2 texCoord = vec2(1.0 - vTexCoord.y, vTexCoord.x);

    int obstacle = texture(uObstacle, texCoord).r;
    if (obstacle == 0) {
        FragColor = vec4(0.3, 0.3, 0.3, 1.0); // walls gray
        return;
    }

    // Base density color (green)
    float density = texture(uDensity, texCoord).r;
    vec3 baseColor = mix(vec3(0.0), vec3(0.0, 1.0, 0.0), density);

    // Overlay pressure hotspots
    float pressure = texture(uPressure, texCoord).r;
    vec3 pressureColor = vec3(0.0); // default no hotspot
    float threshold = 0.9;
    if (abs(pressure) > threshold) {
        pressureColor = pressureColormap(pressure);
    }

    // Blend pressure on top of density (simple max for visibility)
    vec3 finalColor = max(baseColor, pressureColor);

    FragColor = vec4(finalColor, 1.0);
}

