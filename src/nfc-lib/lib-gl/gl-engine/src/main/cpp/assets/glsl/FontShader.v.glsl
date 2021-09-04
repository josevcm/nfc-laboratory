attribute vec4 aVertexPoint;
attribute vec2 aVertexTexel;

uniform   mat4 uMVProjMatrix;

varying   vec2 vTexelCoord;

void main()
{
    vTexelCoord = aVertexTexel;
    
    gl_Position  = uMVProjMatrix * aVertexPoint;
}
