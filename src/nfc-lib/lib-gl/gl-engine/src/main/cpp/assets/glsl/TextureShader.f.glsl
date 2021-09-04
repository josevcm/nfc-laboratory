#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D uSampler0;

uniform vec4 uObjectColor;

varying vec2 vTexelCoord;

void main()
{
    gl_FragColor = texture2D(uSampler0, vTexelCoord) * uObjectColor;
}