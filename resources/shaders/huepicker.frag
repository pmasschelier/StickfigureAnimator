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

float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

float distToAlpha(in float dist) {
    return clamp(0.5 - dist / max(fwidth(dist), 1e-7), 0.0, 1.0);
}

void main() {
    vec2 pos = vec2(gl_FragCoord.x, res.y - gl_FragCoord.y);
    vec3 hsv = vec3(360 * pos.x / res.x, 1.0, 1.0);
    finalColor = vec4(hsv2rgb(hsv), 1.0);

    float dCursor = abs(pos.x - (0.5 + floor(res.x * (color.x / 360.0))));
    float dAlpha = 1 - step(1.5, dCursor);
    finalColor *= 1 - dAlpha;
    finalColor += dAlpha * vec4((1 - step(0.5, dCursor)) * vec3(1.0), 1.0);

    float d = sdBox(pos - res / 2.0, res / 2.0 - vec2(10)) - 10;
    float alpha = distToAlpha(d);
    finalColor = vec4(finalColor.xyz, alpha);
}