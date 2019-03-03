#pragma once
class CMaterialHelper
{
public:
	IMaterial* CoveredLit = nullptr;
	IMaterial* OpenLit = nullptr;
	IMaterial* CoveredFlat = nullptr;
	IMaterial* OpenFlat = nullptr;
	IMaterial* Wire = nullptr;
	IMaterial* AddWire = nullptr;
	CMaterialHelper();
	void ForceMaterial(Color color, IMaterial* material, bool useColor = true, bool forceMaterial = true);
	void GameFunc_LoadFromBuffer(KeyValues * vk_, std::string name_, std::string buffer_);
	IMaterial *CreateMaterial(bool shouldIgnoreZ, bool isLit = true, bool isWireframe = false);
	void GameFunc_InitKeyValues(KeyValues* pKeyValues, const char* name);
};
extern CMaterialHelper* g_MaterialHelper;