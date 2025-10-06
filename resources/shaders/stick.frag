#version 460

// uniform vec2 start;
// uniform vec2 end;
uniform float joint_radius;
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

float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

void main()
{
    float d = sdSegment(fragTexCoord, sticks[0].start, sticks[0].end);
    float d_joints = min(sdCircle(fragTexCoord - sticks[0].start, joint_radius), sdCircle(fragTexCoord - sticks[0].end, joint_radius));
    for(int i = 1; i < sticks.length(); i++) {
        d = min(d, sdSegment(fragTexCoord, sticks[i].start, sticks[i].end));
        d_joints = min(d_joints, min(sdCircle(fragTexCoord - sticks[i].start, joint_radius), sdCircle(fragTexCoord - sticks[i].end, joint_radius)));
    }
    float alpha = 1.0 - step(sticks[0].thickness, d); 
    vec4 color = vec4(sticks[0].color.rgb * alpha, alpha);
    float alpha_joints = 1.0 - step(0, d_joints);
    vec4 color_joints = vec4(alpha_joints, 0.0, 0.0, alpha_joints);
    FragColor = vec4(color_joints + color * (1 - alpha_joints));
}
