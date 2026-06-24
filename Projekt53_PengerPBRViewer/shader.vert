#version 120
attribute vec3 aPos;
attribute vec3 aNormal;
attribute vec2 aUV;

varying vec2 vUV;
varying vec3 vPos;
varying vec3 vNormal;

void main() {
    vUV = aUV;
    vPos = (gl_ModelViewMatrix * vec4(aPos, 1.0)).xyz;
    vNormal = normalize(gl_NormalMatrix * aNormal);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPos, 1.0);
}
