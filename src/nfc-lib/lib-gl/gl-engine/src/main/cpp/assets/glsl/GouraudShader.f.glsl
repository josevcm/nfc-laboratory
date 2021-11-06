#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 uAmbientLightColor;
varying vec3 vVertexLight;

void main()
{
    gl_FragColor = vec4(uAmbientLightColor * vVertexLight, 1.0);
}