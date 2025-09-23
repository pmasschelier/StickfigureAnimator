#version 460

uniform vec2 start;
uniform vec2 end;
uniform float thickness;
uniform vec4 color;

out vec4 FragColor;

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

void main()
{
    float d = sdSegment(gl_FragCoord.xy, start, end);
    float alpha = 1 - step(thickness, d); 
    FragColor = vec4(color.rgb, alpha);
}
