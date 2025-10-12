#version 460

uniform vec4 cameraViewport;
uniform vec4 worldViewport;
uniform vec4 selection;
uniform float selectionThickness;
uniform vec2 resolution;

in vec2 worldPos;
out vec4 FragColor;

float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

void main() {
    float dViewport = sdBox(worldPos - (cameraViewport.xy + 0.5 * cameraViewport.zw), abs(0.5 * cameraViewport.zw));
    float outsideFactor = step(0.0, dViewport);
    vec4 overlay = outsideFactor * vec4(vec3(0.0), 0.5);

    vec2 screenRatio = resolution / worldViewport.zw;
    vec4 screenSelection = vec4(selection.xy - worldViewport.xy, selection.zw) * screenRatio.xyxy;
    vec2 screenPos = (worldPos - worldViewport.xy) * screenRatio;
    float dSelection = abs(sdBox(screenPos - (screenSelection.xy + 0.5 * screenSelection.zw), abs(0.5 * screenSelection.zw))) - 0.5;
    float selectionFactor = clamp(0.5 - dSelection, 0.0, 1.0);
    vec4 selection = selectionFactor * vec4(1.0, 0.0, 0.0, 1.0);

    FragColor = overlay * (1 - selectionFactor) + selection;
}