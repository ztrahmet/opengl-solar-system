#version 330 core
out vec4 FragColor;

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Uniforms
uniform sampler2D ourTexture; // The planet's texture
uniform vec3 lightPos;        // Light source position (Sun's position) in world space
uniform vec3 viewPos;         // Camera position in world space
uniform vec3 lightColor;      // Color of the light (usually white)

void main()
{
    // --- Ambient Light ---
    // Provides a base level of light so dark areas aren't completely black.
    float ambientStrength = 0.1; // Low strength
    vec3 ambient = ambientStrength * lightColor;

    // --- Diffuse Light ---
    // Simulates directional light impact based on surface angle.
    vec3 norm = normalize(Normal); // Ensure normal vector is unit length
    vec3 lightDir = normalize(lightPos - FragPos); // Direction from fragment to light
    float diff = max(dot(norm, lightDir), 0.0); // Intensity based on angle (cosine)
    vec3 diffuse = diff * lightColor;

    // --- Specular Light ---
    // Simulates the bright highlight reflection of the light source.
    float specularStrength = 0.5; // Moderate strength
    vec3 viewDir = normalize(viewPos - FragPos); // Direction from fragment to camera
    vec3 reflectDir = reflect(-lightDir, norm); // Direction of reflected light
    // Use Blinn-Phong modification for slightly softer highlights
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0); // Shininess factor of 32
    // Alternative: Standard Phong
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess factor of 32
    vec3 specular = specularStrength * spec * lightColor;

    // Get the object's base color from its texture
    vec3 objectColor = texture(ourTexture, TexCoord).rgb;

    // --- Final Color ---
    // Combine ambient, diffuse, and specular components, modulated by the object's color.
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0); // Assume full opacity
}
