#ifdef GL_ES
precision highp float;
#endif

in VertexData
{
    vec4 color;
} vertex;

void main()
{
    gl_FragColor = vertex.color;
}