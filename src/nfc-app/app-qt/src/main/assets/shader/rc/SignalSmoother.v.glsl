#version 430

#ifdef GL_ES
precision mediump float;
#endif

struct Data
{
    float range;
    float value;
    float frame[2];
};

// FFT complex input and block number
in      float dataRange;
in      float dataValue;
uniform int   dataBlock;

// FFT process parameters
layout(binding = 0)
uniform ConfigBlock {
    float length;
    float scale;
    float offset;
    float attack;
    float decay;
} config;

// FFT compute buffer
layout(std430, binding = 0)
coherent buffer StorageBlock {
    Data data[];
} storage;

void main()
{
    int frame0 = (dataBlock+0)%2;
    int frame1 = (dataBlock+1)%2;

    float value = 2 * 10 * log10(dataValue / config.length);

    if (storage.data[gl_VertexID].frame[frame0] < value)
        storage.data[gl_VertexID].frame[frame1] = storage.data[gl_VertexID].frame[frame0] * (1.0 - config.attack) + value * config.attack;
    else if (storage.data[gl_VertexID].frame[frame0] > value)
        storage.data[gl_VertexID].frame[frame1] = storage.data[gl_VertexID].frame[frame0] * (1.0 - config.decay) + value * config.decay;
    else
        storage.data[gl_VertexID].frame[frame1] = value;

    float samples = 1;
    float average = storage.data[gl_VertexID].frame[frame0];

    if (gl_VertexID > 0)
    {
        samples += 1;
        average += storage.data[gl_VertexID - 1].frame[frame0];
    }

    if (gl_VertexID > 1)
    {
        samples += 1;
        average += storage.data[gl_VertexID - 2].frame[frame0];
    }

    if (gl_VertexID < config.length - 1)
    {
        samples += 1;
        average += storage.data[gl_VertexID + 1].frame[frame0];
    }

    if (gl_VertexID < config.length - 2)
    {
        samples += 1;
        average += storage.data[gl_VertexID + 2].frame[frame0];
    }

    storage.data[gl_VertexID].range = dataRange;
    storage.data[gl_VertexID].value = clamp(config.offset + average / (samples * config.scale), -1.0, 1.0);

    gl_Position = vec4(0, 0, 2, 1);
}
