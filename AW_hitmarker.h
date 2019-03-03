#pragma once
#include "sdk.h"
#include "global.h"

struct impact_info {
	float x, y, z;
	long long time;
};

struct hitmarker_info {
	impact_info impact;
	int alpha;
	int uid;
};

class CAW_hitmaker {
public:
	void paint();
};

extern CAW_hitmaker* hitmarker_2;

extern std::vector<impact_info> impacts;
extern std::vector<hitmarker_info> hitmarkers;

