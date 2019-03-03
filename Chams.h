#pragma once
#include "Singleton.h"
#include "hooks.h"
#include "Menu.h"
#include "global.h"
#include "MaterialHelper.h"
#include "xor.h"
class Chams
	: public Singleton<Chams>
{
	friend class Singleton<Chams>;

	~Chams();

public:
	void CreateMaterialChams();
};