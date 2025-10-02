#version 460

// uniform vec2 start;
// uniform vec2 end;
// uniform float thickness;
// uniform vec4 color;

struct stick {
    vec2 start;
    vec2 end;
    vec4 color;
    float thickness;
};

layout(std430, binding = 0) buffer stickfigure {
    stick sticks[];
};

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
    float d = sdSegment(fragTexCoord, sticks[0].start, sticks[0].end);
    for(int i = 1; i < sticks.length(); i++) {
        d = min(d, sdSegment(fragTexCoord, sticks[i].start, sticks[i].end));
    }
    float alpha = 1.0 - step(sticks[0].thickness, d); 
    FragColor = vec4(sticks[0].color.rgb, alpha);
}
