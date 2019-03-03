#pragma once

#include <cstdint>
#include <cstddef>

class VMT
{
private:
	uintptr_t* vmt;
public:
	// New virtual method table
	uintptr_t** p_interface = nullptr;

	uintptr_t* original_vmt = nullptr;

	uint32_t methodCount = 0;

	VMT(void* p_interface)
	{
		this->p_interface = reinterpret_cast<uintptr_t**>(p_interface);

		size_t method_count = 0;

		while (reinterpret_cast<uintptr_t*>(*this->p_interface)[method_count])
			method_count++;

		original_vmt = *this->p_interface;

		vmt = new uintptr_t[sizeof(uintptr_t) * method_count];

		memcpy(vmt, original_vmt, sizeof(uintptr_t) * method_count);
	}

	// Hook virtual method
	void HookVM(void* method, size_t methodIndex)
	{
		vmt[methodIndex] = reinterpret_cast<uintptr_t>(method);
	}

	template< class T > T GetMethod(unsigned int nIndex) const
	{
		return (T)m_pdwOldVMT[nIndex];
	}

	template<typename Fn>
	Fn GetOriginalMethod(size_t methodIndex)
	{
		return reinterpret_cast<Fn>(original_vmt[methodIndex]);
	}

	void ApplyVMT()
	{
		*this->p_interface = vmt;
	}

	void ReleaseVMT()
	{
		*this->p_interface = original_vmt;
	}
};

#ifndef __CVMTHOOKMANAGER_H__
#define __CVMTHOOKMANAGER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Windows.h>

class CVMTHookManager
{
private:
	PDWORD*	m_ppdwClassBase;
	PDWORD	m_pdwNewVMT, m_pdwOldVMT;
	DWORD	m_dwVMTSize;

public:
	CVMTHookManager(void)
	{
		memset(this, 0, sizeof(CVMTHookManager));
	}
	CVMTHookManager(PDWORD* ppdwClassBase)
	{
		Initialize(ppdwClassBase);
	}
	~CVMTHookManager(void)
	{
		UnHook();
	}
	bool Initialize(PDWORD* ppdwClassBase)
	{
		m_ppdwClassBase = ppdwClassBase;
		m_pdwOldVMT = *ppdwClassBase;
		m_dwVMTSize = GetVMTCount(*ppdwClassBase);
		m_pdwNewVMT = new DWORD[m_dwVMTSize];
		memcpy(m_pdwNewVMT, m_pdwOldVMT, sizeof(DWORD) * m_dwVMTSize);
		*ppdwClassBase = m_pdwNewVMT;
		return true;
	}
	bool Initialize(PDWORD** pppdwClassBase)
	{
		return Initialize(*pppdwClassBase);
	}
	void UnHook(void)
	{
		if (m_ppdwClassBase)
		{
			*m_ppdwClassBase = m_pdwOldVMT;
		}
	}
	void ReHook(void)
	{
		if (m_ppdwClassBase)
		{
			*m_ppdwClassBase = m_pdwNewVMT;
		}
	}
	int GetFuncCount(void)
	{
		return (int)m_dwVMTSize;
	}
	DWORD GetMethodAddress(int Index)
	{
		if (Index >= 0 && Index <= (int)m_dwVMTSize && m_pdwOldVMT != NULL)
		{
			return m_pdwOldVMT[Index];
		}
		return NULL;
	}
	PDWORD GetOldVMT(void)
	{
		return m_pdwOldVMT;
	}
	DWORD HookMethod(DWORD dwNewFunc, unsigned int iIndex)
	{
		if (m_pdwNewVMT && m_pdwOldVMT && iIndex <= m_dwVMTSize && iIndex >= 0)
		{
			m_pdwNewVMT[iIndex] = dwNewFunc;
			return m_pdwOldVMT[iIndex];
		}
		return NULL;
	}
	template< class T > T GetMethod(unsigned int nIndex) const
	{
		return (T)m_pdwOldVMT[nIndex];
	}

private:
	DWORD GetVMTCount(PDWORD pdwVMT)
	{
		DWORD dwIndex = 0;

		while (IsBadCodePtr((FARPROC)pdwVMT[dwIndex]) == false) {
			dwIndex++;
		}
		return dwIndex;
	}
};

#endif

