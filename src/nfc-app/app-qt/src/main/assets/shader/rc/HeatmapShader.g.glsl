#version 430

#ifdef GL_ES
precision mediump float;
#endif

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform int renderPhase;

in VertexData {
    float range;
    float value;
} vertex[2];

out GeometryData {
    vec2 texel;
    vec4 color;
} geometry;

layout(binding = 0)
uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 worldMatrix;
    mat4 normalMatrix;
    mat4 mvProjMatrix;
} matrix;

void main()
{
    geometry.texel = vec2((vertex[1].value + 1) / 2, 0);
    gl_Position = matrix.mvProjMatrix * vec4(vertex[1].range, vertex[1].value, 0.1, 1.0);
    EmitVertex();

    geometry.texel = vec2((vertex[0].value + 1) / 2, 0);
    gl_Position = matrix.mvProjMatrix * vec4(vertex[0].range, vertex[0].value, 0.1, 1.0);
    EmitVertex();

    geometry.texel = vec2(0, 0);
    gl_Position = matrix.mvProjMatrix * vec4(vertex[1].range, -1.0, 0.1, 1.0);
    EmitVertex();

    geometry.texel = vec2(0, 0);
    gl_Position = matrix.mvProjMatrix * vec4(vertex[0].range, -1.0, 0.1, 1.0);
    EmitVertex();

    EndPrimitive();
}
