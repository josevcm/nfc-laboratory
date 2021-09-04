#ifdef GL_ES
precision mediump float;
#endif

uniform vec3 uAmbientLigthColor;
varying vec3 vVertexLigth;

void main()
{
    gl_FragColor = vec4(uAmbientLigthColor * vVertexLigth, 1.0);
}