#include "Structs.glsl"

uniform Material material;
uniform Light light;

out vec4 fragColor;

in VS_OUT
{
    vec3 position;
    vec3 normal;
    vec3 texcoord;
    vec3 eye;
} fs_in;

vec3 getPosition() { return fs_in.position; }
vec3 getEye() { return fs_in.eye; }

#include "LightingFunctions.glsl"

void main()
{
    if (toDiscard()) discard;

    vec3 color = computeLighting();

    fragColor = vec4(color, 1.0);
}
