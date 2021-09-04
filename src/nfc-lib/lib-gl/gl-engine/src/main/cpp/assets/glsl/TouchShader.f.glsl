precision mediump float;

#define pi 3.141592653589793238462643383279

#define border 0.025
#define radius 0.450
#define width  0.025

uniform vec2 vTouchVector;

varying vec2 vTextelCoord;

void main() 
{
    vec4 color0 = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 color1 = vec4(0.8, 0.9, 0.9, 1.0);
    vec4 color2 = vec4(0.0, 1.0, 0.0, 1.0);
    
    // vector del pixel dentro del recuadro
    vec2 vPixelVector = vTextelCoord - vec2(0.5, 0.5);
    
    // distancia del pixel al centro
    float d = length(vPixelVector);

	// distancia del puntero al centro 
    float l = length(vTouchVector);
    
    // angulo entre vector pixel y vector puntero
    float a = acos(dot(vPixelVector, vTouchVector) / (d * l));

	// funcion de distribucion
	float t = 1.0 - smoothstep(radius - width - border, radius - width, d) + smoothstep(radius, radius + border, d);
	
	// color final del pixel
    gl_FragColor = mix(a < pi / (1.0 + l * 50.0) ? color2:color1, color0, t);
}