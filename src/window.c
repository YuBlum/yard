#include <GLFW/glfw3.h>
#include "yard/core.h"
#include "yard/window.h"

struct window {
  GLFWwindow *handle;
  uint32_t width;
  uint32_t height;
};

static struct window window;

bool
window_make(uint32_t width, uint32_t height) {
  log_info("making window...");
  if (!glfwInit()) {
    log_error("couldn't initialize glfw");
    return false;
  }
  log_info("glfw initialized");
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window.handle = glfwCreateWindow(width, height, "Yet Another Roguelike-ish Dungeon", 0, 0);
  if (!window.handle) {
    log_error("couldn't create window handle");
    return false;
  }
  glfwMakeContextCurrent(window.handle);
  log_infof("window created with %d %d dimensions", width, height);
  const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  if (vidmode) {
    glfwSetWindowPos(window.handle, vidmode->width / 2 - width / 2, vidmode->height / 2 - height / 2);
    log_info("window centered");
  } else {
    log_warn("couldn't center window");
  }
  log_info("window creation complete!");
  return true;
}

bool
window_is_running() {
  glfwPollEvents();
  glfwSwapBuffers(window.handle);
  return !glfwWindowShouldClose(window.handle);
}
