#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uDensity;
uniform usampler2D uObstacle;  // unsigned int texture for walls

void main() {
    // Flip axes for correct orientation
    vec2 texCoord = vec2(1.0 - vTexCoord.y, vTexCoord.x);

    // Density lookup
    float density = texture(uDensity, texCoord).r;

    // Obstacle lookup (0 = fluid, 1 = wall)
    uint obstacle = texture(uObstacle, texCoord).r;

    if (obstacle != 1u) {
        // Wall/obstacle color (e.g., solid gray)
        FragColor = vec4(0.3, 0.3, 0.3, 1.0);
    } else {
        // Density visualization (green scale)
        vec3 color = mix(vec3(0.0,0.0,0.0), vec3(0.0,1.0,0.0), density);
        FragColor = vec4(color, 1.0);
    }
}

