#include "common.h"


/**
 * Frees a vec4
 */
float *vec4_free(float *a) {
  free(a);
  return NULL;
};

/**
 * Creates a new, empty vec4
 *
 * @returns {vec4} a new 3D vector
 */
float *vec4_create() {
  float *out = (float *)malloc(VEC_SIZE_4 * sizeof(*out));
  out[0] = 0;
  out[1] = 0;
  out[2] = 0;
  out[3] = 0;
  return out;
};

/**
 * Creates a new vec4 initialized with values from an existing vector
 *
 * @param {vec4} a vector to clone
 * @returns {vec4} a new 4D vector
 */
float *vec4_clone(float *a) {
  float *out = (float *)malloc(VEC_SIZE_4 * sizeof(*out));
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  out[3] = a[3];
  return out;
};

/**
 * Creates a new vec4 initialized with the given values
 *
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @param {Number} w W component
 * @returns {vec4} a new 4D vector
 */
float *vec4_fromValues(float x, float y, float z, float w) {
  float *out = (float *)malloc(VEC_SIZE_4 * sizeof(*out));
  out[0] = x;
  out[1] = y;
  out[2] = z;
  out[3] = w;
  return out;
};

/**
 * Copy the values from one vec4 to another
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the source vector
 * @returns {vec4} out
 */
float *vec4_copy(float *out, float *a) {
  out[0] = a[0];
  out[1] = a[1];
  out[2] = a[2];
  out[3] = a[3];
  return out;
};

/**
 * Set the components of a vec4 to the given values
 *
 * @param {vec4} out the receiving vector
 * @param {Number} x X component
 * @param {Number} y Y component
 * @param {Number} z Z component
 * @param {Number} w W component
 * @returns {vec4} out
 */
float *vec4_set(float *out, float x, float y, float z, float w) {
  out[0] = x;
  out[1] = y;
  out[2] = z;
  out[3] = w;
  return out;
};

/**
 * Adds two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {vec4} out
 */
float *vec4_add(float *out, float *a, float *b) {
  out[0] = a[0] + b[0];
  out[1] = a[1] + b[1];
  out[2] = a[2] + b[2];
  out[3] = a[3] + b[3];
  return out;
};

/**
 * Subtracts vector b from vector a
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {vec4} out
 */
float *vec4_subtract(float *out, float *a, float *b) {
  out[0] = a[0] - b[0];
  out[1] = a[1] - b[1];
  out[2] = a[2] - b[2];
  out[3] = a[3] - b[3];
  return out;
};

/**
 * Multiplies two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {vec4} out
 */
float *vec4_multiply(float *out, float *a, float *b) {
  out[0] = a[0] * b[0];
  out[1] = a[1] * b[1];
  out[2] = a[2] * b[2];
  out[3] = a[3] * b[3];
  return out;
};

/**
 * Divides two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {vec4} out
 */
float *vec4_divide(float *out, float *a, float *b) {
  out[0] = a[0] / b[0];
  out[1] = a[1] / b[1];
  out[2] = a[2] / b[2];
  out[3] = a[3] / b[3];
  return out;
};

/**
 * Math.ceil the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to ceil
 * @returns {vec4} out
 */
float *vec4_ceil(float *out, float *a) {
  out[0] = ceil(a[0]);
  out[1] = ceil(a[1]);
  out[2] = ceil(a[2]);
  out[3] = ceil(a[3]);
  return out;
};

/**
 * Math.floor the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to floor
 * @returns {vec4} out
 */
float *vec4_floor(float *out, float *a) {
  out[0] = floor(a[0]);
  out[1] = floor(a[1]);
  out[2] = floor(a[2]);
  out[3] = floor(a[3]);
  return out;
};

/**
 * Returns the minimum of two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {vec4} out
 */
float *vec4_min(float *out, float *a, float *b) {
  out[0] = MIN(a[0], b[0]);
  out[1] = MIN(a[1], b[1]);
  out[2] = MIN(a[2], b[2]);
  out[3] = MIN(a[3], b[3]);
  return out;
};

/**
 * Returns the maximum of two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {vec4} out
 */
