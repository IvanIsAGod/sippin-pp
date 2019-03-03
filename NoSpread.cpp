#include "sdk.h"
#include "global.h"
#include "NoSpread.h"
CNoSpread* g_NoSpread = new CNoSpread;
void SpreadVectorAngles(const Vector &vecForward, Vector &vecAngles)
{
	Vector vecView;
	if (vecForward[1] == 0.f && vecForward[0] == 0.f)
	{
		vecView[0] = 0.f;
		vecView[1] = 0.f;
	}
	else
	{
		vecView[1] = atan2(vecForward[1], vecForward[0]) * 180.f / M_PI;

		if (vecView[1] < 0.f)
			vecView[1] += 360.f;

		vecView[2] = sqrt(vecForward[0] * vecForward[0] + vecForward[1] * vecForward[1]);

		vecView[0] = atan2(vecForward[2], vecView[2]) * 180.f / M_PI;
	}

	vecAngles[0] = -vecView[0];
	vecAngles[1] = vecView[1];
	vecAngles[2] = 0.f;
}

void AngleToVectors(const Vector &angles, Vector *forward, Vector *right, Vector *up)
{
	float angle;
	static float sr, sp, sy, cr, cp, cy, cpi = float((M_PI * 2 / 360));

	angle = angles.y * cpi;
	sy = sin(angle);
	cy = cos(angle);
	angle = angles.x * cpi;
	sp = sin(angle);
	cp = cos(angle);
	angle = angles.z * cpi;
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr*sp*cy + -1 * cr*-sy);
		right->y = (-1 * sr*sp*sy + -1 * cr*cy);
		right->z = -1 * sr*cp;
	}

	if (up)
	{
		up->x = (cr*sp*cy + -sr * -sy);
		up->y = (cr*sp*sy + -sr * cy);
		up->z = cr * cp;
	}
}

