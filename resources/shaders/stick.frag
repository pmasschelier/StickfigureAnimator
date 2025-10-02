#version 300 es

precision mediump float;

uniform vec2 start;
uniform vec2 end;
uniform float thickness;
uniform vec4 color;
uniform vec4 viewport;

in vec2 fragTexCoord;

out vec4 FragColor;

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

void main()
{
    float d = sdSegment(fragTexCoord, start, end);
    float alpha = 1.0 - step(thickness, d); 
    FragColor = vec4(color.rgb, alpha);
    // FragColor = vec4(fragTexCoord.x / 1000.0, fragTexCoord.y / 1000.0, 0.0, 1.0);
    // FragColor = vec4(gl_FragCoord.x / 1000.0, gl_FragCoord.y / 1000.0, 0.0, 1.0);
}
