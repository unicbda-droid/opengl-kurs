#include "Camera.h"
#include <GL/glew.h>
#include <cmath>

static const float PI = 3.14159265f;

void Camera::move(float forward, float right, float up) {
    float radYaw = m_yaw * PI / 180.0f;
    m_x += std::sin(radYaw) * forward + std::sin(radYaw + PI / 2.0f) * right;
    m_z += std::cos(radYaw) * forward + std::cos(radYaw + PI / 2.0f) * right;
    m_y += up;
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    m_yaw += yawDelta;
    m_pitch += pitchDelta;
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;
}

void Camera::apply() {
    glRotatef(-m_pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-m_yaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-m_x, -m_y, -m_z);
}

void Camera::lookAt(float tx, float ty, float tz,
                    float yawDeg, float pitchDeg, float dist) {
    m_yaw = yawDeg;
    m_pitch = pitchDeg;
    float radYaw = m_yaw * PI / 180.0f;
    float radPitch = m_pitch * PI / 180.0f;
    m_x = tx + dist * std::cos(radPitch) * std::sin(radYaw);
    m_y = ty + dist * std::sin(radPitch);
    m_z = tz + dist * std::cos(radPitch) * std::cos(radYaw);
}

void Camera::orbit(float yawDelta, float pitchDelta,
                   float targetX, float targetY, float targetZ) {
    float dx = m_x - targetX;
    float dy = m_y - targetY;
    float dz = m_z - targetZ;
    float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (dist < 0.001f) dist = 1.0f;

    m_yaw += yawDelta;
    m_pitch += pitchDelta;
    if (m_pitch > 89.0f) m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    float radYaw = m_yaw * PI / 180.0f;
    float radPitch = m_pitch * PI / 180.0f;
    m_x = targetX + dist * std::cos(radPitch) * std::sin(radYaw);
    m_y = targetY + dist * std::sin(radPitch);
    m_z = targetZ + dist * std::cos(radPitch) * std::cos(radYaw);
}
