class CPanel
{
public:
	const char* GetName( unsigned int Panel )
	{
		typedef const char*(__thiscall* Fn)(void*, unsigned int);
		return CallVFunction<Fn>(this, 36)(this, Panel );
	}

};

class IStudioRender
{
public:
	virtual void Unkonwn(void) = 0;
	virtual void Unkonwn1(void) = 0;
	virtual void Unkonwn2(void) = 0;
	virtual void Unkonwn3(void) = 0;
	virtual void Unkonwn4(void) = 0;
	virtual void Unkonwn5(void) = 0;
	virtual void Unkonwn6(void) = 0;
	virtual void Unkonwn7(void) = 0;

	virtual void BeginFrame(void) = 0; // 9
	virtual void EndFrame(void) = 0;
};