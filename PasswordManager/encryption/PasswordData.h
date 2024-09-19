#pragma once
#include <StringUtils.h>
#include <base.h>
#include <vector>

struct alignas(0x10) Password_t
{
	wchar_t szTitleName[0x20] = { L'\0' };
	wchar_t szLogin[0x40] = { L'\0' };
	wchar_t szPassword[0x80] = { L'\0' };
	wchar_t szDescription[0x200] = { L'\0' };

	void SetTitleName(const std::wstring& str)
	{
		swprintf_s(szTitleName, str.c_str());
	}
	void SetTitleName(const std::string& str)
	{
		swprintf_s(szTitleName, convert::AnsiToWide(str).c_str());
	}


	void SetLogin(const std::wstring& str)
	{
		swprintf_s(szLogin, str.c_str());
	}
	void SetLogin(const std::string& str)
	{
		swprintf_s(szLogin, convert::AnsiToWide(str).c_str());
	}


	void SetPassword(const std::wstring& str)
	{
		swprintf_s(szPassword, str.c_str());
	}
	void SetPassword(const std::string& str)
	{
		swprintf_s(szPassword, convert::AnsiToWide(str).c_str());
	}


	void SetDescription(const std::wstring& str)
	{
		swprintf_s(szDescription, str.c_str());
	}
	void SetDescription(const std::string& str)
	{
		swprintf_s(szDescription, convert::AnsiToWide(str).c_str());
	}

	std::vector<unsigned char> ToByteArray()
	{
		std::vector<unsigned char> dataOut;
		dataOut.resize(sizeof(Password_t));
		int result = memcpy_s(dataOut.data(), dataOut.size() * sizeof(*dataOut.data()), (void*)(this), sizeof(Password_t));
		return dataOut;
	}

};

class CPasswordDataManager
{
private:
	std::vector<Password_t> m_passwordList;
	std::string m_key;

public:
	void SetKey(const std::string& key)
	{
		m_key = key;
	}

	std::vector<Password_t>& GetList()
	{
		return m_passwordList;
	}
	void ParseData(const std::vector<unsigned char>& data, bool* pValidSizeCheck = nullptr)
	{
		size_t elemCount = data.size() / sizeof(Password_t);
		size_t checkBack = elemCount * sizeof(Password_t);
		bool validSize = checkBack == checkBack;
		if (pValidSizeCheck)
		{
			*pValidSizeCheck = validSize;
		}

		m_passwordList.resize(elemCount);

		size_t destSize = m_passwordList.size() * sizeof(*m_passwordList.data());
		size_t srcSize = data.size() * sizeof(*data.data());
		int result = memcpy_s(m_passwordList.data(), destSize, data.data(), srcSize);
	}
	int ConvertToByteArray(std::vector<unsigned char>& dataOut)
	{
		dataOut.resize(sizeof(Password_t) * m_passwordList.size());
		size_t destSize = dataOut.size() * sizeof(*dataOut.data());
		size_t srcSize = m_passwordList.size() * sizeof(*m_passwordList.data());
		int result = memcpy_s(dataOut.data(), destSize, m_passwordList.data(), srcSize);
		return result;
	}
};