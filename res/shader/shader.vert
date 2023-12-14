#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 col;
} ubo;

layout( push_constant ) uniform constants {
    vec4 data;
    mat4 transform;
} PushConstants;

layout(location = 0) in vec3 inVertPos;
layout(location = 1) in vec3 inVertCol;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = PushConstants.transform * vec4(inVertPos, 1.0);
    fragColor = inVertCol;
    fragTexCoord = inTexCoord;
}