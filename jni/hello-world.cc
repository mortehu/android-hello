#include <algorithm>
#include <vector>

#include <jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "utils/log.h"

namespace {

static const char kVertexShader[] =
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char kFragmentShader[] =
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(0.0, 0.25, 0.5, 1.0);\n"
    "}\n";

GLuint program;
GLuint gvPositionHandle;
GLuint indexBuffer;
GLuint vertexBuffer;

bool hold = false;
float gray;

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
  UTILS_REQUIRE(compiled == GL_TRUE);

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
  program = createProgram(kVertexShader, kFragmentShader);

  UTILS_GL_CHECK(gvPositionHandle = glGetAttribLocation(program, "vPosition"));
  UTILS_GL_CHECK(glViewport(0, 0, width, height));
  UTILS_GL_CHECK(glGenBuffers(1, &indexBuffer));
  UTILS_GL_CHECK(glGenBuffers(1, &vertexBuffer));
}

const GLshort gTriangleIndices[] = {0, 1, 2, 3, 4, 5};
const GLfloat gTriangleVertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};

void drawFrame() {
  if (!hold) gray = std::max(0.0f, gray - 0.08f);

  UTILS_GL_CHECK(glClearColor(gray * 0.5, gray, gray, 1.0f));
  UTILS_GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  UTILS_GL_CHECK(glUseProgram(program));

  UTILS_GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
  UTILS_GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(gTriangleVertices),
                              gTriangleVertices, GL_STREAM_DRAW));

  UTILS_GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
  UTILS_GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gTriangleIndices),
                              gTriangleIndices, GL_STREAM_DRAW));

  UTILS_GL_CHECK(glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE,
                                       0, arrayOffset(0)));
  UTILS_GL_CHECK(glEnableVertexAttribArray(gvPositionHandle));

  UTILS_GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr));
}

}  // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_surfaceCreated(JNIEnv* env,
                                                      jobject obj) {}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_surfaceChanged(JNIEnv* env, jobject obj,
                                                      jint width, jint height) {
  try {
    surfaceChanged(width, height);
  } catch (std::runtime_error& e) {
    error("Runtime error: %s", e.what());
  }
}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_drawFrame(JNIEnv* env, jobject obj) {
  try {
    drawFrame();
  } catch (std::runtime_error& e) {
    error("Runtime error: %s", e.what());
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
