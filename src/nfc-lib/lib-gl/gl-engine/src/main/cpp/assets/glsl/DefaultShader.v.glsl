#version 430

attribute vec4 aVertexPoint;
attribute vec4 aVertexColor;

out VertexData
{
    vec4 color;
} vertex;

layout(binding = 0) uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 worldMatrix;
    mat4 normalMatrix;
    mat4 mvProjMatrix;
} matrix;

void main()
{
    vertex.color = aVertexColor;
    gl_Position  = matrix.mvProjMatrix * aVertexPoint;
}
