#pragma once
#include "sdk.h"
#include "global.h"
#include "Draw.h"
extern int showtime;
extern string prefix;


class CMessage
{
public:
	CMessage(string str, float time, Color clr, int a)
	{
		this->str = str;
		this->time = time;
		this->clr = clr;
		alpha = a;
	}
	std::string str;
	float time;
	Color clr;
	int alpha;
};


namespace Eventlog
{
	extern std::vector <CMessage> messages;
	extern 	void Msg(string str, bool red);

	extern void Draw();
}