float *vec4_max(float *out, float *a, float *b) {
  out[0] = MAX(a[0], b[0]);
  out[1] = MAX(a[1], b[1]);
  out[2] = MAX(a[2], b[2]);
  out[3] = MAX(a[3], b[3]);
  return out;
};

/**
 * Math.round the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to round
 * @returns {vec4} out
 */
float *vec4_round(float *out, float *a) {
  out[0] = round(a[0]);
  out[1] = round(a[1]);
  out[2] = round(a[2]);
  out[3] = round(a[3]);
  return out;
};

/**
 * Scales a vec4 by a scalar number
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the vector to scale
 * @param {Number} b amount to scale the vector by
 * @returns {vec4} out
 */
float *vec4_scale(float *out, float *a, float b) {
  out[0] = a[0] * b;
  out[1] = a[1] * b;
  out[2] = a[2] * b;
  out[3] = a[3] * b;
  return out;
};

/**
 * Adds two vec4's after scaling the second operand by a scalar value
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @param {Number} scale the amount to scale b by before adding
 * @returns {vec4} out
 */
float *vec4_scaleAndAdd(float *out, float *a, float *b, float scale) {
  out[0] = a[0] + (b[0] * scale);
  out[1] = a[1] + (b[1] * scale);
  out[2] = a[2] + (b[2] * scale);
  out[3] = a[3] + (b[3] * scale);
  return out;
};

/**
 * Calculates the euclidian distance between two vec4's
 *
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {Number} distance between a and b
 */
float vec4_distance(float *a, float *b) {
  float x = b[0] - a[0];
  float y = b[1] - a[1];
  float z = b[2] - a[2];
  float w = b[3] - a[3];
  return sqrt(x * x + y * y + z * z + w * w);
};

/**
 * Calculates the squared euclidian distance between two vec4's
 *
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {Number} squared distance between a and b
 */
float vec4_squaredDistance(float *a, float *b) {
  float x = b[0] - a[0];
  float y = b[1] - a[1];
  float z = b[2] - a[2];
  float w = b[3] - a[3];
  return x * x + y * y + z * z + w * w;
};

/**
 * Calculates the length of a vec4
 *
 * @param {vec4} a vector to calculate length of
 * @returns {Number} length of a
 */
float vec4_length(float *a) {
  float x = a[0];
  float y = a[1];
  float z = a[2];
  float w = a[3];
  return sqrt(x * x + y * y + z * z + w * w);
};

/**
 * Calculates the squared length of a vec4
 *
 * @param {vec4} a vector to calculate squared length of
 * @returns {Number} squared length of a
 */
float vec4_squaredLength(float *a) {
  float x = a[0];
  float y = a[1];
  float z = a[2];
  float w = a[3];
  return x * x + y * y + z * z + w * w;
};

/**
 * Negates the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to negate
 * @returns {vec4} out
 */
float *vec4_negate(float *out, float *a) {
  out[0] = -a[0];
  out[1] = -a[1];
  out[2] = -a[2];
  out[3] = -a[3];
  return out;
};

/**
 * Returns the inverse of the components of a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to invert
 * @returns {vec4} out
 */
float *vec4_inverse(float *out, float *a) {
  out[0] = 1.0 / a[0];
  out[1] = 1.0 / a[1];
  out[2] = 1.0 / a[2];
  out[3] = 1.0 / a[3];
  return out;
};

/**
 * Normalize a vec4
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a vector to normalize
 * @returns {vec4} out
 */
float *vec4_normalize(float *out, float *a) {
  float x = a[0];
  float y = a[1];
  float z = a[2];
  float w = a[3];
  float len = x * x + y * y + z * z + w * w;
  if (len > 0) {
    len = 1 / sqrt(len);
    out[0] = x * len;
    out[1] = y * len;
    out[2] = z * len;
    out[3] = w * len;
  }
  return out;
};

/**
 * Calculates the dot product of two vec4's
 *
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @returns {Number} dot product of a and b
 */
float vec4_dot(float *a, float *b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
};

/**
 * Performs a linear interpolation between two vec4's
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the first operand
 * @param {vec4} b the second operand
 * @param {Number} t interpolation amount between the two inputs
 * @returns {vec4} out
 */
