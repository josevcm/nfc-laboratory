#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 uPickColor;

void main()
{
    gl_FragColor = vec4(uPickColor, 1.0);
}