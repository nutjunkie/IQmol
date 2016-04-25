#include "GLSLmath.h"

// -------------------------

mat4x4::mat4x4()
{
	M[0] = 1.0f; M[4] = 0.0f; M[8] = 0.0f; M[12] = 0.0f;
	M[1] = 0.0f; M[5] = 1.0f; M[9] = 0.0f; M[13] = 0.0f;
	M[2] = 0.0f; M[6] = 0.0f; M[10] = 1.0f; M[14] = 0.0f;
	M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;
}

mat4x4::~mat4x4()
{
}

mat4x4::mat4x4(const mat4x4 &Matrix)
{
	for(int i = 0; i < 16; i++)
	{
		M[i] = Matrix.M[i];
	}
}

mat4x4& mat4x4::operator = (const mat4x4 &Matrix)
{
	for(int i = 0; i < 16; i++)
	{
		M[i] = Matrix.M[i];
	}

	return *this;
}

float& mat4x4::operator [] (int Index)
{
	return M[Index];
}

float* mat4x4::operator & ()
{
	return (float*)this;
}

mat4x4 operator * (const mat4x4 &Matrix1, const mat4x4 &Matrix2)
{
	mat4x4 Matrix3;

	Matrix3.M[0] = Matrix1.M[0] * Matrix2.M[0] + Matrix1.M[4] * Matrix2.M[1] + Matrix1.M[8] * Matrix2.M[2] + Matrix1.M[12] * Matrix2.M[3];
	Matrix3.M[1] = Matrix1.M[1] * Matrix2.M[0] + Matrix1.M[5] * Matrix2.M[1] + Matrix1.M[9] * Matrix2.M[2] + Matrix1.M[13] * Matrix2.M[3];
	Matrix3.M[2] = Matrix1.M[2] * Matrix2.M[0] + Matrix1.M[6] * Matrix2.M[1] + Matrix1.M[10] * Matrix2.M[2] + Matrix1.M[14] * Matrix2.M[3];
	Matrix3.M[3] = Matrix1.M[3] * Matrix2.M[0] + Matrix1.M[7] * Matrix2.M[1] + Matrix1.M[11] * Matrix2.M[2] + Matrix1.M[15] * Matrix2.M[3];

	Matrix3.M[4] = Matrix1.M[0] * Matrix2.M[4] + Matrix1.M[4] * Matrix2.M[5] + Matrix1.M[8] * Matrix2.M[6] + Matrix1.M[12] * Matrix2.M[7];
	Matrix3.M[5] = Matrix1.M[1] * Matrix2.M[4] + Matrix1.M[5] * Matrix2.M[5] + Matrix1.M[9] * Matrix2.M[6] + Matrix1.M[13] * Matrix2.M[7];
	Matrix3.M[6] = Matrix1.M[2] * Matrix2.M[4] + Matrix1.M[6] * Matrix2.M[5] + Matrix1.M[10] * Matrix2.M[6] + Matrix1.M[14] * Matrix2.M[7];
	Matrix3.M[7] = Matrix1.M[3] * Matrix2.M[4] + Matrix1.M[7] * Matrix2.M[5] + Matrix1.M[11] * Matrix2.M[6] + Matrix1.M[15] * Matrix2.M[7];

	Matrix3.M[8] = Matrix1.M[0] * Matrix2.M[8] + Matrix1.M[4] * Matrix2.M[9] + Matrix1.M[8] * Matrix2.M[10] + Matrix1.M[12] * Matrix2.M[11];
	Matrix3.M[9] = Matrix1.M[1] * Matrix2.M[8] + Matrix1.M[5] * Matrix2.M[9] + Matrix1.M[9] * Matrix2.M[10] + Matrix1.M[13] * Matrix2.M[11];
	Matrix3.M[10] = Matrix1.M[2] * Matrix2.M[8] + Matrix1.M[6] * Matrix2.M[9] + Matrix1.M[10] * Matrix2.M[10] + Matrix1.M[14] * Matrix2.M[11];
	Matrix3.M[11] = Matrix1.M[3] * Matrix2.M[8] + Matrix1.M[7] * Matrix2.M[9] + Matrix1.M[11] * Matrix2.M[10] + Matrix1.M[15] * Matrix2.M[11];

	Matrix3.M[12] = Matrix1.M[0] * Matrix2.M[12] + Matrix1.M[4] * Matrix2.M[13] + Matrix1.M[8] * Matrix2.M[14] + Matrix1.M[12] * Matrix2.M[15];
	Matrix3.M[13] = Matrix1.M[1] * Matrix2.M[12] + Matrix1.M[5] * Matrix2.M[13] + Matrix1.M[9] * Matrix2.M[14] + Matrix1.M[13] * Matrix2.M[15];
	Matrix3.M[14] = Matrix1.M[2] * Matrix2.M[12] + Matrix1.M[6] * Matrix2.M[13] + Matrix1.M[10] * Matrix2.M[14] + Matrix1.M[14] * Matrix2.M[15];
	Matrix3.M[15] = Matrix1.M[3] * Matrix2.M[12] + Matrix1.M[7] * Matrix2.M[13] + Matrix1.M[11] * Matrix2.M[14] + Matrix1.M[15] * Matrix2.M[15];

	return Matrix3;
}

