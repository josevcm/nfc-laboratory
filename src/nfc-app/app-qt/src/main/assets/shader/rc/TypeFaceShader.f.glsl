#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 uObjectColor;

uniform sampler2D uSampler0;

in VertexData
{
    vec2 texel;
} vertex;

void main()
{
    gl_FragColor = texture2D(uSampler0, vertex.texel) * uObjectColor;
}