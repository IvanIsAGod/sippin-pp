#pragma once



class CNetVars
{
public:
	CNetVars(void);
	int GetOffset(const char *tableName, const char *propName);
	bool HookProp(const char *tableName, const char *propName, RecvVarProxyFn fun);
	RecvTable *GetTable(const char *tableName);
	std::vector<RecvTable*>    m_tables;
	void DumpTable(RecvTable *table, bool log);
	void DumpNetvars(bool log);

private:
	int GetProp(const char *tableName, const char *propName, RecvProp **prop = 0);
	int GetProp(RecvTable *recvTable, const char *propName, RecvProp **prop = 0);
	
};
#define GET_NETVAR g_pNetvars->GetOffset
extern CNetVars *g_pNetVars;