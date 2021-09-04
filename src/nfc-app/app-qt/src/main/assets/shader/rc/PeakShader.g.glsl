#version 430

#ifdef GL_ES
precision mediump float;
#endif

layout (points) in;
layout (line_strip, max_vertices = 5) out;

layout(binding = 0)
uniform MatrixBlock {
    mat4 modelMatrix;
    mat4 worldMatrix;
    mat4 normalMatrix;
    mat4 mvProjMatrix;
} matrix;

void main()
{
    gl_Position = matrix.mvProjMatrix * (gl_in[0].gl_Position + vec4(-0.04, -0.04, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix.mvProjMatrix * (gl_in[0].gl_Position + vec4(+0.04, -0.04, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix.mvProjMatrix * (gl_in[0].gl_Position + vec4(+0.04, +0.04, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix.mvProjMatrix * (gl_in[0].gl_Position + vec4(-0.04, +0.04, 0.0, 0.0));
    EmitVertex();

    gl_Position = matrix.mvProjMatrix * (gl_in[0].gl_Position + vec4(-0.04, -0.04, 0.0, 0.0));
    EmitVertex();

    EndPrimitive();
}
