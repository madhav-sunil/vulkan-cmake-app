#include "Camera.hpp"
#include "CameraConstants.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float aspect, const glm::vec3 &position)
    : _position(position), _worldUp(CameraConstants::WORLD_UP),
      _yaw(CameraConstants::Defaults::FREE_CAMERA_YAW),
      _pitch(CameraConstants::Defaults::FREE_CAMERA_PITCH),
      _fov(CameraConstants::Defaults::FREE_CAMERA_FOV), _aspect(aspect),
      _nearPlane(CameraConstants::Defaults::NEAR_PLANE),
      _farPlane(CameraConstants::Defaults::FAR_PLANE),
      _moveSpeed(CameraConstants::Defaults::FREE_CAMERA_MOVE_SPEED),
      _mouseSensitivity(CameraConstants::Defaults::FREE_CAMERA_SENSITIVITY),
      _zoomSpeed(CameraConstants::Defaults::FREE_CAMERA_ZOOM_SPEED) {
  updateVectors();
}

void Camera::update(float deltaTime) {
  // Update logic if needed
}

void Camera::updateAspect(float aspect) { _aspect = aspect; }

void Camera::moveForward(float amount) {
  _position += _front * amount * _moveSpeed;
}

void Camera::moveRight(float amount) {
  _position += _right * amount * _moveSpeed;
}

void Camera::moveUp(float amount) {
  _position +=
      _worldUp * amount * _moveSpeed; // Use WORLD_UP for vertical movement
}

void Camera::rotate(float yawDelta, float pitchDelta) {
  _yaw += yawDelta * _mouseSensitivity;
  _pitch += pitchDelta * _mouseSensitivity;

  // Constrain pitch using constants
  _pitch = std::clamp(_pitch, CameraConstants::Defaults::MIN_PITCH,
                      CameraConstants::Defaults::MAX_PITCH);

  updateVectors();
}

void Camera::zoom(float amount) {
  _fov -= amount * _zoomSpeed;
  _fov = std::clamp(_fov, CameraConstants::Defaults::MIN_FOV,
                    CameraConstants::Defaults::MAX_FOV);
}

void Camera::setFov(float fov) {
  _fov = std::clamp(fov, CameraConstants::Defaults::MIN_FOV,
                    CameraConstants::Defaults::MAX_FOV);
}

void Camera::setRotation(float yaw, float pitch) {
  _yaw = yaw;
  _pitch = std::clamp(pitch, CameraConstants::Defaults::MIN_PITCH,
                      CameraConstants::Defaults::MAX_PITCH);
  updateVectors();
}

auto Camera::getViewMatrix() const -> glm::mat4 {
  return glm::lookAt(_position, CameraConstants::ORIGIN, _up);
}

auto Camera::getProjectionMatrix() const -> glm::mat4 {
  glm::mat4 proj =
      glm::perspective(glm::radians(_fov), _aspect, _nearPlane, _farPlane);
  // Vulkan clip space correction
  proj[1][1] *= -1;
  return proj;
}

auto Camera::getViewProjectionMatrix() const -> glm::mat4 {
  return getProjectionMatrix() * getViewMatrix();
}

void Camera::updateVectors() {
  // Calculate new front vector from Euler angles
  glm::vec3 front;
  front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
  front.y = sin(glm::radians(_pitch));
  front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
  _front = glm::normalize(front);

  // Recalculate right and up vectors using world up constant
  _right = glm::normalize(glm::cross(_front, _worldUp));
  _up = glm::normalize(glm::cross(_right, _front));
}
