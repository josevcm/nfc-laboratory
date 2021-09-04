#version 430

#ifdef GL_ES
precision mediump float;
#endif

struct Data {
    float range;
    float value;
    float frame[2];
};

in int peakMark;

layout(std430, binding = 0)
readonly buffer StorageBlock {
    Data data[];
} storage;

void main()
{
    if (peakMark > 0)
        gl_Position = vec4(storage.data[peakMark].range, storage.data[peakMark].value, 0.0, 1.0);
    else
        gl_Position = vec4(0, 0, 0, 0);
}
