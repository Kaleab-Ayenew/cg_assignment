#version 330 core
out vec4 FragColor;

in vec3 FragColor_vs; // Renamed to avoid confusion with output

void main()
{
    FragColor = vec4(FragColor_vs, 1.0);
}
