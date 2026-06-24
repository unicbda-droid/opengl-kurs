#pragma once

class Camera {
public:
    Camera() = default;

    void move(float forward, float right, float up);
    void rotate(float yawDelta, float pitchDelta);
    void apply();

    void setPosition(float x, float y, float z) { m_x = x; m_y = y; m_z = z; }
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getZ() const { return m_z; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }

    void setSpeedMultiplier(float s) { m_speedMul = s; }
    float getSpeedMultiplier() const { return m_speedMul; }

    void setOrtho(bool o) { m_ortho = o; }
    bool isOrtho() const { return m_ortho; }

    void lookAt(float tx, float ty, float tz,
                float yawDeg, float pitchDeg, float dist);
    void orbit(float yawDelta, float pitchDelta,
               float targetX, float targetY, float targetZ);

private:
    float m_x = 0.0f, m_y = 5.0f, m_z = 20.0f;
    float m_yaw = 0.0f;
    float m_pitch = -15.0f;
    float m_speedMul = 1.0f;
    bool m_ortho = false;
};
