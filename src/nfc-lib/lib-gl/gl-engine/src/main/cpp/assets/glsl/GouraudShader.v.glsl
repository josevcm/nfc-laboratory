attribute vec4 aVertexPoint;
attribute vec4 aVertexNormal;

uniform   vec3 uDiffuseLigthColor;
uniform   vec3 uDiffuseLigthVector;

uniform   mat4 uModelVMatrix;
uniform   mat4 uNormalMatrix;
uniform   mat4 uMVProjMatrix;

varying   vec3 vVertexLigth;

void main()
{
    vec3 vVertex  = vec3(uModelVMatrix * aVertexPoint);
    vec3 vNormal  = vec3(uNormalMatrix * aVertexNormal);
    vec3 vDiffuse = normalize(uDiffuseLigthVector);
    
    vVertexLigth = uDiffuseLigthColor * max(dot(vDiffuse, vNormal), 0.0);
    
    gl_Position  = uMVProjMatrix * aVertexPoint;
}
