#ifdef GL_ES
precision mediump float;
#endif

const float threshold = 0.50;

uniform sampler2D uSampler0;

uniform vec4  uFontColor;       // = vec4(1.0, 1.0, 1.0, 1.0);   // color del relleno
uniform float uFontSmooth;      // = 0.04;

uniform vec4  uStrokeColor;     // = vec4(0.0, 0.0, 0.0, 1.0);   // color del trazo
uniform float uStrokeWidth;     // = 0.00;

uniform vec4  uShadowColor;     // = vec4(0.1, 0.0, 0.1, 1.0);   // color del sombreado
uniform vec2  uShadowOffset;    // = vec2(0.000, 0.000);
uniform float uShadowSmooth;    // = 0.05;

varying vec2  vTexelCoord;

void main()
{
    // factor de distancia
    float a = texture2D(uSampler0, vTexelCoord).a;
    
    // color base de la fuente
    vec4  c = uFontColor;
    
    // trazo 
    if (uStrokeWidth > 0.0 && a >= threshold - uStrokeWidth && a <= threshold + uStrokeWidth)
    {
        if (a <= threshold + uStrokeWidth)
        {
            if (uFontSmooth > 0.0)
            {
                c = mix(c, uStrokeColor, smoothstep(threshold + uStrokeWidth, threshold + uStrokeWidth - uFontSmooth, a));
            }
            else
            {
                c = uStrokeColor;
            }
        }
        else
        {
            c = uStrokeColor;
        }
    }    
   
    // suavizado
    if (uFontSmooth > 0.0 )
    {
        c.a = smoothstep(threshold - uStrokeWidth, threshold - uStrokeWidth + uFontSmooth, a);
    }
    else if (a < threshold - uStrokeWidth)
    {
        c.a = 0.0;
    }

    // sombreado
    if ((uShadowOffset.x > 0.0 || uShadowOffset.y > 0.0) && a < (threshold - uStrokeWidth))
    {
        float g = texture2D(uSampler0, vTexelCoord - uShadowOffset).a;
        
        c = uShadowColor;
        
        if (uShadowSmooth > 0.0)
        {
            c.a = smoothstep(threshold - uStrokeWidth, threshold - uStrokeWidth + uShadowSmooth, g);
        }
        else if (g < threshold - uStrokeWidth)
        {
            c.a = 0.0;
        }
    }
    
    gl_FragColor = c;
}