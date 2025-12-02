#pragma once
#include <glm/glm.hpp>

namespace CameraConstants {
constexpr glm::vec3 ORIGIN = glm::vec3(0.0f, 0.0f, 0.0f);

// ============ World Axis Vectors ============
constexpr glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
constexpr glm::vec3 WORLD_DOWN = glm::vec3(0.0f, -1.0f, 0.0f);
constexpr glm::vec3 WORLD_RIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
constexpr glm::vec3 WORLD_LEFT = glm::vec3(-1.0f, 0.0f, 0.0f);
constexpr glm::vec3 WORLD_FORWARD = glm::vec3(0.0f, 0.0f, -1.0f);
constexpr glm::vec3 WORLD_BACKWARD = glm::vec3(0.0f, 0.0f, 1.0f);

// ============ Default Camera Settings ============
namespace Defaults {
// Free Camera (FPS-style)
constexpr glm::vec3 FREE_CAMERA_POSITION = glm::vec3(0.0f, 2.0f, 5.0f);
constexpr float FREE_CAMERA_YAW = -90.0f;      // Looking towards -Z
constexpr float FREE_CAMERA_PITCH = 0.0f;      // Horizontal
constexpr float FREE_CAMERA_FOV = 45.0f;       // Degrees
constexpr float FREE_CAMERA_MOVE_SPEED = 5.0f; // Units per second
constexpr float FREE_CAMERA_SPRINT_MULT =
    3.0f; // Speed multiplier when sprinting
constexpr float FREE_CAMERA_SENSITIVITY = 0.1f; // Mouse look sensitivity
constexpr float FREE_CAMERA_ZOOM_SPEED = 2.0f;  // Scroll zoom speed

// Orbit Camera (Blender-style)
constexpr glm::vec3 ORBIT_CAMERA_TARGET = ORIGIN;
constexpr float ORBIT_CAMERA_DISTANCE = 10.0f; // Distance from target
constexpr float ORBIT_CAMERA_THETA = 0.0f;     // Azimuth angle (degrees)
constexpr float ORBIT_CAMERA_PHI = 30.0f;      // Elevation angle (degrees)
constexpr float ORBIT_CAMERA_FOV = 45.0f;
constexpr float ORBIT_CAMERA_SPEED = 0.5f;     // Orbit speed
constexpr float ORBIT_CAMERA_PAN_SPEED = 5.0f; // Pan target speed
constexpr float ORBIT_CAMERA_ZOOM_SPEED = 2.0f;

// Projection settings
constexpr float NEAR_PLANE = 0.1f;
constexpr float FAR_PLANE = 1000.0f; // For galaxy rendering, may need more
constexpr float MIN_FOV = 1.0f;      // Minimum field of view (degrees)
constexpr float MAX_FOV = 120.0f;    // Maximum field of view (degrees)

// Constraints
constexpr float MIN_PITCH = -89.0f; // Prevent gimbal lock
constexpr float MAX_PITCH = 89.0f;
constexpr float MIN_ORBIT_DIST = 1.0f;    // Minimum orbit distance
constexpr float MAX_ORBIT_DIST = 1000.0f; // Maximum orbit distance
} // namespace Defaults

// ============ Galaxy-Specific Constants ============
namespace Galaxy {
constexpr float PARSEC_TO_UNITS = 1.0f;         // Scale: 1 unit = 1 parsec
constexpr float LIGHTYEAR_TO_UNITS = 0.306601f; // 1 ly ≈ 0.3066 pc
constexpr float AU_TO_UNITS = 4.84814e-6f;      // 1 AU in parsecs

// Camera presets for different scales
constexpr float STAR_CLUSTER_FOV = 60.0f; // Wide view for clusters
constexpr float GALAXY_VIEW_FOV = 45.0f;  // Medium view for galaxies
constexpr float COSMIC_WEB_FOV = 30.0f;   // Narrow for large structures

constexpr float STAR_CLUSTER_DISTANCE = 100.0f;   // Parsecs
constexpr float GALAXY_VIEW_DISTANCE = 10000.0f;  // Parsecs
constexpr float COSMIC_WEB_DISTANCE = 1000000.0f; // Parsecs
} // namespace Galaxy
} // namespace CameraConstants