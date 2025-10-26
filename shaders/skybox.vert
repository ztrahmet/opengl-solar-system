#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view; // We remove the translation part

void main()
{
    // Remove translation from the view matrix so the skybox follows the camera
    mat4 viewNoTranslation = mat4(mat3(view));
    vec4 pos = projection * viewNoTranslation * vec4(aPos, 1.0);
    // Force depth to be 1.0 (furthest possible) by setting z = w
    gl_Position = pos.xyww;
    // Pass vertex position directly as texture coordinate for cubemap sampling
    TexCoords = aPos;
}
