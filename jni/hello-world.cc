#include <algorithm>
#include <vector>

#include <jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "geometry/sphere.h"
#include "geometry/vector.h"
#include "utils/log.h"

namespace {

static const char kVertexShader[] =
    "attribute vec3 attr_VertexPosition;\n"
    "attribute vec3 attr_VertexColor;\n"
    "varying vec3 var_Color;\n"
    "uniform mat4 uniform_ModelViewProjection;\n"
    "\n"
    "void main(void) {\n"
    "  gl_Position = (uniform_ModelViewProjection\n"
    "                 * vec4(attr_VertexPosition, 1.0));\n"
    "  var_Color = attr_VertexColor;\n"
    "}\n";

static const char kFragmentShader[] =
    "precision mediump float;\n"
    "varying vec3 var_Color;\n"
    "void main(void) {\n"
    "  gl_FragColor = vec4(var_Color, 1.0);\n"
    "}\n";

int window_width, window_height;

GLuint vertexBuffer;
GLuint indexBuffer;

GLuint program;

// Shader variables.
GLint gaVertexPosition;
GLint gaVertexColor;
GLint guModelViewProjection;

struct vertex {
  vec3 position;
  std::array<uint8_t, 3> color;
};

std::vector<uint16_t> sphere_indices;
std::vector<vertex> sphere_vertices;

bool hold = false;
float gray;

bool done = false;

uint64_t frame_counter;

// Converts an array offset in bytes to void*, as required by
// glVertexAttribPointer.
constexpr void* arrayOffset(uintptr_t offset) {
  union union_type {
    constexpr union_type(uintptr_t i) : i_{i} {}
    uintptr_t i_;
    void* v_;
  } x{offset};
  return x.v_;
}

GLuint loadShader(GLenum shaderType, const char* pSource) {
  GLuint shader;
  UTILS_REQUIRE(shader = glCreateShader(shaderType));

  glShaderSource(shader, 1, &pSource, NULL);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled != GL_TRUE) {
    GLchar log[1024];
    GLsizei log_length;
    glGetShaderInfoLog(shader, sizeof(log), &log_length, log);
    throw std::runtime_error{std::string(log, log_length)};
  }

  return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
  const auto vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
  const auto pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);

  GLuint program;
  UTILS_REQUIRE(program = glCreateProgram());

  UTILS_GL_CHECK(glAttachShader(program, vertexShader));
  UTILS_GL_CHECK(glAttachShader(program, pixelShader));

  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  UTILS_REQUIRE(linkStatus == GL_TRUE);

  return program;
}

void surfaceChanged(int width, int height) {
  if (sphere_indices.empty()) {
    std::vector<vec3> positions;
    sphere(1, &positions, &sphere_indices);

    std::mt19937_64 rng;
    std::uniform_int_distribution<uint8_t> dist;
    for (const auto& position : positions) {
      vertex v;
      v.position = position * 10.0f;
      v.color = std::array<uint8_t, 3>{{dist(rng), dist(rng), dist(rng)}};
      sphere_vertices.emplace_back(v);
    }
  }

  program = createProgram(kVertexShader, kFragmentShader);

  UTILS_GL_CHECK(gaVertexPosition =
                     glGetAttribLocation(program, "attr_VertexPosition"));
  UTILS_GL_CHECK(gaVertexColor =
                     glGetAttribLocation(program, "attr_VertexColor"));
  UTILS_GL_CHECK(guModelViewProjection = glGetUniformLocation(
                     program, "uniform_ModelViewProjection"));

  UTILS_GL_CHECK(glViewport(0, 0, width, height));
  UTILS_GL_CHECK(glGenBuffers(1, &indexBuffer));
  UTILS_GL_CHECK(glGenBuffers(1, &vertexBuffer));

  UTILS_GL_CHECK(glEnable(GL_CULL_FACE));
  UTILS_GL_CHECK(glEnable(GL_DEPTH_TEST));

  window_width = width;
  window_height = height;
}

void drawFrame() {
  if (!hold) gray = std::max(0.0f, gray - 0.08f);

  UTILS_GL_CHECK(glClearColor(gray * 0.5, gray, gray, 1.0f));
  UTILS_GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  UTILS_GL_CHECK(glUseProgram(program));

  const auto aspect_ratio = static_cast<float>(window_width) / window_height;
  const auto projection = mat4x4::projection(1.0f, M_PI / 8.0f, aspect_ratio);
  const auto camera_angle = (frame_counter % 180) * 2 * M_PI / 180;
  const auto camera =
      mat4x4::from_quat(vec4::rotation(0.0f, 1.0f, 0.0f, camera_angle)) *
      mat4x4::translation(0.0f, 0.0f, 50.0f);
  const auto camera_projection = projection * camera.invert();

  UTILS_GL_CHECK(glUniformMatrix4fv(guModelViewProjection, 1, GL_FALSE,
                                    &camera_projection.m[0][0]));

  UTILS_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
  UTILS_GL_CHECK(glBufferData(
      GL_ARRAY_BUFFER, sizeof(sphere_vertices[0]) * sphere_vertices.size(),
      sphere_vertices.data(), GL_STREAM_DRAW));

  UTILS_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
  UTILS_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                              sizeof(sphere_indices[0]) * sphere_indices.size(),
                              sphere_indices.data(), GL_STREAM_DRAW));

  UTILS_GL_CHECK(glVertexAttribPointer(
      gaVertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
      arrayOffset(offsetof(vertex, position))));
  UTILS_GL_CHECK(glVertexAttribPointer(gaVertexColor, 3, GL_UNSIGNED_BYTE,
                                       GL_TRUE, sizeof(vertex),
                                       arrayOffset(offsetof(vertex, color))));
  UTILS_GL_CHECK(glEnableVertexAttribArray(gaVertexColor));
  UTILS_GL_CHECK(glEnableVertexAttribArray(gaVertexPosition));

  UTILS_GL_CHECK(glDrawElements(GL_TRIANGLES, sphere_indices.size(),
                                GL_UNSIGNED_SHORT, nullptr));

  ++frame_counter;
}

}  // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_surfaceCreated(JNIEnv* env,
                                                      jobject obj) {}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_surfaceChanged(JNIEnv* env, jobject obj,
                                                      jint width, jint height) {
  done = false;
  try {
    surfaceChanged(width, height);
  } catch (std::runtime_error& e) {
    error("Runtime error: %s", e.what());
    done = true;
  }
}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_drawFrame(JNIEnv* env, jobject obj) {
  if (done) return;
  try {
    drawFrame();
  } catch (std::runtime_error& e) {
    error("Runtime error: %s", e.what());
    done = true;
  }
}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_touchEvent(JNIEnv* env, jobject obj,
                                                  float x, float y, int state) {
  switch (state) {
    case 0:
      gray = 1.0f;
      hold = true;
      break;

    case 1:
      break;

    case 2:
      hold = false;
      break;
  }
}
