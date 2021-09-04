#version 430

#ifdef GL_ES
precision mediump float;
#endif

in vec4 aVertexPoint;
in vec2 aVertexTexel;

out VertexData
{
    vec2 texel;
} vertex;

layout(binding = 0)
uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 worldMatrix;
    mat4 normalMatrix;
    mat4 mvProjMatrix;
} matrix;

varying   vec2 vTexelCoord;

void main()
{
    vertex.texel = aVertexTexel;
    gl_Position  = matrix.mvProjMatrix * aVertexPoint;
}