vec2 operator * (const mat4x4 &Matrix, const vec2 &Vector)
{
	return Matrix * vec4(Vector, 0.0f, 1.0f);
}

vec3 operator * (const mat4x4 &Matrix, const vec3 &Vector)
{
	return Matrix * vec4(Vector, 1.0f);
}

vec4 operator * (const mat4x4 &Matrix, const vec4 &Vector)
{
	vec4 v;

	v.x = Matrix.M[0] * Vector.x + Matrix.M[4] * Vector.y + Matrix.M[8] * Vector.z + Matrix.M[12] * Vector.w;
	v.y = Matrix.M[1] * Vector.x + Matrix.M[5] * Vector.y + Matrix.M[9] * Vector.z + Matrix.M[13] * Vector.w;
	v.z = Matrix.M[2] * Vector.x + Matrix.M[6] * Vector.y + Matrix.M[10] * Vector.z + Matrix.M[14] * Vector.w;
	v.w = Matrix.M[3] * Vector.x + Matrix.M[7] * Vector.y + Matrix.M[11] * Vector.z + Matrix.M[15] * Vector.w;
	
	return v;
}

// ----------------------------------------------------------------------------------------------------------------------------

float dot(const vec2 &u, const vec2 &v)
{
	return u.x * v.x + u.y * v.y;
}

float length(const vec2 &u)
{
	return sqrt(u.x * u.x + u.y * u.y);
}

float length2(const vec2 &u)
{
	return u.x * u.x + u.y * u.y;
}

vec2 normalize(const vec2 &u)
{
	return u * (1.0f / sqrt(u.x * u.x + u.y * u.y));
}

vec2 reflect(const vec2 &i, const vec2 &n)
{
	return i - 2.0f * dot(n, i) * n;
}

vec2 refract(const vec2 &i, const vec2 &n, float eta)
{
	vec2 r;

	float ndoti = dot(n, i), k = 1.0f - eta * eta * (1.0f - ndoti * ndoti);

	if(k >= 0.0f)
	{
		r = eta * i - n * (eta * ndoti + sqrt(k));
	}

	return r;
}

vec2 rotate(const vec2 &u, float angle)
{
	return RotationMatrix(angle, vec3(0.0f, 0.0f, 1.0f)) * vec4(u, 0.0f, 1.0f);
}

// ----------------------------------------------------------------------------------------------------------------------------

vec3 cross(const vec3 &u, const vec3 &v)
{
	return vec3(u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x);
}

