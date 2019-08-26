#ifndef FOO_H_
#define FOO_H_

#include "../common.hpp"

float const PI = 3.141592653589793;
const float PI_180 = 0.017453292519943295; // PI/180.0
float const EPSILON = 0.000001;
float const degree = PI_180;

float toRadian(float a);

#define X 0
#define Y 1
#define Z 2
#define W 3

// sizes
int const VEC_SIZE_3 = 3;
int const VEC_SIZE_4 = 4;
int const MAT_SIZE_4 = 4 * 4;

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(a, min, max) MIN(max, MAX(min, a))
int CLAMP255(int a);
float CLAMP01(float a);
float LERP(float alpha, float from, float to);

extern float mathTmp[VEC_SIZE_3 + VEC_SIZE_4 + MAT_SIZE_4];
#define vec3Tmp (float *)mathTmp
#define vec4Tmp (float *)(mathTmp + VEC_SIZE_3)
#define mat4Tmp (float *)(mathTmp + VEC_SIZE_3 + VEC_SIZE_4)

float *mat4_free(float *a);
float *mat4_create();
float *mat4_clone(float *a);
float *mat4_copy(float *out, float *a);
float *mat4_fromValues(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
float *mat4_set(float *out, float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
float *mat4_identity(float *out);
float *mat4_transpose(float *out, float *a);
float *mat4_invert(float *out, float *a);
float *mat4_adjoint(float *out, float *a);
float mat4_determinant(float *a);
float *mat4_multiply(float *out, float *a, float *b);
float *mat4_translate(float *out, float *a, float *v);
float *mat4_scale(float *out, float *a, float *v);
float *mat4_rotate(float *out, float *a, float rad, float *axis);
float *mat4_rotateX(float *out, float *a, float rad);
float *mat4_rotateY(float *out, float *a, float rad);
float *mat4_rotateZ(float *out, float *a, float rad);
float *mat4_fromTranslation(float *out, float *v);
float *mat4_fromScaling(float *out, float *v);
float *mat4_fromRotation(float *out, float rad, float *axis);
float *mat4_fromXRotation(float *out, float rad);
float *mat4_fromYRotation(float *out, float rad);
float *mat4_fromZRotation(float *out, float rad);
float *mat4_fromRotationTranslation(float *out, float *q, float *v);
float *mat4_getTranslation(float *out, float *mat);
float *mat4_getScaling(float *out, float *mat);
float *mat4_getRotation(float *out, float *mat);
float *mat4_fromRotationTranslationScale(float *out, float *q, float *v, float *s);
float *mat4_fromRotationTranslationScaleOrigin(float *out, float *q, float *v, float *s, float *o);
float *mat4_fromQuat(float *out, float *q);
float *mat4_frustum(float *out, float left, float right, float bottom, float top, float near, float far);
float *mat4_perspective(float *out, float fovy, float aspect, float near, float far);
float *mat4_perspectiveFromFieldOfView(float *out, float upDegrees, float downDegrees, float leftDegrees, float rightDegrees, float near, float far);
float *mat4_ortho(float *out, float left, float right, float bottom, float top, float near, float far);
float *mat4_lookAt(float *out, float *eye, float *center, float *up);
float *mat4_targetTo(float *out, float *eye, float *target, float *up);
float mat4_frob(float *a);
float *mat4_add(float *out, float *a, float *b);
float *mat4_subtract(float *out, float *a, float *b);
float *mat4_multiplyScalar(float *out, float *a, float b);
float *mat4_multiplyScalarAndAdd(float *out, float *a, float *b, float scale);
int mat4_exactEquals(float *a, float *b);
int mat4_equals(float *a, float *b);
float *mat4_mul(float *out, float *a, float *b);
float *mat4_sub(float *out, float *a, float *b);

float *vec3_free(float *a);
float *vec3_create();
float *vec3_clone(float *a);
float vec3_length(float *a);
float *vec3_fromValues(float x, float y, float z);
float *vec3_copy(float *out, float *a);
float *vec3_set(float *out, float x, float y, float z);
float *vec3_add(float *out, float *a, float *b);
float *vec3_subtract(float *out, float *a, float *b);
float *vec3_multiply(float *out, float *a, float *b);
float *vec3_divide(float *out, float *a, float *b);
float *vec3_ceil(float *out, float *a);
float *vec3_floor(float *out, float *a);
float *vec3_min(float *out, float *a, float *b);
float *vec3_max(float *out, float *a, float *b);
float *vec3_round(float *out, float *a);
float *vec3_scale(float *out, float *a, float b);
float *vec3_scaleAndAdd(float *out, float *a, float *b, float scale);
float vec3_distance(float *a, float *b);
float vec3_squaredDistance(float *a, float *b);
float vec3_squaredLength(float *a);
float *vec3_negate(float *out, float *a);
float *vec3_inverse(float *out, float *a);
float *vec3_normalize(float *out, float *a);
float vec3_dot(float *a, float *b);
float *vec3_cross(float *out, float *a, float *b);
float *vec3_lerp(float *out, float *a, float *b, float t);
float *vec3_hermite(float *out, float *a, float *b, float *c, float *d, float t);
float *vec3_bezier(float *out, float *a, float *b, float *c, float *d, float t);
float *vec3_random(float *out, float scale);
float *vec3_transformMat4(float *out, float *a, float *m);
float *vec3_transformMat3(float *out, float *a, float *m);
float *vec3_transformQuat(float *out, float *a, float *q);
float *vec3_rotateX(float *out, float *a, float *b, float c);
float *vec3_rotateY(float *out, float *a, float *b, float c);
float *vec3_rotateZ(float *out, float *a, float *b, float c);
float vec3_angle(float *a, float *b);
int vec3_exactEquals(float *a, float *b);
int vec3_equals(float *a, float *b);
float *vec3_sub(float *out, float *a, float *b);
float *vec3_mul(float *out, float *a, float *b);
float *vec3_div(float *out, float *a, float *b);
float vec3_dist(float *a, float *b);
float vec3_sqrDist(float *a, float *b);
float vec3_len(float *a);
float vec3_sqrLen(float *a);

float *vec4_free(float *a);
float *vec4_create();
float *vec4_clone(float *a);
float *vec4_fromValues(float x, float y, float z, float w);
float *vec4_copy(float *out, float *a);
float *vec4_set(float *out, float x, float y, float z, float w);
float *vec4_add(float *out, float *a, float *b);
float *vec4_subtract(float *out, float *a, float *b);
float *vec4_multiply(float *out, float *a, float *b);
float *vec4_divide(float *out, float *a, float *b);
float *vec4_ceil(float *out, float *a);
float *vec4_floor(float *out, float *a);
float *vec4_min(float *out, float *a, float *b);
float *vec4_max(float *out, float *a, float *b);
float *vec4_round(float *out, float *a);
float *vec4_scale(float *out, float *a, float b);
float *vec4_scaleAndAdd(float *out, float *a, float *b, float scale);
float vec4_distance(float *a, float *b);
float vec4_squaredDistance(float *a, float *b);
float vec4_length(float *a);
float vec4_squaredLength(float *a);
float *vec4_negate(float *out, float *a);
float *vec4_inverse(float *out, float *a);
float *vec4_normalize(float *out, float *a);
float vec4_dot(float *a, float *b);
float *vec4_lerp(float *out, float *a, float *b, float t);
float *vec4_transformMat4(float *out, float *a, float *m);
float *vec4_transformQuat(float *out, float *a, float *q);
int vec4_exactEquals(float *a, float *b);
int vec4_equals(float *a, float *b);
float *vec4_sub(float *out, float *a, float *b);
float *vec4_mul(float *out, float *a, float *b);
float *vec4_div(float *out, float *a, float *b);
float vec4_dist(float *a, float *b);
float vec4_sqrDist(float *a, float *b);
float vec4_len(float *a);
float vec4_sqrLen(float *a);

#endif