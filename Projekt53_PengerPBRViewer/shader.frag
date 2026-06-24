#version 120
varying vec2 vUV;
varying vec3 vPos;
varying vec3 vNormal;

uniform sampler2D uDiffuse;
uniform sampler2D uNormal;
uniform sampler2D uBump;
uniform sampler2D uHeight;

uniform vec3 uLightPos;
uniform vec3 uLightColor;
uniform vec3 uAmbient;

void main() {
    vec3 N = normalize(vNormal);

    vec3 L = normalize(uLightPos - vPos);
    vec3 V = normalize(-vPos);
    vec3 H = normalize(L + V);

    vec3 diffColor = texture2D(uDiffuse, vUV).rgb;

    vec3 bump = texture2D(uBump, vUV).rgb;
    float height = texture2D(uHeight, vUV).r;
    vec3 normalTS = texture2D(uNormal, vUV).rgb * 2.0 - 1.0;
    normalTS = normalize(normalTS);

    vec3 wNormal = normalize(N);
    vec3 tangent = normalize(cross(wNormal, vec3(0.0, 1.0, 0.0)));
    if (length(tangent) < 0.01) tangent = normalize(cross(wNormal, vec3(1.0, 0.0, 0.0)));
    vec3 bitangent = normalize(cross(wNormal, tangent));

    vec3 N_final = normalize(
        tangent * normalTS.x +
        bitangent * normalTS.y +
        wNormal * normalTS.z
    );

    float diff = max(dot(N_final, L), 0.0);
    float spec = pow(max(dot(N_final, H), 0.0), 32.0);

    vec3 color = diffColor * (uAmbient + uLightColor * diff) + uLightColor * spec * 0.5;

    gl_FragColor = vec4(color, 1.0);
}
