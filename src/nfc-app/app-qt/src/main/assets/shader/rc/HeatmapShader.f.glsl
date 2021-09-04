#version 430

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D uSampler0;

in GeometryData {
    vec2 texel;
    vec4 color;
} geometry;

void main()
{
    gl_FragColor = texture2D(uSampler0, geometry.texel) + geometry.color;
}