typedef float(*RandomFloat_t)(float, float);
RandomFloat_t m_RandomFloat;
float RandomFloat(float flLow, float flHigh)
{
	if (m_RandomFloat == NULL)
		m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat");

	return m_RandomFloat(flLow, flHigh);
}
typedef void(*RandomSeed_t)(UINT);
RandomSeed_t m_RandomSeed = 0;
void RandomSeed(UINT Seed)
{


	if (m_RandomSeed == NULL)
		m_RandomSeed = (RandomSeed_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomSeed");

	m_RandomSeed(Seed);

	return;
}
Vector CNoSpread::SpreadFactor(int seed)
{
	auto GetWavRoll = [](Vector dir) -> float
	{
		Vector forward(1, -dir.x, dir.y);

		Vector up(0, -dir.x, abs(dir.y + (1 / dir.y) + (1 / dir.y)));

		if (dir.x > 0 && dir.y < 0)
			up.y = abs(up.y);
		else if (dir.x < 0 && dir.y < 0)
			up.y = -abs(up.y);

		Vector left = up.Cross(forward);

		float roll = RAD2DEG(atan2f(left.z, (left.y * forward.x) - (left.x * forward.y)));

		return roll;
	};

	Vector vecForward, vecRight, vecDir, vecUp, vecAntiDir;
	float flSpread, flInaccuracy;
	Vector qAntiSpread = csgo::UserCmd->viewangles;


	csgo::MainWeapon->UpdateAccuracyPenalty();

	flSpread = csgo::MainWeapon->GetSpread();

	flInaccuracy = csgo::MainWeapon->GetInaccuracy();
	if (seed == csgo::UserCmd->random_seed)
	{
		csgo::UserCmd->random_seed = MD5_PseudoRandom(csgo::UserCmd->command_number) & 0x7FFFFFFF;
		RandomSeed((csgo::UserCmd->random_seed & 0xFF) + 1);
	}
	else
	{
		RandomSeed(seed);
	}



	float fRand1 = RandomFloat(0.f, 1.f);
	float fRandPi1 = RandomFloat(0.f, 2.f * (float)M_PI);
	float fRand2 = RandomFloat(0.f, 1.f);
	float fRandPi2 = RandomFloat(0.f, 2.f * (float)M_PI);

	float m_flRecoilIndex = csgo::MainWeapon->GetFloatRecoilIndex();


	if (csgo::MainWeapon->WeaponID() == 64)
	{
		if (csgo::UserCmd->buttons & IN_ATTACK2)
		{
			fRand1 = 1.f - fRand1 * fRand1;
			fRand2 = 1.f - fRand2 * fRand2;
		}
	}
	else if (csgo::MainWeapon->WeaponID() == NEGEV && m_flRecoilIndex < 3.f)
	{
		for (int i = 3; i > m_flRecoilIndex; --i)
		{
			fRand1 *= fRand1;
			fRand2 *= fRand2;
		}

		fRand1 = 1.f - fRand1;
		fRand2 = 1.f - fRand2;
	}

	float fRandInaccuracy = fRand1 * csgo::MainWeapon->GetInaccuracy();
	float fRandSpread = fRand2 * csgo::MainWeapon->GetSpread();

	float fSpreadX = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
	float fSpreadY = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;

	AngleToVectors(csgo::UserCmd->viewangles, &vecForward, &vecRight, &vecUp);

	vecDir.x = (float)((float)(vecRight.x * fSpreadX) + vecForward.x) + (float)(vecUp.x * fSpreadY);
	vecDir.y = (float)((float)(fSpreadX * vecRight.y) + vecForward.y) + (float)(fSpreadY * vecUp.y);
	vecDir.z = (float)((float)(vecRight.z * fSpreadX) + vecForward.z) + (float)(vecUp.z * fSpreadY);

	vecAntiDir = vecForward + (vecRight * -fSpreadX) + (vecUp * -fSpreadY);

	vecAntiDir.NormalizeInPlace();

	//vecAntiDir.z 



	SpreadVectorAngles(vecAntiDir, qAntiSpread);

	//qAntiSpread.x += RAD2DEG(atan(sqrt(fSpreadX * fSpreadX + fSpreadY * fSpreadY)));
	//qAntiSpread.z = RAD2DEG(atan2(fSpreadX, fSpreadY));
	return qAntiSpread;
}

void CNoSpread::NoSpread(CUserCmd* pCmd)
{
	Vector vecForward, vecRight, vecDir, vecUp, vecAntiDir;
	float flSpread, flInaccuracy;
	Vector qAntiSpread;

	csgo::MainWeapon->UpdateAccuracyPenalty();

	flSpread = csgo::MainWeapon->GetSpread();

	flInaccuracy = csgo::MainWeapon->GetInaccuracy();
	csgo::UserCmd->random_seed = MD5_PseudoRandom(csgo::UserCmd->command_number) & 0x7FFFFFFF;
	RandomSeed((csgo::UserCmd->random_seed & 0xFF) + 1);


	float fRand1 = RandomFloat(0.f, 1.f);
	float fRandPi1 = RandomFloat(0.f, 2.f * (float)M_PI);
	float fRand2 = RandomFloat(0.f, 1.f);
	float fRandPi2 = RandomFloat(0.f, 2.f * (float)M_PI);

	float m_flRecoilIndex = csgo::MainWeapon->GetFloatRecoilIndex();


	if (csgo::MainWeapon->WeaponID() == 64)
	{
		if (csgo::UserCmd->buttons & IN_ATTACK2)
		{
			fRand1 = 1.f - fRand1 * fRand1;
			fRand2 = 1.f - fRand2 * fRand2;
		}
	}
	else if (csgo::MainWeapon->WeaponID() == NEGEV && m_flRecoilIndex < 3.f)
	{
		for (int i = 3; i > m_flRecoilIndex; --i)
		{
			fRand1 *= fRand1;
			fRand2 *= fRand2;
		}

		fRand1 = 1.f - fRand1;
		fRand2 = 1.f - fRand2;
	}

	float fRandInaccuracy = fRand1 * csgo::MainWeapon->GetInaccuracy();
	float fRandSpread = fRand2 * csgo::MainWeapon->GetSpread();

	float fSpreadX = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
	float fSpreadY = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;


	pCmd->viewangles.x += RAD2DEG(atan(sqrt(fSpreadX * fSpreadX + fSpreadY * fSpreadY)));
	pCmd->viewangles.z = RAD2DEG(atan2(fSpreadX, fSpreadY));
}

C_Random* Random = new C_Random;

void C_Random::SetSeed(int iSeed)
{
	m_idum = ((iSeed < 0) ? iSeed : -iSeed);
	m_iy = 0;
}

int C_Random::GenerateRandomNumber()
{
	int j;
	int k;

	if (m_idum <= 0 || !m_iy) {
		if (-(m_idum) < 1) m_idum = 1;
		else m_idum = -(m_idum);

		for (j = NTAB + 7; j >= 0; j--) {
			k = (m_idum) / IQ;
			m_idum = IA * (m_idum - k * IQ) - IR * k;
			if (m_idum < 0) m_idum += IM;
			if (j < NTAB) m_iv[j] = m_idum;
		}
		m_iy = m_iv[0];
	}
	k = (m_idum) / IQ;
	m_idum = IA * (m_idum - k * IQ) - IR * k;
	if (m_idum < 0) m_idum += IM;
	j = m_iy / NDIV;
	m_iy = m_iv[j];
	m_iv[j] = m_idum;

	return m_iy;
}

float C_Random::RandomFloat(float flLow, float flHigh)
{
	float fl = AM * (float)this->GenerateRandomNumber();
	if (fl > RNMX)
		fl = float(RNMX);
	return (fl * (flHigh - flLow)) + flLow;
}
void AngleVectors(const Vector angles, Vector& forward, Vector& right, Vector& up)
{
	float angle;
	static float sr, sp, sy, cr, cp, cy;

	angle = angles.y * (M_PI * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);

	angle = angles.x * (M_PI * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles.z * (M_PI * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	//if (forward)
	//{
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	//}
	//if (right)
	//{
	right.x = (-1 * sr*sp*cy + -1 * cr*-sy);
	right.y = (-1 * sr*sp*sy + -1 * cr*cy);
	right.z = -1 * sr*cp;
	//}
	//if (up)
	//{
	up.x = (cr*sp*cy + -sr * -sy);
	up.y = (cr*sp*sy + -sr * cy);
	up.z = cr * cp;
	//}
}
void VectorAngles(const Vector &forward, Vector &angles)
{
	static float tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 90.0;
		else
			pitch = 270.0;
	}
	else
	{
		yaw = (float)(atan2(forward[1], forward[0]) * 180.0 / M_PI);
		if (yaw < 0)
			yaw += 360.0;
		tmp = (float)sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (float)(atan2(forward[2], tmp) * 180 / M_PI);
	}

	angles.x = -pitch;
	angles.y = yaw;
	angles.z = 0;
}
void CNoSpread::CalcClient(Vector vSpreadVec, Vector ViewIn, Vector &ViewOut)
{
	Vector vecForward, vecRight, vecUp, vecSpreadDir;

	AngleVectors(ViewIn, vecForward, vecRight, vecUp);

	vecSpreadDir = vecForward + vecRight * -vSpreadVec.x + vecUp * -vSpreadVec.y;

	VectorAngles(vecSpreadDir, ViewOut);

}

void CNoSpread::CalcServer(Vector vSpreadVec, Vector ViewIn, Vector &vecSpreadDir)
{
	Vector vecViewForward, vecViewRight, vecViewUp;

	AngleVectors(ViewIn, vecViewForward, vecViewRight, vecViewUp);

	vecSpreadDir = vecViewForward + vecViewRight * vSpreadVec.x + vecViewUp * vSpreadVec.y;
}

void CNoSpread::GetSpreadVec(CUserCmd*pCmd, Vector &vSpreadVec)
{
	CBaseEntity* self = csgo::LocalPlayer;

	if (!self)
		return;

	CBaseCombatWeapon* localWeapon = self->GetWeapon();

	if (!localWeapon)
		return;

	Vector vSpread;

	localWeapon->UpdateAccuracyPenalty();

	Random->SetSeed((pCmd->random_seed & 0xFF) + 1);

	float fRand1 = Random->RandomFloat(0.f, 1.f);
	float fRandPi1 = Random->RandomFloat(0.f, 2.f * (float)M_PI);
	float fRand2 = Random->RandomFloat(0.f, 1.f);
	float fRandPi2 = Random->RandomFloat(0.f, 2.f * (float)M_PI);

	float m_flRecoilIndex = csgo::MainWeapon->GetFloatRecoilIndex();

	if (csgo::MainWeapon->WeaponID() == 64)
	{
		if (csgo::UserCmd->buttons & IN_ATTACK2)
		{
			fRand1 = 1.f - fRand1 * fRand1;
			fRand2 = 1.f - fRand2 * fRand2;
		}
	}
	else if (csgo::MainWeapon->WeaponID() == NEGEV && m_flRecoilIndex < 3.f)
	{
		for (int i = 3; i > m_flRecoilIndex; --i)
		{
			fRand1 *= fRand1;
			fRand2 *= fRand2;
		}

		fRand1 = 1.f - fRand1;
		fRand2 = 1.f - fRand2;
	}

	float fRandInaccuracy = fRand1 * localWeapon->GetInaccuracy();
	float fRandSpread = fRand2 * localWeapon->GetSpread();

	vSpreadVec.x = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
	vSpreadVec.y = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;

}
long double VectorNormalize2(float *flAngles)
{
	long double ldRet = sqrt(flAngles[0] * flAngles[0] + flAngles[1] * flAngles[1] + flAngles[2] * flAngles[2]);

	if (ldRet != 0.0f)
	{
		flAngles[0] = flAngles[0] * (1.0f / (float)ldRet);
		flAngles[1] = flAngles[1] * (1.0f / (float)ldRet);
		flAngles[2] = flAngles[2] * (1.0f / (float)ldRet);
	}

	return ldRet;
}


void CNoSpread::CompensateInAccuracyNumeric(CUserCmd*cmd)
{
	Vector Aim, qAntiSpread;

	Vector vSpreadVec, vForward, vServerSpreadForward;

	Aim = cmd->viewangles;

	//AngleVectors(Aim, vForward);

	GetSpreadVec(cmd, vSpreadVec);

	CalcClient(vSpreadVec, Aim, qAntiSpread);

	for (int i = 0; i <= 24; i++)
	{
		CalcServer(vSpreadVec, qAntiSpread, vServerSpreadForward);

		Vector qModifer;
		VectorAngles(vServerSpreadForward, qModifer);

		qModifer = Aim - qModifer;

		if (sqrt((qModifer.x * qModifer.x) + (qModifer.y * qModifer.y)) == 0)
		{
			break;
		}

		qAntiSpread = qAntiSpread + qModifer;

		cmd->viewangles = qAntiSpread;
	}

	cmd->viewangles -= csgo::LocalPlayer->GetPunchAngle() * 2;
}
#define square( x ) ( x * x ) 

float sseSqrt(float x)
{
	float root = 0.0f;

	__asm
	{
		sqrtss xmm0, x
		movss root, xmm0
	}

	return root;
}

void VectorAngles(const Vector &forward, const Vector &pseudoup, Vector &angles)
{
	Vector left;

	left = CrossProduct(pseudoup, forward);

	left.NormalizeInPlace();

	float xyDist = sseSqrt(forward[0] * forward[0] + forward[1] * forward[1]);

	if (xyDist > 0.001f)
	{
		angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));
		angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

		float up_z = (left[1] * forward[0]) - (left[0] * forward[1]);

		angles[2] = RAD2DEG(atan2f(left[2], up_z));
	}
	else
	{
		angles[1] = RAD2DEG(atan2f(-left[0], left[1]));
		angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));
		angles[2] = 0;
	}
}
//#include "RollVector.h"
void CNoSpread::RollSpread(CBaseCombatWeapon*localWeap, int seed, CUserCmd*cmd, Vector& pflInAngles)
{
	auto GetWavRoll = [](Vector dir) -> float
	{
		Vector forward(1, -dir.x, dir.y);

		Vector up(0, -dir.x, abs(dir.y + (1 / dir.y) + (1 / dir.y)));

		if (dir.x > 0 && dir.y < 0)
			up.y = abs(up.y);
		else if (dir.x < 0 && dir.y < 0)
			up.y = -abs(up.y);

		Vector left = up.Cross(forward);

		float roll = RAD2DEG(atan2f(left.z, (left.y * forward.x) - (left.x * forward.y)));

		return roll;
	};

	localWeap->UpdateAccuracyPenalty();

	Random->SetSeed((seed & 0xFF) + 1);

	cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;

	float fRand1 = Random->RandomFloat(0.f, 1.f);
	float fRandPi1 = Random->RandomFloat(0.f, 2.f * (float)M_PI);
	float fRand2 = Random->RandomFloat(0.f, 1.f);
	float fRandPi2 = Random->RandomFloat(0.f, 2.f * (float)M_PI);

	float m_flRecoilIndex = csgo::MainWeapon->GetFloatRecoilIndex();

	if (csgo::MainWeapon->WeaponID() == 64)
	{
		if (csgo::UserCmd->buttons & IN_ATTACK2)
		{
			fRand1 = 1.f - fRand1 * fRand1;
			fRand2 = 1.f - fRand2 * fRand2;
		}
	}
	else if (csgo::MainWeapon->WeaponID() == NEGEV && m_flRecoilIndex < 3.f)
	{
		for (int i = 3; i > m_flRecoilIndex; --i)
		{
			fRand1 *= fRand1;
			fRand2 *= fRand2;
		}

		fRand1 = 1.f - fRand1;
		fRand2 = 1.f - fRand2;
	}

	float fRandInaccuracy = fRand1 * localWeap->GetInaccuracy();
	float fRandSpread = fRand2 * localWeap->GetSpread();

	Vector vForward, vRight, vUp, vDir;
	Vector vView, vSpread, flIdentity[3];


	vSpread.x = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
	vSpread.y = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;

	ns_vector v = ns_vector(pflInAngles.x, pflInAngles.y, pflInAngles.z);

	ns_vector dir = v.Forward() + v.Right() * vSpread.x + v.Up() * vSpread.y;

	ns_vector angle = dir.Angle(&v.Up());
	angle.z += GetWavRoll(vSpread);

	pflInAngles = Vector(angle.x, angle.y, angle.z);

	/*
	Vector qViewAngles = pflInAngles;
	vSpread.x = -vSpread.x;
	vSpread.y = -vSpread.y;
	AngleVectors(qViewAngles, vForward, vRight, vUp);
	vDir.x = vForward.x + (vRight.x * vSpread.x) + (vUp.x * vSpread.y);
	vDir.y = vForward.y + (vRight.y * vSpread.x) + (vUp.y * vSpread.y);
	vDir.z = vForward.z + (vRight.z * vSpread.x) + (vUp.z * vSpread.y);
	vDir.NormalizeInPlace();
	vDir = vForward + (vRight * -vSpread.x) + (vUp * -vSpread.y);
	vDir.NormalizeInPlace();
	SpreadVectorAngles(vDir, pflInAngles);
	flIdentity[2].x = 1.0f;
	flIdentity[2].y = -vSpread.x;
	flIdentity[2].z = vSpread.y;
	flIdentity[2].NormalizeInPlace();
	flIdentity[0].x = 0.0f;
	flIdentity[0].y = -vSpread.x;
	flIdentity[0].z = (1.0f / vSpread.y) + (1.0f / flIdentity[2].z) + vSpread.y;
	if (vSpread.x > 0.0f && vSpread.y  < 0.0f)
	{
	if (flIdentity[0].y < 0.0f)
	flIdentity[0].y = -flIdentity[0].y;
	}
	else if (vSpread.x  < 0.0f && vSpread.y  < 0.0f)
	{
	if (flIdentity[0].y  > 0.0f)
	flIdentity[0].y = -flIdentity[0].y;
	}
	if (flIdentity[0].z < 0.0f)
	flIdentity[0].z = -flIdentity[0].z;
	flIdentity[0].NormalizeInPlace();
	flIdentity[1] = CrossProduct(flIdentity[0], flIdentity[2]);
	flIdentity[1].NormalizeInPlace();
	flCross = (flIdentity[1].y  * flIdentity[2].x) - (flIdentity[1].x  * flIdentity[2].y);
	if (qViewAngles.x  > 84.0f || qViewAngles.x  < -84.0f)
	flRoll = RAD2DEG(atan2f(flIdentity[1].z, sseSqrt(flCross)));
	else
	flRoll = RAD2DEG(atan2f(flIdentity[1].z, flCross));
	if (flRoll < 0.0f)
	flRoll += 360.0f;

	//pflInAngles.NormalizeAngle();
	//pflInAngles.ClampAngle();
	pflInAngles.z += flRoll;
	//pflInAngles.x = AngleNormalize(pflInAngles.x);
	//pflInAngles.y = AngleNormalize(pflInAngles.y);*/
}