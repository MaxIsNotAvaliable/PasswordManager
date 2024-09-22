#pragma once
#include "AES.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include "SHA256.h"

static bool SaveFileEncrypt(std::vector<unsigned char> buffer, const std::string& outputFilePath, const std::vector<unsigned char>& key)
{
    size_t originalSize = buffer.size();
    size_t paddedSize = ((originalSize + 15) / 16) * 16;
    buffer.resize(paddedSize);

    AES aes(AESKeyLength::AES_256);

    buffer = aes.EncryptECB(buffer, key);

    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile) 
    {
        return false;
    }

    outputFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    outputFile.close();
    return true;
}

static bool OpenFileEncrypt(std::vector<unsigned char>& outBuffer, const std::string& inputFilePath, const std::vector<unsigned char>& key)
{
    std::ifstream outputFile(inputFilePath, std::ios::binary);
    if (!outputFile)
    {
        return false;
    }

    size_t originalSize = std::filesystem::file_size(inputFilePath);
    size_t paddedSize = ((originalSize + 15) / 16) * 16;
    outBuffer.resize(paddedSize);

    outputFile.read(reinterpret_cast<char*>(outBuffer.data()), outBuffer.size());
    outputFile.close();

    AES aes(AESKeyLength::AES_256);
    outBuffer = aes.DecryptECB(outBuffer, key);
    return true;
}
