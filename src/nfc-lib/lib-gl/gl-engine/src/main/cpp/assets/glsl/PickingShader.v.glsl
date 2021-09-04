attribute vec4 aVertexPoint;

uniform   mat4 uMVProjMatrix;

void main()
{
    gl_Position = uMVProjMatrix * aVertexPoint;
}
