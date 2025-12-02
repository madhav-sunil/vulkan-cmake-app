#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>

// Input action types
enum class InputAction {
  MoveForward,
  MoveRight,
  MoveUp,
  LookYaw,
  LookPitch,
  Zoom,
  SpeedBoost,
  ToggleMouseCapture,
  Exit
};

// Input binding (maps keys to actions)
struct InputBinding {
  int key;
  InputAction action;
  float scale = 1.0f; // For axis mapping (e.g., -1 for backward)
};

class InputSystem {
public:
  InputSystem(GLFWwindow *window);
  ~InputSystem();

  // Query input state (call per frame)
  float getAxis(InputAction action) const;
  bool getButton(InputAction action) const;
  bool getButtonDown(InputAction action) const; // Pressed this frame
  glm::vec2 getMouseDelta() const;
  float getScrollDelta() const;

  // Configuration
  void bindKey(int key, InputAction action, float scale = 1.0f);
  void enableMouseCapture(bool capture);
  bool isMouseCaptured() const { return _mouseCaptured; }

  // Update (call once per frame BEFORE processing)
  void update();

private:
  // GLFW callbacks
  static void mouseCallback(GLFWwindow *window, double xpos, double ypos);
  static void scrollCallback(GLFWwindow *window, double xoffset,
                             double yoffset);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

  void handleMouseMove(double xpos, double ypos);
  void handleScroll(double xoffset, double yoffset);
  void handleKey(int key, int action);

private:
  GLFWwindow *_window;

  // Input state
  std::unordered_map<InputAction, float> _axisValues;
  std::unordered_map<InputAction, bool> _buttonStates;
  std::unordered_map<InputAction, bool> _buttonPressed; // This frame
  std::unordered_map<int, InputBinding> _keyBindings;

  // Mouse state
  bool _mouseCaptured;
  bool _firstMouse;
  double _lastMouseX, _lastMouseY;
  glm::vec2 _mouseDelta;
  float _scrollDelta;
};