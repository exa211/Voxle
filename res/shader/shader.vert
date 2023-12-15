#version 450
#extension GL_EXT_debug_printf : enable

// Time to deprecate :(
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 col;
} ubo;

layout(push_constant) uniform constants {
    vec4 data;
    mat4 transform;
} PushConstants;
// Packed vertex values
layout(location = 0) in uint packedVertData;

layout(location = 1) out vec3 texCoord_Layer;

vec2 texCoord[4] = vec2[4](
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f),
    vec2(0.0, 1.0f)
);

void main() {
    // Reversed little endian order
    // Needs fixing i guess? But im really bad at shifting bits.
    uint z = packedVertData & 255u;
    uint y = packedVertData / 256 & 255u;
    uint x = packedVertData / (256*256) & 255u;
    uint w = packedVertData / (256*256*256) & 255u;

    gl_Position = PushConstants.transform * vec4(x, y, z, 1.0);

    // Out texture UV's and Array Depth
    texCoord_Layer = vec3(texCoord[w], 1);
}