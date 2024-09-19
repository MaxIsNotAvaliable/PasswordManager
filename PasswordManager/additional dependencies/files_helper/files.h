#pragma once
#include <iostream>
#include <fstream>
#include <win.h>
#include <future>

#include <filesystem>
#pragma comment(lib, "Shell32.lib")

namespace manage_files
{
	static std::string GetAbsFileName()
	{
		char pBuf[MAX_PATH] = {0};
		GetModuleFileNameA(NULL, pBuf, sizeof(pBuf));
		return pBuf;
	}

	static std::string GetFolder()
	{
		char pBuf[MAX_PATH];
		GetModuleFileNameA(NULL, pBuf, sizeof(pBuf));

		size_t lastSlashPos = std::string(pBuf).find_last_of('\\', sizeof(pBuf));

		if (lastSlashPos != std::string::npos)
			pBuf[lastSlashPos] = '\0';
		return pBuf;
	}

	static std::vector<std::string> GetFoldersInDirectory(const std::string& path)
	{
		std::vector<std::string> folders;
		for (const auto& entry : std::filesystem::directory_iterator(path)) 
		{
			if (std::filesystem::is_directory(entry.status())) 
			{
				folders.push_back(entry.path().string());
			}
		}
		return folders;
	}

	static std::vector<std::string> GetFilesInDirectory(const std::string& path, bool onlyFileName = false)
	{
		std::vector<std::string> files;
		for (const auto& entry : std::filesystem::directory_iterator(path)) 
		{
			files.push_back((onlyFileName ? entry.path().filename() : entry.path()).string());
		}
		return files;
	}


	static bool fileExists(const std::string& filename)
	{
		struct stat buf;
		return (stat(filename.c_str(), &buf) != -1);
	}

	static bool folderExists(const std::string& filename)
	{
		struct stat buf;
		return (stat(filename.c_str(), &buf) == 0);
	}

	static bool IsExists(const std::string& path)
	{
		return fileExists(path) || folderExists(path);
	}

	static void createDirectory(const std::string& path)
	{
		if (folderExists(path)) return;
		std::filesystem::create_directory(path);
	}

	static void openDir(const std::string& path)
	{
		ShellExecuteA(NULL, NULL, path.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	static bool CraftFile(const std::string& fileName, const std::vector<char>& fileData)
	{
		std::ofstream outputFile;
		outputFile.open(fileName, std::ios::binary);
		if (!outputFile.is_open())
			return false;

		outputFile.write(fileData.data(), fileData.size());
		outputFile.close();

		return true;
	}

	static bool ReadFile(const std::string& filename, std::string & out) {
		if (!fileExists(filename))
			return false;

		std::ifstream ifs(filename);

		std::stringstream buffer;
		buffer << ifs.rdbuf();
		out = buffer.str();

		ifs.close();
		return true;
	}

	static std::string ReadFirstLine(const std::string& filename)
	{
		std::ifstream file(filename);
		std::string line;
		if (file.is_open()) {
			if (getline(file, line)) {
			}
			file.close();
		}
		return line;
	}

	static bool DestroyFile(const std::string& fileName)
	{
		return remove(fileName.c_str()) == 0;
	}
};