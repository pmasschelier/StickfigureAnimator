#version 460

uniform float joint_radius;

#define STICK 0
#define CIRCLE 1

struct stick {
    vec4 color;
    uint start;
    uint end;
    uint type;
    float thickness;
};

layout(std430, binding = 0) readonly buffer StickfigureEdges {
    stick sticks[];
};

layout(std430, binding = 1) readonly buffer StickfigureJoints {
    vec2 joints[];
};

in vec2 worldPos;

out vec4 FragColor;

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

float sdCircle(in vec2 p, in float r )
{
    return length(p) - r;
}

float sdStick(in vec2 p, in stick s) {
    float d = 0.0;
    vec2 start = joints[s.start];
    vec2 end = joints[s.end];
    if(s.type == 0)
        d = sdSegment(worldPos, start, end);
    else if(s.type == 1)
        d = abs(sdCircle(worldPos - 0.5 * (start + end), 0.5 * distance(start, end)));
    return d - 0.5 * s.thickness;
}

void main()
{
    vec4 finalColor = vec4(0.0);
    float dEdges = sdStick(worldPos, sticks[0]);
    for(int i = 1; i < sticks.length(); i++) {
        dEdges = min(dEdges, sdStick(worldPos, sticks[i]));
    }
    float dPivot = sdCircle(worldPos - joints[0], joint_radius);
    float dJoints = sdCircle(worldPos - joints[1], joint_radius);
    for(int i = 2; i < joints.length(); i++) {
        dJoints = min(dJoints, sdCircle(worldPos - joints[i], joint_radius));
    }
    float aEdges = 1.0 - step(0.0, dEdges); 
    vec4 color = vec4(sticks[0].color.rgb * aEdges, aEdges);
    float aPivot = 1.0 - step(0, dPivot);
    color *= (1 - aPivot);
    color += aPivot * vec4(1.0, 0.6, 0.6, 1.0);
    float aJoints = 1.0 - step(0, dJoints);
    color *= (1 - aJoints);
    color += aJoints * vec4(1.0, 0.0, 0.0, 1.0);
    FragColor = color;
}
