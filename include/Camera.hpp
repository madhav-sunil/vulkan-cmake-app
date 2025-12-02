#pragma once
#include "CameraConstants.hpp"
#include <glm/glm.hpp>

/*
@brief A simple free-moving camera class for 3D applications.
*/
class Camera {
public:
  Camera(float aspect, const glm::vec3 &position =
                           CameraConstants::Defaults::FREE_CAMERA_POSITION);

  // Update camera state
  void update(float deltaTime);
  void updateAspect(float aspect);

  // Movement controls
  void moveForward(float amount);
  void moveRight(float amount);
  void moveUp(float amount);
  void rotate(float yaw, float pitch); // In radians
  void zoom(float amount);

  auto getViewMatrix() const -> glm::mat4;
  auto getProjectionMatrix() const -> glm::mat4;
  auto getViewProjectionMatrix() const -> glm::mat4;
  auto getPosition() const -> const glm::vec3 & { return _position; }
  auto getFront() const -> const glm::vec3 & { return _front; }
  auto getUp() const -> const glm::vec3 & { return _up; }
  auto getRight() const -> const glm::vec3 & { return _right; }
  auto getFov() const -> float { return _fov; }

  void setPosition(const glm::vec3 &position) { _position = position; }
  void setSpeed(float speed) { _moveSpeed = speed; }
  void setSensitivity(float sensitivity) { _mouseSensitivity = sensitivity; }
  void setFov(float fov);
  void setRotation(float yaw, float pitch);

private:
  void updateVectors();

  // Camera vectors
  glm::vec3 _position;
  glm::vec3 _front;
  glm::vec3 _up;
  glm::vec3 _right;
  glm::vec3 _worldUp;

  // Euler angles (in degrees)
  float _yaw;
  float _pitch;

  // Camera parameters
  float _fov;    // Field of view
  float _aspect; // Aspect ratio
  float _nearPlane;
  float _farPlane;

  // Movement settings
  float _moveSpeed;
  float _mouseSensitivity;
  float _zoomSpeed;
};