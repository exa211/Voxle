#version 450

layout(location = 1) in vec3 texCoord_Layer;
layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) out vec4 outColor;

// Textures in array xy -> uv | z -> layer depth of array (which texture to use)
//layout(binding = 2) uniform sampler2DArray textures;

void main() {
    vec4 texCol = texture(texSampler, texCoord_Layer.xy);
    if(texCol.a == 0) discard; // Discard pixel if no alpha
    outColor = texCol;
}