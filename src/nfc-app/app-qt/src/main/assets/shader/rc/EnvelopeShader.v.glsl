#version 430

#ifdef GL_ES
precision mediump float;
#endif

in float dataRange; // data x range

struct Data {
    float range;
    float value;
    float frame[2];
};

layout(binding = 0)
uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 worldMatrix;
    mat4 normalMatrix;
    mat4 mvProjMatrix;
} matrix;

layout(std430, binding = 0)
readonly buffer StorageBlock {
    Data data[];
} storage;

void main()
{
    gl_Position = matrix.mvProjMatrix * vec4(dataRange, storage.data[gl_VertexID].value, -0.1, 1);
}
