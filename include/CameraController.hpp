#pragma once
#include "Camera.hpp"
#include "InputSystem.hpp"

// Base camera controller (strategy interface)
class CameraController {
public:
  virtual ~CameraController() = default;
  virtual void update(Camera &camera, const InputSystem &input,
                      float deltaTime) = 0;
};

// Free-fly camera (FPS-style, like your current implementation)
class FreeCameraController : public CameraController {
public:
  FreeCameraController();

  void update(Camera &camera, const InputSystem &input,
              float deltaTime) override;

  void setMoveSpeed(float speed) { _moveSpeed = speed; }
  void setLookSensitivity(float sensitivity) { _lookSensitivity = sensitivity; }
  void setZoomSpeed(float speed) { _zoomSpeed = speed; }

private:
  float _moveSpeed;
  float _lookSensitivity;
  float _zoomSpeed;
  float _sprintMultiplier;
};

// Orbit camera (rotates around a target point, like Blender viewport)
class OrbitCameraController : public CameraController {
public:
  OrbitCameraController(const glm::vec3 &target = glm::vec3(0.0f));

  void update(Camera &camera, const InputSystem &input,
              float deltaTime) override;

  void setTarget(const glm::vec3 &target) { _target = target; }
  void setDistance(float distance) { _distance = distance; }
  void setOrbitSpeed(float speed) { _orbitSpeed = speed; }

private:
  glm::vec3 _target;
  float _distance;
  float _orbitSpeed;
  float _theta; // Azimuth angle
  float _phi;   // Elevation angle
};