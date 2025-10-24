#version 460

uniform vec2 res;
uniform vec3 color;

out vec4 finalColor;

float f(in float n, in vec3 hsv) {
    float k = mod(n + hsv.x / 60.0, 6);
    return hsv.z * (1 - hsv.y * clamp(min(k, 4 - k), 0.0, 1.0));
}

vec3 hsv2rgb(in vec3 hsv) {
    return vec3(f(5, hsv), f(3, hsv), f(1, hsv));
}

float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

float distToAlpha(in float dist) {
    return clamp(0.5 - dist / max(fwidth(dist), 1e-7), 0.0, 1.0);
}

void main() {
    vec2 pos = vec2(gl_FragCoord.x, res.y - gl_FragCoord.y);
    vec3 hsv = vec3(color.x, pos / res);
    finalColor = vec4(hsv2rgb(hsv), 1.0);

    float dPointer = abs(sdCircle(pos - (res * color.yz), 5.0)) - 2.f;
    float aPointer = distToAlpha(dPointer);
    finalColor *= (1 - aPointer);
    finalColor += aPointer * vec4(hsv2rgb(vec3(mod(hsv.x + 180, 360), 1.0, 1.0)), 1.0);
}