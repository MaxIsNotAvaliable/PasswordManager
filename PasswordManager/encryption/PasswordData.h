#pragma once
#include <StringUtils.h>
#include <base.h>
#include <files_helper/files.h>
#include <vector>
#include <functional>
#include "SHA256.h"


struct alignas(0x10) Password_t
{
	char szTitleName[0x20] = { NULL };
	char szLogin[0x40] = { NULL };
	char szPassword[0x80] = { NULL };
	char szDescription[0x200] = { NULL };

	int priority = 0;
	bool favorite = false;

	char reserved[0xD1B] = { NULL };

	void SetTitleName(const std::string& str)
	{
		sprintf_s(szTitleName, str.c_str());
	}
	std::string GetTitleName()
	{
		return szTitleName;
	}

	void SetLogin(const std::string& str)
	{
		sprintf_s(szLogin, str.c_str());
	}
	std::string GetLogin()
	{
		return szLogin;
	}

	void SetPassword(const std::string& str)
	{
		sprintf_s(szPassword, str.c_str());
	}
	std::string GetPassword()
	{
		return szPassword;
	}

	void SetDescription(const std::string& str)
	{
		sprintf_s(szDescription, str.c_str());
	}
	std::string GetDescription()
	{
		return szDescription;
	}

	bool CheckStrings()
	{
		if (strlen(szTitleName) >= (sizeof(szTitleName) / sizeof(*szTitleName)))
			return false;
		if (strlen(szLogin) >= (sizeof(szLogin) / sizeof(*szLogin)))
			return false;
		if (strlen(szPassword) >= (sizeof(szPassword) / sizeof(*szPassword)))
			return false;
		if (strlen(szDescription) >= (sizeof(szDescription) / sizeof(*szDescription)))
			return false;
		return true;
	}

	std::vector<unsigned char> ToByteArray()
	{
		std::vector<unsigned char> dataOut;
		dataOut.resize(sizeof(Password_t));
		int result = memcpy_s(dataOut.data(), dataOut.size() * sizeof(*dataOut.data()), (void*)(this), sizeof(Password_t));
		return dataOut;
	}

	bool operator<(const Password_t& other)
	{
		return strcmp(this->szTitleName, other.szTitleName) < 0; // 
	}
};

static_assert (sizeof(Password_t) == 0x1000, "Size is not correct");

class CPasswordDataManager
{
private:
	std::vector<Password_t> m_passwordList;
	std::string m_key;
	std::string m_initFileHash;
	std::string m_filename;

	static inline std::string m_fileFolder = manage_files::GetFolder() + "\\passwords\\";

	std::vector<unsigned char> GeyKeyData()
	{
		std::string hashedKey = GetKeyHash();

		std::vector<unsigned char> keyArr(hashedKey.size());

		memcpy_s(keyArr.data(), keyArr.size(), hashedKey.data(), hashedKey.size());
		return keyArr;
	}

public:
	bool CheckForStrings()
	{
		for (size_t i = 0; i < m_passwordList.size(); i++)
		{
			if (!m_passwordList[i].CheckStrings())
				return false;
		}
		return true;
	}

	std::string GetKeyHash()
	{
		SHA256 sha;
		sha.update(m_key);
		std::array<uint8_t, 32> digest = sha.digest();
		std::string result = SHA256::toString(digest);
		return result;
	}

	void SetKey(const std::string& key)
	{
		m_key = key;
		while (m_key.size() < 0x20)
		{
			size_t hash = std::hash<std::string>{}(m_key);
			m_key += std::to_string(hash);
		}
		if (m_key.size() > 0x20)
		{
			m_key.erase(0x20);
		}
	}
	void SetFilename(const std::string& filename)
	{
		m_filename = filename;
		if (!m_filename.ends_with(".enc"))
			m_filename += ".enc";
	}
	void ClearData()
	{
		this->m_filename = "\0";
		this->m_initFileHash = "\0";
		this->m_passwordList.clear();
	}
	std::string GetFilename()
	{
		return m_filename;
	}
	bool DecryptAndOpenData()
	{
		std::vector<unsigned char> keyArr = GeyKeyData();

		std::vector<unsigned char> fileDataBuffer;
		if (!OpenFileEncrypt(fileDataBuffer, m_fileFolder + m_filename, keyArr))
			return false;

		ParseData(fileDataBuffer);
		m_initFileHash = GetOpenFileHash();

		return true;
	}

	std::string GetOpenFileHash()
	{
		SHA256 sha;
		sha.update((uint8_t*)m_passwordList.data(), m_passwordList.size() * sizeof(Password_t));
		return SHA256::toString(sha.digest());
	}

	bool IsFileUpdated()
	{
		return m_initFileHash != GetOpenFileHash();
	}

	bool EncryptAndSaveData()
	{
		std::vector<unsigned char> keyArr = GeyKeyData();
		std::vector<unsigned char> fileDataBuffer;
		ConvertToByteArray(fileDataBuffer);
		m_initFileHash = GetOpenFileHash();
		return SaveFileEncrypt(fileDataBuffer, m_fileFolder + m_filename, keyArr);
	}

	std::vector<Password_t>& GetList()
	{
		//std::sort(m_passwordList.begin(), m_passwordList.end());

		return m_passwordList;
	}

	const std::vector<Password_t> GetList() const
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


namespace std {
	template <>
	struct hash<Password_t>
	{
		std::size_t operator()(const Password_t& s) const
		{
			std::size_t h = 0;
			for (const auto& c : s.szTitleName)
			{
				h ^= std::hash<char>()(c);
			}
			for (const auto& c : s.szLogin)
			{
				h ^= std::hash<char>()(c);
			}
			for (const auto& c : s.szPassword)
			{
				h ^= std::hash<char>()(c);
			}
			for (const auto& c : s.szDescription)
			{
				h ^= std::hash<char>()(c);
			}
			return h;
		}
	};

	template<>
	struct hash<CPasswordDataManager>
	{
		std::size_t operator()(const CPasswordDataManager& s) const
		{
			std::size_t h = 0;
			for (const auto& item : s.GetList())
			{
				h ^= std::hash<Password_t>()(item);
			}
			return h;
		}
	};
}

//struct VectorHasher {
//
//	std::size_t operator()(const std::vector<Password_t>& vec) const 
//	{
//		std::size_t h = 0;
//		for (const auto& item : vec) 
//		{
//			h ^= std::hash<Password_t>()(item);
//		}
//		return h;
//	}
//};

