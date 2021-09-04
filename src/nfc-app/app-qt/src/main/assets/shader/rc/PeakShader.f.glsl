#version 430

#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 uObjectColor;

void main()
{
    gl_FragColor = uObjectColor;
}