float dot(const vec3 &u, const vec3 &v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

float length(const vec3 &u)
{
	return sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
}

float length2(const vec3 &u)
{
	return u.x * u.x + u.y * u.y + u.z * u.z;
}

vec3 mix(const vec3 &u, const vec3 &v, float a)
{
	return u * (1.0f - a) + v * a;
}

vec3 normalize(const vec3 &u)
{
	return u * (1.0f / sqrt(u.x * u.x + u.y * u.y + u.z * u.z));
}

vec3 reflect(const vec3 &i, const vec3 &n)
{
	return i - 2.0f * dot(n, i) * n;
}

vec3 refract(const vec3 &i, const vec3 &n, float eta)
{
	vec3 r;

	float ndoti = dot(n, i), k = 1.0f - eta * eta * (1.0f - ndoti * ndoti);

	if(k >= 0.0f)
	{
		r = eta * i - n * (eta * ndoti + sqrt(k));
	}

	return r;
}

vec3 rotate(const vec3 &u, float angle, const vec3 &v)
{
	return RotationMatrix(angle, v) * vec4(u, 1.0f);
}

// ----------------------------------------------------------------------------------------------------------------------------

mat4x4 BiasMatrix()
{
	mat4x4 B;

	B[0] = 0.5f; B[4] = 0.0f; B[8] = 0.0f; B[12] = 0.5f;
	B[1] = 0.0f; B[5] = 0.5f; B[9] = 0.0f; B[13] = 0.5f;
	B[2] = 0.0f; B[6] = 0.0f; B[10] = 0.5f; B[14] = 0.5f;
	B[3] = 0.0f; B[7] = 0.0f; B[11] = 0.0f; B[15] = 1.0f;

	return B;
}

mat4x4 BiasMatrixInverse()
{
	mat4x4 BI;

	BI[0] = 2.0f; BI[4] = 0.0f; BI[8] = 0.0f; BI[12] = -1.0f;
	BI[1] = 0.0f; BI[5] = 2.0f; BI[9] = 0.0f; BI[13] = -1.0f;
	BI[2] = 0.0f; BI[6] = 0.0f; BI[10] = 2.0f; BI[14] = -1.0f;
	BI[3] = 0.0f; BI[7] = 0.0f; BI[11] = 0.0f; BI[15] = 1.0f;

	return BI;
}

mat4x4 ViewMatrix(const vec3 &x, const vec3 &y, const vec3 &z, const vec3 &position)
{
	mat4x4 V;

	V[0] = x.x;
	V[1] = y.x;
	V[2] = z.x;
	V[3] = 0.0f;

	V[4] = x.y;
	V[5] = y.y;
	V[6] = z.y;
	V[7] = 0.0f;

	V[8] = x.z;
	V[9] = y.z;
	V[10] = z.z;
	V[11] = 0.0f;

	V[12] = - dot(x, position);
	V[13] = - dot(y, position);
	V[14] = - dot(z, position);
	V[15] = 1.0f;

	return V;
}

mat4x4 ViewMatrixInverse(mat4x4 &V)
{
	mat4x4 VI;

	VI[0] = V[0];
	VI[1] = V[4];
	VI[2] = V[8];
	VI[3] = 0.0f;

	VI[4] = V[1];
	VI[5] = V[5];
	VI[6] = V[9];
	VI[7] = 0.0f;

	VI[8] = V[2];
	VI[9] = V[6];
	VI[10] = V[10];
	VI[11] = 0.0f;

	VI[12] = - (VI[0] * V[12] + VI[4] * V[13] + VI[8] * V[14]);
	VI[13] = - (VI[1] * V[12] + VI[5] * V[13] + VI[9] * V[14]);
	VI[14] = - (VI[2] * V[12] + VI[6] * V[13] + VI[10] * V[14]);
	VI[15] = 1.0f;

	return VI;
}

mat4x4 OrthogonalProjectionMatrix(float left, float right, float bottom, float top, float n, float f)
{
	mat4x4 OP;

	OP[0] = 2.0f / (right - left);
	OP[1] = 0.0f;
	OP[2] = 0.0f;
	OP[3] = 0.0f;

	OP[4] = 0.0f;
	OP[5] = 2.0f / (top - bottom);
	OP[6] = 0.0f;
	OP[7] = 0.0f;

	OP[8] = 0.0f;
	OP[9] = 0.0f;
	OP[10] = -2.0f / (f - n);
	OP[11] = 0.0f;

	OP[12] = - (right + left) / (right - left);
	OP[13] = - (top + bottom) / (top - bottom);
	OP[14] = - (f + n) / (f - n);
	OP[15] = 1.0f;

	return OP;
}

mat4x4 PerspectiveProjectionMatrix(float fovy, float x, float y, float n, float f)
{
	mat4x4 PP;

	float coty = 1.0f / tan(fovy * (float)M_PI / 360.0f);
	float aspect = x / (y > 0.0f ? y : 1.0f);

	PP[0] = coty / aspect;
	PP[1] = 0.0f;
	PP[2] = 0.0f;
	PP[3] = 0.0f;

	PP[4] = 0.0f;
	PP[5] = coty;
	PP[6] = 0.0f;
	PP[7] = 0.0f;

	PP[8] = 0.0f;
	PP[9] = 0.0f;
	PP[10] = (n + f) / (n - f);
	PP[11] = -1.0f;

	PP[12] = 0.0f;
	PP[13] = 0.0f;
	PP[14] = 2.0f * n * f / (n - f);
	PP[15] = 0.0f;

	return PP;
}

mat4x4 PerspectiveProjectionMatrixInverse(mat4x4 &PP)
{
	mat4x4 PPI;

	PPI[0] = 1.0f / PP[0];
	PPI[1] = 0.0f;
	PPI[2] = 0.0f;
	PPI[3] = 0.0f;

	PPI[4] = 0.0f;
	PPI[5] = 1.0f / PP[5];
	PPI[6] = 0.0f;
	PPI[7] = 0.0f;

	PPI[8] = 0.0f;
	PPI[9] = 0.0f;
	PPI[10] = 0.0f;
	PPI[11] = 1.0f / PP[14];

	PPI[12] = 0.0f;
	PPI[13] = 0.0f;
	PPI[14] = 1.0f / PP[11];
	PPI[15] = - PP[10] / (PP[11] * PP[14]);

	return PPI;
}

mat4x4 RotationMatrix(float angle, const vec3 &u)
{
	mat4x4 R;

	angle = angle / 180.0f * (float)M_PI;

	vec3 v = normalize(u);

	float c = 1.0f - cos(angle), s = sin(angle);

	R[0] = 1.0f + c * (v.x * v.x - 1.0f);
	R[1] = c * v.x * v.y + v.z * s;
	R[2] = c * v.x * v.z - v.y * s;
	R[3] = 0.0f;

	R[4] = c * v.x * v.y - v.z * s;
	R[5] = 1.0f + c * (v.y * v.y - 1.0f);
	R[6] = c * v.y * v.z + v.x * s;
	R[7] = 0.0f;

	R[8] = c * v.x * v.z + v.y * s;
	R[9] = c * v.y * v.z - v.x * s;
	R[10] = 1.0f + c * (v.z * v.z - 1.0f);
	R[11] = 0.0f;

	R[12] = 0.0f;
	R[13] = 0.0f;
	R[14] = 0.0f;
	R[15] = 1.0f;

	return R;
}

mat4x4 ScaleMatrix(float x, float y, float z)
{
	mat4x4 S;

	S[0] = x;
	S[1] = 0.0f;
	S[2] = 0.0f;
	S[3] = 0.0f;

	S[4] = 0.0f;
	S[5] = y;
	S[6] = 0.0f;
	S[7] = 0.0f;

	S[8] = 0.0f;
	S[9] = 0.0f;
	S[10] = z;
	S[11] = 0.0f;

	S[12] = 0.0f;
	S[13] = 0.0f;
	S[14] = 0.0f;
	S[15] = 1.0f;

	return S;
}

mat4x4 TranslationMatrix(float x, float y, float z)
{
	mat4x4 T;

	T[0] = 1.0f;
	T[1] = 0.0f;
	T[2] = 0.0f;
	T[3] = 0.0f;

	T[4] = 0.0f;
	T[5] = 1.0f;
	T[6] = 0.0f;
	T[7] = 0.0f;

	T[8] = 0.0f;
	T[9] = 0.0f;
	T[10] = 1.0f;
	T[11] = 0.0f;

	T[12] = x;
	T[13] = y;
	T[14] = z;
	T[15] = 1.0f;

	return T;
}
