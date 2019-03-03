class CClient
{
public:
	ClientClass* GetAllClasses()
	{
		typedef ClientClass* (__thiscall* OriginalFn)(PVOID); //Anything inside a VTable is a __thiscall unless it completly disregards the thisptr. You can also call them as __stdcalls, but you won't have access to the __thisptr.
		return CallVFunction<OriginalFn>(this, 8)(this); //Return the pointer to the head CClientClass.
	}
};