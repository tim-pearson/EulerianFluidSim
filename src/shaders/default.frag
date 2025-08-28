#version 330 core
in float vDensity;
out vec4 FragColor;

void main()
{
    // simple blue-to-red gradient
    vec3 color = mix(vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0), vDensity);
    FragColor = vec4(color, 1.0);
}