float *vec4_lerp(float *out, float *a, float *b, float t) {
  float ax = a[0];
  float ay = a[1];
  float az = a[2];
  float aw = a[3];
  out[0] = ax + t * (b[0] - ax);
  out[1] = ay + t * (b[1] - ay);
  out[2] = az + t * (b[2] - az);
  out[3] = aw + t * (b[3] - aw);
  return out;
};

/**
 * Transforms the vec4 with a mat4.
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the vector to transform
 * @param {mat4} m matrix to transform with
 * @returns {vec4} out
 */
float *vec4_transformMat4(float *out, float *a, float *m) {
  float x = a[0], y = a[1], z = a[2], w = a[3];
  out[0] = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
  out[1] = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
  out[2] = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
  out[3] = m[3] * x + m[7] * y + m[11] * z + m[15] * w;
  return out;
};

/**
 * Transforms the vec4 with a quat
 *
 * @param {vec4} out the receiving vector
 * @param {vec4} a the vector to transform
 * @param {quat} q quaternion to transform with
 * @returns {vec4} out
 */
float *vec4_transformQuat(float *out, float *a, float *q) {
  float x = a[0], y = a[1], z = a[2];
  float qx = q[0], qy = q[1], qz = q[2], qw = q[3];
  // calculate quat * vec
  float ix = qw * x + qy * z - qz * y;
  float iy = qw * y + qz * x - qx * z;
  float iz = qw * z + qx * y - qy * x;
  float iw = -qx * x - qy * y - qz * z;
  // calculate result * inverse quat
  out[0] = ix * qw + iw * -qx + iy * -qz - iz * -qy;
  out[1] = iy * qw + iw * -qy + iz * -qx - ix * -qz;
  out[2] = iz * qw + iw * -qz + ix * -qy - iy * -qx;
  out[3] = a[3];
  return out;
};

/**
 * Returns whether or not the vectors have exactly the same elements in the same position (when compared with ===)
 *
 * @param {vec4} a The first vector.
 * @param {vec4} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
int vec4_exactEquals(float *a, float *b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
};

/**
 * Returns whether or not the vectors have approximately the same elements in the same position.
 *
 * @param {vec4} a The first vector.
 * @param {vec4} b The second vector.
 * @returns {Boolean} True if the vectors are equal, false otherwise.
 */
int vec4_equals(float *a, float *b) {
  float a0 = a[0], a1 = a[1], a2 = a[2], a3 = a[3];
  float b0 = b[0], b1 = b[1], b2 = b[2], b3 = b[3];
  return (abs(a0 - b0) <= EPSILON * MAX(1.0, MAX(abs(a0), abs(b0))) &&
          abs(a1 - b1) <= EPSILON * MAX(1.0, MAX(abs(a1), abs(b1))) &&
          abs(a2 - b2) <= EPSILON * MAX(1.0, MAX(abs(a2), abs(b2))) &&
          abs(a3 - b3) <= EPSILON * MAX(1.0, MAX(abs(a3), abs(b3))));
};

/**
 * Alias for {@link vec4.subtract}
 * @function
 */
float *vec4_sub(float *out, float *a, float *b) {
  return vec4_subtract(out, a, b);
};

/**
 * Alias for {@link vec4.multiply}
 * @function
 */
float *vec4_mul(float *out, float *a, float *b) {
  return vec4_multiply(out, a, b);
};

/**
 * Alias for {@link vec4.divide}
 * @function
 */
float *vec4_div(float *out, float *a, float *b) {
  return vec4_divide(out, a, b);
};

/**
 * Alias for {@link vec4.distance}
 * @function
 */
float vec4_dist(float *a, float *b) {
  return vec4_distance(a, b);
};

/**
 * Alias for {@link vec4.squaredDistance}
 * @function
 */
float vec4_sqrDist(float *a, float *b) {
  return vec4_squaredDistance(a, b);
};

/**
 * Alias for {@link vec4.length}
 * @function
 */
float vec4_len(float *a) {
  return vec4_length(a);
};

/**
 * Alias for {@link vec4.squaredLength}
 * @function
 */
float vec4_sqrLen(float *a) {
  return vec4_squaredLength(a);
};