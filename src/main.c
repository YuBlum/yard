#include <assert.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "include/core.h"

#define WINDOW_W 640
#define WINDOW_H 480

int
main(void) {
  assert(glfwInit());
  log_info("glfw initialized");
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "jarpg", 0, 0);
  assert(window);
  glfwMakeContextCurrent(window);
  log_infof("window created with %d %d dimensions", WINDOW_W, WINDOW_H);
  const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  if (vidmode) {
    glfwSetWindowPos(window, vidmode->width / 2 - WINDOW_W / 2, vidmode->height / 2 - WINDOW_H / 2);
    log_info("window centered");
  } else {
    log_warn("couldn't center window");
  }
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}
