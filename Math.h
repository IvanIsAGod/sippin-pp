#pragma once
namespace Math
{
	extern float NormalizeYaw(float value);
	float remap_value(float val, float A, float B, float C, float D);
	extern void NormalizeAngle(QAngle & value);
	extern QAngle NormalizeAngle2(QAngle value);
	extern QAngle CalcAngle(Vector src, Vector dst);
	void CalcAngle(Vector src, Vector dst, Vector & angles);
	void calculate_angle(Vector src, Vector dst, Vector & angles);
	extern void VectorAngles(const Vector&vecForward, Vector&vecAngles);
	extern void AngleVectors(const Vector angles, Vector& forward, Vector& right, Vector& up);
	extern void VectorMA(const Vector & start, float scale, const Vector & direction, Vector & dest);
	extern void NormalizeVector(Vector & vecIn);
	extern void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	extern	void VectorTransform(const float * in1, const matrix3x4_t & in2, float * out);
	extern void VectorTransform2(const float *in1, const matrix3x4_t& in2, float *out);
	extern void VectorSubtract(const Vector & a, const Vector & b, Vector & c);
	extern void NormalizeNum(Vector & vIn, Vector & vOut);
	extern void AngleVectors(const QAngle &angles, Vector* forward);
	extern void AngleVectors(const QAngle & angles, Vector & forward);
	void Normalize(Vector & vIn, Vector & vOut);
	void normalize(Vector & vIn, Vector & vOut);
	extern void ClampAngles(QAngle& angles);
	extern float RandomFloat(float min, float max);
	int RandomInt(int a, int b);
	extern void AngleMatrix2(const QAngle angles, matrix3x4_t & matrix);
	extern void MatrixSetColumn(const Vector & in, int column, matrix3x4_t & out);
	extern void AngleMatrix(const QAngle & angles, const Vector & position, matrix3x4_t & matrix_out);
	extern void MatrixCopy(const matrix3x4_t & source, matrix3x4_t & target);
	extern void MatrixMultiply(matrix3x4_t & in1, const matrix3x4_t & in2);
	extern void VectorRotate(const float * in1, const matrix3x4_t & in2, float * out);
	extern void VectorRotate(const Vector & in1, const matrix3x4_t & in2, Vector & out);
	extern void VectorRotate(const Vector & in1, const QAngle & in2, Vector & out);
	extern float RandomFloat2(float min, float max);
    extern void AngleVectors(const Vector &angles, Vector *forward, Vector *right, Vector *up);
    extern void vector_subtract(const Vector& a, const Vector& b, Vector& c);
	extern void vector_transform(const Vector in1, float in2[3][4], Vector &out);
	extern void VectorTransform(Vector& in1, matrix3x4_t& in2, Vector &out);
	extern bool DotInVector(Vector vector, Vector dot, float limit);
	extern float GetCurtime();
}

extern bool enabledtp;