#version 430

#ifdef GL_ES
precision mediump float;
#endif

struct Data {
    float range;
    float value;
    float frame[2];
};

in float dataRange;// data x range

out VertexData {
    float range;
    float value;
} vertex;

layout(std430, binding = 0)
readonly buffer StorageBlock {
    Data data[];
} storage;

void main()
{
    vertex.range = dataRange;
    vertex.value = storage.data[gl_VertexID].value;
}
