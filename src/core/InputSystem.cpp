#include "InputSystem.hpp"
#include <iostream>

InputSystem::InputSystem(GLFWwindow *window)
    : _window(window), _mouseCaptured(false), _firstMouse(true),
      _lastMouseX(0.0), _lastMouseY(0.0), _mouseDelta(0.0f),
      _scrollDelta(0.0f) {

  glfwSetWindowUserPointer(window, this);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glfwSetKeyCallback(window, keyCallback);

  // Default bindings
  bindKey(GLFW_KEY_W, InputAction::MoveForward, 1.0f);
  bindKey(GLFW_KEY_S, InputAction::MoveForward, -1.0f);
  bindKey(GLFW_KEY_D, InputAction::MoveRight, 1.0f);
  bindKey(GLFW_KEY_A, InputAction::MoveRight, -1.0f);
  bindKey(GLFW_KEY_E, InputAction::MoveUp, 1.0f);
  bindKey(GLFW_KEY_Q, InputAction::MoveUp, -1.0f);
  bindKey(GLFW_KEY_LEFT_SHIFT, InputAction::SpeedBoost, 1.0f);
  bindKey(GLFW_KEY_TAB, InputAction::ToggleMouseCapture, 1.0f);
  bindKey(GLFW_KEY_ESCAPE, InputAction::Exit, 1.0f);
}

InputSystem::~InputSystem() {
  if (_mouseCaptured) {
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

void InputSystem::update() {
  // Clear per-frame state
  _buttonPressed.clear();
  _mouseDelta = glm::vec2(0.0f);
  _scrollDelta = 0.0f;

  // Update axis values from key states
  _axisValues.clear();
  for (const auto &[key, binding] : _keyBindings) {
    if (glfwGetKey(_window, key) == GLFW_PRESS) {
      _axisValues[binding.action] += binding.scale;
    }
  }
}

float InputSystem::getAxis(InputAction action) const {
  auto it = _axisValues.find(action);
  return it != _axisValues.end() ? it->second : 0.0f;
}

bool InputSystem::getButton(InputAction action) const {
  auto it = _buttonStates.find(action);
  return it != _buttonStates.end() ? it->second : false;
}

bool InputSystem::getButtonDown(InputAction action) const {
  auto it = _buttonPressed.find(action);
  return it != _buttonPressed.end() ? it->second : false;
}

glm::vec2 InputSystem::getMouseDelta() const { return _mouseDelta; }

float InputSystem::getScrollDelta() const { return _scrollDelta; }

void InputSystem::bindKey(int key, InputAction action, float scale) {
  _keyBindings[key] = {key, action, scale};
}

void InputSystem::enableMouseCapture(bool capture) {
  _mouseCaptured = capture;
  if (capture) {
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _firstMouse = true;
    std::cout << "Mouse captured\n";
  } else {
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    std::cout << "Mouse released\n";
  }
}

void InputSystem::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  auto input = static_cast<InputSystem *>(glfwGetWindowUserPointer(window));
  input->handleMouseMove(xpos, ypos);
}

void InputSystem::scrollCallback(GLFWwindow *window, double xoffset,
                                 double yoffset) {
  auto input = static_cast<InputSystem *>(glfwGetWindowUserPointer(window));
  input->handleScroll(xoffset, yoffset);
}

void InputSystem::keyCallback(GLFWwindow *window, int key, int scancode,
                              int action, int mods) {
  auto input = static_cast<InputSystem *>(glfwGetWindowUserPointer(window));
  input->handleKey(key, action);
}

void InputSystem::handleMouseMove(double xpos, double ypos) {
  if (!_mouseCaptured)
    return;

  if (_firstMouse) {
    _lastMouseX = xpos;
    _lastMouseY = ypos;
    _firstMouse = false;
    return;
  }

  _mouseDelta.x = static_cast<float>(xpos - _lastMouseX);
  _mouseDelta.y = static_cast<float>(_lastMouseY - ypos); // Inverted Y

  _lastMouseX = xpos;
  _lastMouseY = ypos;
}

void InputSystem::handleScroll(double xoffset, double yoffset) {
  _scrollDelta = static_cast<float>(yoffset);
}

void InputSystem::handleKey(int key, int action) {
  auto it = _keyBindings.find(key);
  if (it != _keyBindings.end()) {
    InputAction inputAction = it->second.action;

    if (action == GLFW_PRESS) {
      _buttonStates[inputAction] = true;
      _buttonPressed[inputAction] = true;
    } else if (action == GLFW_RELEASE) {
      _buttonStates[inputAction] = false;
    }
  }
}