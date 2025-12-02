#include "CameraController.hpp"
#include "CameraConstants.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

using namespace CameraConstants;

// ============ Free Camera ============

FreeCameraController::FreeCameraController()
    : _moveSpeed(Defaults::FREE_CAMERA_MOVE_SPEED),
      _lookSensitivity(Defaults::FREE_CAMERA_SENSITIVITY),
      _zoomSpeed(Defaults::FREE_CAMERA_ZOOM_SPEED),
      _sprintMultiplier(Defaults::FREE_CAMERA_SPRINT_MULT) {}

void FreeCameraController::update(Camera &camera, const InputSystem &input,
                                  float deltaTime) {
  // Movement with sprint
  float speed = _moveSpeed;
  if (input.getButton(InputAction::SpeedBoost)) {
    speed *= _sprintMultiplier;
  }

  float forward = input.getAxis(InputAction::MoveForward) * speed * deltaTime;
  float right = input.getAxis(InputAction::MoveRight) * speed * deltaTime;
  float up = input.getAxis(InputAction::MoveUp) * speed * deltaTime;

  camera.moveForward(forward);
  camera.moveRight(right);
  camera.moveUp(up);

  // Look
  if (input.isMouseCaptured()) {
    glm::vec2 mouseDelta = input.getMouseDelta();
    camera.rotate(mouseDelta.x * _lookSensitivity,
                  mouseDelta.y * _lookSensitivity);
  }

  // Zoom
  float scroll = input.getScrollDelta();
  if (scroll != 0.0f) {
    camera.zoom(scroll * _zoomSpeed);
  }
}

// ============ Orbit Camera ============

OrbitCameraController::OrbitCameraController(const glm::vec3 &target)
    : _target(target), _distance(Defaults::ORBIT_CAMERA_DISTANCE),
      _orbitSpeed(Defaults::ORBIT_CAMERA_SPEED),
      _theta(Defaults::ORBIT_CAMERA_THETA), _phi(Defaults::ORBIT_CAMERA_PHI) {}

void OrbitCameraController::update(Camera &camera, const InputSystem &input,
                                   float deltaTime) {
  // Orbit with mouse
  if (input.isMouseCaptured()) {
    glm::vec2 mouseDelta = input.getMouseDelta();
    _theta += mouseDelta.x * _orbitSpeed;
    _phi -= mouseDelta.y * _orbitSpeed;

    // Clamp pitch using constants
    _phi = std::clamp(_phi, Defaults::MIN_PITCH, Defaults::MAX_PITCH);
  }

  // Zoom with scroll (constrain distance)
  float scroll = input.getScrollDelta();
  if (scroll != 0.0f) {
    _distance -= scroll * 2.0f;
    _distance = std::clamp(_distance, Defaults::MIN_ORBIT_DIST,
                           Defaults::MAX_ORBIT_DIST);
  }

  // Pan target with WASD
  float panSpeed = Defaults::ORBIT_CAMERA_PAN_SPEED * deltaTime;
  glm::vec3 right = camera.getRight();
  glm::vec3 forward =
      glm::normalize(glm::vec3(camera.getFront().x, 0.0f, camera.getFront().z));

  _target += forward * input.getAxis(InputAction::MoveForward) * panSpeed;
  _target += right * input.getAxis(InputAction::MoveRight) * panSpeed;

  // Calculate camera position using spherical coordinates
  float thetaRad = glm::radians(_theta);
  float phiRad = glm::radians(_phi);

  glm::vec3 offset;
  offset.x = _distance * cos(phiRad) * cos(thetaRad);
  offset.y = _distance * sin(phiRad);
  offset.z = _distance * cos(phiRad) * sin(thetaRad);

  camera.setPosition(_target + offset);

  // Look at target
  glm::vec3 direction = glm::normalize(_target - camera.getPosition());
  float yaw = glm::degrees(atan2(direction.z, direction.x));
  float pitch = glm::degrees(asin(direction.y));
  camera.setRotation(yaw, pitch);
}