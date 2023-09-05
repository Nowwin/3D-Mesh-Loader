#version 410 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec4 color;  // RGBA color

out vec4 fragColor;

void main()
{
    // Ambient lighting
    vec3 ambient = 0.1 * vec3(color);

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(color);

    // Calculate final color
    //vec3 result = (ambient + diffuse) * vec3(color);
    vec3 result = vec3(color);
    fragColor = vec4(result, 1.0);
}
