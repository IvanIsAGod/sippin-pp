#include "sdk.h"
#include <stdio.h>
#include "xor.h"
CNetVars *g_pNetVars = 0;

CNetVars::CNetVars(void)
{
	m_tables.clear();

	ClientClass *clientClass = g_pClient->GetAllClasses();
	if (!clientClass)
		return;

	while (clientClass)
	{
		RecvTable *recvTable = clientClass->m_pRecvTable;
		m_tables.push_back(recvTable);

		clientClass = clientClass->m_pNext;
	}
}

void LogNet(const char* szString)
{
	FILE* stream;

	errno_t pFile = fopen_s(&stream, XorStr("NetVarDump.txt"), XorStr("a"));
	fprintf(stream, szString);
	fclose(stream);
}

// calls GetProp wrapper to get the absolute offset of the prop
int CNetVars::GetOffset(const char *tableName, const char *propName)
{
	int offset = this->GetProp(tableName, propName);
	if (!offset)
	{
		//g_pCVar->ConsoleColorPrintf(Color::Red(), "%s not found!\n", propName);
		Msg(XorStr("%s not found!\n"), propName);
		return 0;
	}
	//g_pCVar->ConsoleColorPrintf(Color::Green(), "%s: 0x%02X\n", propName, offset);
	//Msg(XorStr("%s: 0x%02X\n"), propName, offset);
	return offset;
}


// calls GetProp wrapper to get prop and sets the proxy of the prop
bool CNetVars::HookProp(const char *tableName, const char *propName, RecvVarProxyFn fun)
{
	RecvProp *recvProp = 0;
	this->GetProp(tableName, propName, &recvProp);
	if (!recvProp)
		return false;

	recvProp->m_ProxyFn = fun;

	return true;
}


// wrapper so we can use recursion without too much performance loss
int CNetVars::GetProp(const char *tableName, const char *propName, RecvProp **prop)
{
	RecvTable *recvTable = GetTable(tableName);
	if (!recvTable)
		return 0;

	int offset = this->GetProp(recvTable, propName, prop);
	if (!offset)
		return 0;

	return offset;
}


// uses recursion to return a the relative offset to the given prop and sets the prop param
int CNetVars::GetProp(RecvTable *recvTable, const char *propName, RecvProp **prop)
{
	int extraOffset = 0;
	for (int i = 0; i < recvTable->m_nProps; ++i)
	{
		RecvProp *recvProp = &recvTable->m_pProps[i];
		RecvTable *child = recvProp->m_pDataTable;

		if (child && (child->m_nProps > 0))
		{
			int tmp = this->GetProp(child, propName, prop);
			if (tmp)
				extraOffset += (recvProp->m_Offset + tmp);
		}

		if (_stricmp(recvProp->m_pVarName, propName))
			continue;

		if (prop)
			*prop = recvProp;

		return (recvProp->m_Offset + extraOffset);
	}

	return extraOffset;
}


RecvTable *CNetVars::GetTable(const char *tableName)
{
	if (m_tables.empty())
		return 0;

	for each (RecvTable *table in m_tables)
	{
		if (!table)
			continue;

		if (_stricmp(table->m_pNetTableName, tableName) == 0)
			return table;
	}

	return 0;
}

void CNetVars::DumpTable(RecvTable *table, bool log)
{

}

void CNetVars::DumpNetvars(bool log)
{

}
