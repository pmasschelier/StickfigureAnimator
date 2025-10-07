#version 460

uniform float joint_radius;

#define STICK 0
#define CIRCLE 1

struct stick {
    vec2 start;
    vec2 end;
    vec4 color;
    uint type;
    float thickness;
};

layout(std430, binding = 0) readonly buffer stickfigure {
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

float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

float sdStick(in vec2 p, in stick s) {
    float d = 0.0;
    if(s.type == 0)
        d = sdSegment(fragTexCoord, s.start, s.end);
    else if(s.type == 1)
        d = abs(sdCircle(fragTexCoord - 0.5 * (s.start + s.end), 0.5 * distance(s.start, s.end)));
    return d - 0.5 * s.thickness;
}

void main()
{
    vec4 finalColor = vec4(0.0);
    float d = sdStick(fragTexCoord, sticks[0]);
    float d_joints = min(sdCircle(fragTexCoord - sticks[0].start, joint_radius), sdCircle(fragTexCoord - sticks[0].end, joint_radius));
    for(int i = 1; i < sticks.length(); i++) {
        d = min(d, sdStick(fragTexCoord, sticks[i]));
        d_joints = min(d_joints, min(sdCircle(fragTexCoord - sticks[i].start, joint_radius), sdCircle(fragTexCoord - sticks[i].end, joint_radius)));
    }
    float alpha = 1.0 - step(0.0, d); 
    vec4 color = vec4(sticks[0].color.rgb * alpha, alpha);
    float alpha_joints = 1.0 - step(0, d_joints);
    vec4 color_joints = vec4(alpha_joints, 0.0, 0.0, alpha_joints);
    finalColor = vec4(color_joints + color * (1 - alpha_joints));
    FragColor = finalColor;
}
