#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// This is the texture sampler
uniform sampler2D ourTexture;

void main() {
    // Sample the texture at the given texture coordinate
    FragColor = texture(ourTexture, TexCoord);
}
