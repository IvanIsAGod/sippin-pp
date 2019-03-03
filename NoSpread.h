#pragma once
#include "sdk.h"
class CNoSpread
{
public:
	Vector SpreadFactor(int seed);
	void NoSpread(CUserCmd* pCmd);



	void CalcClient(Vector vSpreadVec, Vector ViewIn, Vector &ViewOut);
	void CalcServer(Vector vSpreadVec, Vector ViewIn, Vector &vecSpreadDir);
	void GetSpreadVec(CUserCmd*cmd, Vector &vSpreadVec);
	void CompensateInAccuracyNumeric(CUserCmd*cmd);
	void RollSpread(CBaseCombatWeapon*localWeap, int seed, CUserCmd*cmd, Vector& pflInAngles);
}; extern CNoSpread* g_NoSpread;

extern float RandomFloat(float flLow, float flHigh);

extern void RandomSeed(UINT Seed);

#define IA 16807
#define IM 2147483647
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define MAX_RANDOM_RANGE 0x7FFFFFFFUL

#define AM (1.0/IM)
#define EPS 1.2e-7
#define RNMX (1.0-EPS) 

class C_Random
{
private:
	int        m_idum;
	int        m_iy;
	int        m_iv[NTAB];
	int        GenerateRandomNumber(void);

public:
	void    SetSeed(int iSeed);
	float    RandomFloat(float flMinVal = 0.0f, float flMaxVal = 1.0f);
};
extern C_Random*Random;

#pragma once

typedef float matrix3x4[3][4];
#include "sse69.h"
#include <math.h>
#define pi 3.14159265358979323846
#define rad(a) a*(pi/180)
#define deg(a) a*(180/pi)

class ns_vector
{
public:
	ns_vector(float a = 0, float b = 0, float c = 0) { x = a; y = b; z = c; }

	inline void operator=(ns_vector a) { x = a.x; y = a.y; z = a.z; }
	inline bool operator!() { return !x && !y && !z; }
	inline bool operator==(ns_vector a) { return x == a.x && y == a.y && z == a.z; }
	inline bool operator!=(ns_vector a) { return x != a.x || y != a.y || z != a.z; }
	inline float& operator[](int i) { return ((float*)this)[i]; }
	inline ns_vector operator+(ns_vector a) { return ns_vector(x + a.x, y + a.y, z + a.z); }
	inline ns_vector operator-(ns_vector a) { return ns_vector(x - a.x, y - a.y, z - a.z); }
	inline ns_vector operator*(ns_vector a) { return ns_vector(x*a.x, y*a.y, z*a.z); }
	inline ns_vector operator/(ns_vector a) { return ns_vector(x / a.x, y / a.y, z / a.z); }
	inline void operator+=(ns_vector a) { *this = ns_vector(x + a.x, y + a.y, z + a.z); }
	inline void operator-=(ns_vector a) { *this = ns_vector(x - a.x, y - a.y, z - a.z); }
	inline void operator*=(ns_vector a) { *this = ns_vector(x*a.x, y*a.y, z*a.z); }
	inline void operator/=(ns_vector a) { *this = ns_vector(x / a.x, y / a.y, z / a.z); }
	inline ns_vector operator*(float a) { return ns_vector(x*a, y*a, z*a); }
	inline ns_vector operator/(float a) { return ns_vector(x / a, y / a, z / a); }
	inline void operator*=(float a) { *this = ns_vector(x*a, y*a, z*a); }
	inline void operator/=(float a) { *this = ns_vector(x / a, y / a, z / a); }
	inline float Length() { return sqrt2(x*x + y * y + z * z); }
	inline float Dot(ns_vector a) { return x * a.x + y * a.y + z * a.z; }
	inline ns_vector Cross(ns_vector a) { return ns_vector(y*a.z - z * a.y, z*a.x - x * a.z, x*a.y - y * a.x); }

	inline ns_vector Angle(ns_vector* up = 0)
	{
		if (!x && !y)
			return ns_vector(0, 0, 0);

		float roll = 0;

		if (up)
		{
			ns_vector left = (*up).Cross(*this);

			roll = deg(atan2f(left.z, (left.y * x) - (left.x * y)));
		}

		return ns_vector(deg(atan2f(-z, sqrt2(x*x + y * y))), deg(atan2f(y, x)), roll);
	}
	inline ns_vector Forward()
	{
		float sp, sy, cp, cy;

		SinCos(rad(x), sp, cp);
		SinCos(rad(y), sy, cy);

		return ns_vector(cp*cy, cp*sy, -sp);
	}
	inline ns_vector Right()
	{
		float sp, sy, sr, cp, cy, cr;

		SinCos(rad(x), sp, cp);
		SinCos(rad(y), sy, cy);
		SinCos(rad(z), sr, cr);

		return ns_vector(1 * sr*sp*cy + -1 * cr*-sy, -1 * sr*sp*sy + -1 * cr*cy, -1 * sr*cr);
	}
	inline ns_vector Up()
	{
		float sp, sy, sr, cp, cy, cr;

		SinCos(rad(x), sp, cp);
		SinCos(rad(y), sy, cy);
		SinCos(rad(z), sr, cr);

		return ns_vector(cr*sp*cy + -sr * -sy, cr*sp*sy + -sr * cy, cr*cp);
	}

	float x, y, z;
};