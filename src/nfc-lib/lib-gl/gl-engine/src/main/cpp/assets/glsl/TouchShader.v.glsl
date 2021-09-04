attribute vec4 vVertexCoord;
attribute vec2 vVertexTextel;

uniform mat4 mMVProjMatrix;

varying vec2 vTextelCoord;

void main()
{
    vTextelCoord = vVertexTextel;

    gl_Position  = mMVProjMatrix * vVertexCoord;
}
