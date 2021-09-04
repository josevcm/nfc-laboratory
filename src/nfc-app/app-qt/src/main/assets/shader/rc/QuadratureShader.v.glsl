#version 430

#ifdef GL_ES
precision mediump float;
#endif

attribute vec2  dataValue; // complex signal value

layout(binding = 0)
uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 worldMatrix;
    mat4 normalMatrix;
    mat4 mvProjMatrix;
} matrix;

void main()
{
    gl_Position = matrix.mvProjMatrix * vec4(dataValue.x, dataValue.y, 0.0, 1.0);
}
