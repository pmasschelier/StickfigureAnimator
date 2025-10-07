#version 460

uniform vec4 viewport;

in vec2 fragTexCoord;
out vec4 FragColor;

void main() {
    float outsideFactor = 1 - (
        step(viewport.x, fragTexCoord.x)
        * step(viewport.y, fragTexCoord.y)
        * (1 - step(viewport.x + viewport.z, fragTexCoord.x))
        * (1 - step(viewport.y + viewport.w, fragTexCoord.y))
    );
    FragColor = vec4(vec3(0.0), outsideFactor * 0.5);
}
