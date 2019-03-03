#pragma once

namespace Eventlog {
	void Msg(string str, bool attack);
	void Draw();
}
class IGameEvent
{
public:
	virtual ~IGameEvent() = 0;
	virtual const char *GetName() const = 0;

	virtual bool  IsReliable() const = 0;
	virtual bool  IsLocal() const = 0;
	virtual bool  IsEmpty(const char *keyName = NULL) = 0;

	virtual bool  GetBool(const char *keyName = NULL, bool defaultValue = false) = 0;
	virtual int   GetInt(const char *keyName = NULL, int defaultValue = 0) = 0;
	virtual unsigned long GetUint64(const char *keyName = NULL, unsigned long defaultValue = 0) = 0;
	virtual float GetFloat(const char *keyName = NULL, float defaultValue = 0.0f) = 0;
	virtual const char *GetString(const char *keyName = NULL, const char *defaultValue = "") = 0;
	virtual const wchar_t *GetWString(const char *keyName, const wchar_t *defaultValue = L"") = 0;

	virtual void SetBool(const char *keyName, bool value) = 0;
	virtual void SetInt(const char *keyName, int value) = 0;
	virtual void SetUint64(const char *keyName, unsigned long value) = 0;
	virtual void SetFloat(const char *keyName, float value) = 0;
	virtual void SetString(const char *keyName, const char *value) = 0;
	virtual void SetWString(const char *keyName, const wchar_t *value) = 0;
};
extern int GetServerSideID(int CBaseEntityIndex);
class IGameEventListener2
{
public:
	virtual	~IGameEventListener2(void) {};

	// FireEvent is called by EventManager if event just occured
	// KeyValue memory will be freed by manager if not needed anymore
	virtual void FireGameEvent(IGameEvent *event) = 0;
	virtual int  GetEventDebugID(void) = 0;
};

class IGameEventManager
{
public:
	bool AddListener(IGameEventListener2 *listener, const char *name, bool bServerSide)
	{
		if (!this)
			return false;

		typedef bool(__thiscall* OriginalFn)(PVOID, IGameEventListener2*, const char*, bool);
		return CallVFunction<OriginalFn>(this, 3)(this, listener, name, bServerSide);
	}

	bool FireEventClientSide(IGameEvent *event)
	{
		if (!this)
			return false;

		typedef bool(__thiscall* OriginalFn)(PVOID, IGameEvent*);
		return  CallVFunction<OriginalFn>(this, 9)(this, event);
	}
};

class cGameEvent : public IGameEventListener2
{


public:

	void FireGameEvent(IGameEvent *event);
	int GetEventDebugID();
	void RegisterSelf();
	void Register();
};

extern cGameEvent g_Event;

class trace_info
{
public:
	trace_info(Vector starts, Vector positions, float times, int userids)
	{
		this->start = starts;
		this->position = positions;
		this->time = times;
		this->userid = userids;
	}

	Vector position;
	Vector start;
	float time;
	int userid;
};

class sound_info
{
public:
	sound_info(Vector positions, float times, int userids)
	{
		this->position = positions;
		this->time = times;
		this->userid = userids;
	}

	Vector position;
	float time;
	int userid;
};


extern std::vector<trace_info> trace_logs;
extern std::vector<sound_info> sound_logs;
