#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Now we will use the normal
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;  // Fragment position in world space
out vec3 Normal;   // Normal vector in world space
out vec2 TexCoord; // Texture coordinate

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Calculate the normal vector in world space
    // Use the normal matrix (transpose(inverse(model))) to handle non-uniform scaling correctly
    Normal = mat3(transpose(inverse(model))) * aNormal;

    TexCoord = aTexCoord;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
