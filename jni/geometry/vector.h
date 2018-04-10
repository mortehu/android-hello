#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

class vec3 {
 public:
  vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  vec3() = default;
  vec3(const vec3&) = default;
  vec3& operator=(const vec3&) = default;

  bool operator==(const vec3& rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  vec3 normalize() const { return *this / magnitude(); }

  float magnitude() const { return std::sqrt(squared_magnitude()); }

  float squared_magnitude() const { return x * x + y * y + z * z; }

  vec3 operator*(float v) const { return vec3(x * v, y * v, z * v); }

  vec3 operator/(float v) const { return *this * (1.0f / v); }

  float operator*(const vec3& rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  vec3 operator+(const vec3& rhs) const {
    return vec3(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  vec3 operator-(const vec3& rhs) const {
    return vec3(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  vec3 cross(const vec3& rhs) const {
    return vec3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x);
  }

  vec3& operator+=(const vec3& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;

    return *this;
  }

  vec3& operator*=(float v) {
    x *= v;
    y *= v;
    z *= v;

    return *this;
  }

  float get(size_t idx) const {
    switch (idx) {
      case 0:
        return x;
      case 1:
        return y;
      case 2:
        return z;
      default:
        assert(!"invalid axis");
    }
  }

  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

class vec4 {
  public:
  vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

  vec4() = default;

  vec4(const vec4&) = default;
  vec4& operator=(const vec4&) = default;

  vec4 normalize() const { return *this / magnitude(); }

  float magnitude() const { return std::sqrt(squared_magnitude()); }

  float squared_magnitude() const { return x * x + y * y + z * z + w * w; }

  vec4 operator*(float v) const { return vec4(x * v, y * v, z * v, w * v); }

  vec4 operator/(float v) const { return *this * (1.0f / v); }

  float operator*(const vec4& rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
  }

  vec4 operator+(const vec4& rhs) const {
    return vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
  }

  vec4 operator-(const vec4& rhs) const {
    return vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
  }

  vec4& operator+=(const vec4& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;

    return *this;
  }

  vec4& operator*=(float v) {
    x *= v;
    y *= v;
    z *= v;
    w *= v;

    return *this;
  }

  // Returns a rotation quaternion.
  static vec4 rotation(float x, float y, float z, float angle) {
    vec4 result;

    float s, c;

#if HAVE_SINCOSF
    sincosf(angle * 0.5f, &s, &c);
#else
    s = sin(angle * 0.5f);
    c = cos(angle * 0.5f);
#endif

    result.x = s * x;
    result.y = s * y;
    result.z = s * z;
    result.w = c;

    return result;
  }

  float& operator[](size_t idx) { return a[idx]; }
  const float& operator[](size_t idx) const { return a[idx]; }

  union {
    struct {
      float x, y, z, w;
    };
    float a[4];
  };
};

union mat4x4 {
  public:
  mat4x4() = default;

  static mat4x4 from_quat(const vec4& quat) {
    const auto xx = 2.0f * quat.x * quat.x;
    const auto xy = 2.0f * quat.x * quat.y;
    const auto xz = 2.0f * quat.x * quat.z;
    const auto xw = 2.0f * quat.x * quat.w;
    const auto yy = 2.0f * quat.y * quat.y;
    const auto yz = 2.0f * quat.y * quat.z;
    const auto yw = 2.0f * quat.y * quat.w;
    const auto zz = 2.0f * quat.z * quat.z;
    const auto zw = 2.0f * quat.z * quat.w;

    mat4x4 result;

    result.m[0][0] = 1.0f - yy - zz;
    result.m[1][0] = xy - zw;
    result.m[2][0] = xz + yw;

    result.m[0][1] = xy + zw;
    result.m[1][1] = 1.0f - xx - zz;
    result.m[2][1] = yz - xw;

    result.m[0][2] = xz - yw;
    result.m[1][2] = yz + xw;
    result.m[2][2] = 1.0f - xx - yy;

    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;

    result.m[0][3] = 0.0f;
    result.m[1][3] = 0.0f;
    result.m[2][3] = 0.0f;

    result.m[3][3] = 1.0f;

    return result;
  }

  static mat4x4 projection(float znear, float fovx, float aspect) {
    mat4x4 result;

    const auto fovy = fovx / aspect;
    const auto max_x = znear * tan(fovx);
    const auto max_y = znear * tan(fovy);

    const auto min_x = -max_x;
    const auto min_y = -max_y;

    result.m[0][0] = 2.0f * znear / (max_x - min_x);
    result.m[1][0] = 0.0f;
    result.m[2][0] = (max_x + min_x) / (max_x - min_x);
    result.m[3][0] = 0.0f;
    result.m[0][1] = 0.0f;
    result.m[1][1] = 2.0f * znear / (max_y - min_y);
    result.m[2][1] = (max_y + min_y) / (max_y - min_y);
    result.m[3][1] = 0.0f;
    result.m[0][2] = 0.0f;
    result.m[1][2] = 0.0f;
    result.m[2][2] = -1.0f;
    result.m[3][2] = -2.0f * znear;
    result.m[0][3] = 0.0f;
    result.m[1][3] = 0.0f;
    result.m[2][3] = -1.0f;
    result.m[3][3] = 0.0f;

    return result;
  }

  static mat4x4 translation(float x, float y, float z) {
    mat4x4 result = identity();
    result.m[3][0] = x;
    result.m[3][1] = y;
    result.m[3][2] = z;
    return result;
  }

  static mat4x4 identity() {
    mat4x4 result;
    result.m[0] = vec4{1.0f, 0.0f, 0.0f, 0.0f};
    result.m[1] = vec4{0.0f, 1.0f, 0.0f, 0.0f};
    result.m[2] = vec4{0.0f, 0.0f, 1.0f, 0.0f};
    result.m[3] = vec4{0.0f, 0.0f, 0.0f, 1.0f};
    return result;
  }

  static mat4x4 zero() {
    mat4x4 result;
    result.m[0] = vec4{0.0f, 0.0f, 0.0f, 0.0f};
    result.m[1] = vec4{0.0f, 0.0f, 0.0f, 0.0f};
    result.m[2] = vec4{0.0f, 0.0f, 0.0f, 0.0f};
    result.m[3] = vec4{0.0f, 0.0f, 0.0f, 0.0f};
    return result;
  }

  vec4 row(size_t i) const { return {m[0][i], m[1][i], m[2][i], m[3][i]}; }

  const vec4& column(size_t i) const { return m[i]; }

  bool operator==(const mat4x4& rhs) const {
    for (size_t i = 0; i < 4; ++i) {
      for (size_t j = 0; j < 4; ++j) {
        if (m[i][j] != rhs.m[i][j]) return false;
      }
    }
    return true;
  }

  mat4x4 operator-(const mat4x4& rhs) const {
    mat4x4 result;

    for (size_t i = 0; i < 4; ++i) result.m[i] = m[i] - rhs.m[i];

    return result;
  }

  mat4x4 operator*(const mat4x4& rhs) const {
    mat4x4 result;

    for (size_t i = 0; i < 4; ++i) {
      const auto& r = row(i);
      for (size_t j = 0; j < 4; ++j) {
        result.m[j][i] = r * rhs.column(j);
      }
    }

    return result;
  }

  mat4x4& operator*=(const mat4x4& rhs) {
    *this = rhs * (*this);
    return *this;
  }

  vec4 operator*(const vec4& rhs) const {
    return {rhs[0] * m[0][0] + rhs[1] * m[1][0] + rhs[2] * m[2][0] +
                rhs[3] * m[3][0],
            rhs[0] * m[0][1] + rhs[1] * m[1][1] + rhs[2] * m[2][1] +
                rhs[3] * m[3][1],
            rhs[0] * m[0][2] + rhs[1] * m[1][2] + rhs[2] * m[2][2] +
                rhs[3] * m[3][2],
            rhs[0] * m[0][3] + rhs[1] * m[1][3] + rhs[2] * m[2][3] +
                rhs[3] * m[3][3]};
  }

  // Returns an inverted matrix, using Gauss-Jordan elimination.
  mat4x4 invert() const {
    mat4x4 src = *this;
    auto result = identity();

    for (size_t i = 0; i < 4; ++i) {
      // Select column with largest element as pivot column.
      auto max = std::fabs(src.m[i][i]);
      auto max_idx = i;

      for (size_t j = i + 1; j < 4; ++j) {
        if (std::fabs(src.m[j][i]) > max) {
          max = std::fabs(src.m[j][i]);
          max_idx = j;
        }
      }

      // Swap pivot column into the `i' position.
      if (max_idx != i) {
        std::swap(src.m[i], src.m[max_idx]);
        std::swap(result.m[i], result.m[max_idx]);
      }

      // Everything below and including the diagonal is zero, no chance of
      // inverting.
      if (src.m[i][i] > -1e-6 && src.m[i][i] < 1e-6) return zero();

      // Scale pivot column to make the `i' element equal 1.
      auto scale = 1.0f / src.m[i][i];

      src.m[i] *= scale;
      result.m[i] *= scale;

      /* Zero out the `i' row, except in the `i' position */
      for (size_t k = 1; k < 4; ++k) {
        const size_t j = (i + k) & 3;

        scale = -src.m[j][i];

        src.m[j] += src.m[i] * scale;
        result.m[j] += result.m[i] * scale;
      }
    }

    return result;
  }

  float norm(float order) {
    float result = 0.0f;
    for (size_t i = 0; i < 4; ++i) {
      for (size_t j = 0; j < 4; ++j) {
        result += std::pow(m[i][j], order);
      }
    }

    return std::pow(result, 1.0f / order);
  }

  mat4x4& translate(float x, float y, float z) {
    for (size_t i = 0; i < 4; ++i) {
      m[i][0] += m[i][3] * x;
      m[i][1] += m[i][3] * y;
      m[i][2] += m[i][3] * z;
    }
    return *this;
  }

  // Array of columns, as in OpenGL.
  vec4 m[4];

  // Array of scalar.
  float v[4][4];
};

inline std::ostream& operator<<(std::ostream& out, const vec4& m) {
  out << '[';
  for (size_t i = 0; i < 4; ++i) out << ' ' << m[i];
  out << " ]";
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const mat4x4& m) {
  std::array<std::string, 4 * 4> strings;
  std::array<size_t, 4> max_length{{0, 0, 0, 0}};

  for (size_t i = 0; i < 16; ++i) {
    std::ostringstream buf;
    buf << m.m[i / 4][i % 4];
    strings[i] = buf.str();
    max_length[i % 4] = std::max(strings[i].size(), max_length[i % 4]);
  }

  for (size_t i = 0; i < 4; ++i) {
    out << '[';
    for (size_t j = 0; j < 4; ++j) {
      out << std::setw(max_length[j] + 1) << strings[i * 4 + j];
    }
    out << " ]\n";
  }
  return out;
}
