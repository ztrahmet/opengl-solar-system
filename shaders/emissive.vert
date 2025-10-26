#version 330 core
layout (location = 0) in vec3 aPos;
// Location 1 (aNormal) is unused but exists in the Planet VAO
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
