attribute vec4 aVertexPoint;
attribute vec4 aVertexNormal;

uniform   vec3 uDiffuseLightColor;
uniform   vec3 uDiffuseLightVector;

uniform   mat4 uModelVMatrix;
uniform   mat4 uNormalMatrix;
uniform   mat4 uMVProjMatrix;

varying   vec3 vVertexLight;

void main()
{
    vec3 vVertex  = vec3(uModelVMatrix * aVertexPoint);
    vec3 vNormal  = vec3(uNormalMatrix * aVertexNormal);
    vec3 vDiffuse = normalize(uDiffuseLightVector);
    
    vVertexLight = uDiffuseLightColor * max(dot(vDiffuse, vNormal), 0.0);
    
    gl_Position  = uMVProjMatrix * aVertexPoint;
}
