#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 projection;
uniform vec3 objectColor;

out vec3 FragColor_vs; // Renamed to match fragment shader

void main()
{
    gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
    FragColor_vs = objectColor;
}
