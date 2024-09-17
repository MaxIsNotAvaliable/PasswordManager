#pragma once
#include <algorithm>
#include <numeric>

#include <string>
#include <sstream>

#include "math/vectors.h"

enum GuessedType : short
{
    GT_INT = 0,
    GT_BOOL,
    GT_FLOAT,
    GT_VEC,
    GT_STRING,
};

class EString : public std::string
{
public:

#pragma region Ctors
    explicit EString(const bool value) : std::string(value ? "true" : "false") {}
    explicit EString(const Vec2& value) : std::string(value) {}
    explicit EString(const Vec3& value) : std::string(value) {}
    explicit EString(const Vec4& value) : std::string(value) {}
    explicit EString(const int value) : std::string(std::to_string(value)) {}
    explicit EString(const float value) : std::string(std::to_string(value)) {}
    EString(const std::string& value) : std::string(value) {}
    EString(const char* value) : std::string(value) {}
    EString() : std::string() {}
#pragma endregion

#pragma region Data convert
    inline const int StrToInteger() const
    {
        int i;
        sscanf_s(this->c_str(), "%d", &i);
        return i;
    }

    inline const bool StrToBool() const
    {
        if (*this == EString(true))
            return true;
        else if (*this == EString(false))
            return false;

        return bool(this->StrToInteger());
    }

    inline const float StrToFloat() const
    {
        float f;
        sscanf_s(this->c_str(), "%f", &f);
        return f;
    }

    inline const void StrToFloat3(float arr[3]) const
    {
        auto delimiter1 = this->find(",");
        EString f1 = this->substr(0, delimiter1);
        EString newPart = this->substr(delimiter1 + 1, this->length());
        auto delimiter2 = newPart.find(",");
        EString f2 = this->substr(delimiter1 + 1, delimiter2);
        EString f3 = newPart.substr(delimiter2 + 1, this->length() + 1);
        arr[0] = f1.StrToFloat();
        arr[1] = f2.StrToFloat();
        arr[2] = f3.StrToFloat();
    }

    inline const std::vector<EString> StrToVec() const
    {
        std::stringstream ss(this->c_str());
        std::vector<EString> result;

        while (ss.good())
        {
            EString substr;
            std::getline(ss, substr, ',');
            result.push_back(substr);
        }

        return result;
    }

    inline const Vec2 StrToVec2() const
    {
        std::vector<EString> arr = this->StrToVec();
        if (arr.size() != 2) return Vec2();
        Vec2 retV;
        for (size_t i = 0; i < 2; i++)
        {
            retV.raw[i] = arr[i].StrToFloat();
        }
        return retV;
    }

    inline const Vec3 StrToVec3() const
    {
        std::vector<EString> arr = this->StrToVec();
        if (arr.size() != 3) return Vec3();
        Vec3 retV;
        for (size_t i = 0; i < 3; i++)
        {
            retV.raw[i] = arr[i].StrToFloat();
        }
        return retV;
    }

    inline const Vec4 StrToVec4() const
    {
        std::vector<EString> arr = this->StrToVec();
        if (arr.size() != 4) return Vec4();

        Vec4 retV;
        for (size_t i = 0; i < 4; i++)
        {
            retV.raw[i] = arr[i].StrToFloat();
        }
        return retV;
    }
#pragma endregion 

#pragma region Hasing struct

    struct LocalStringHash
    {
        std::size_t operator()(const EString& key) const
        {
            return std::hash<std::string>()(key.c_str());
        }
    };

#pragma endregion

#pragma region Some string work
    inline const EString ToLowerCase() const
    {
        EString str = *this;
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    inline const EString ToUpperCase() const
    {
        EString str = *this;
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
        return str;
    }

    inline size_t LevenstheinDistance(const EString& other)
    {
        return LevenstheinDistance(*this, other);
    }

    static size_t LevenstheinDistance(const EString& str1, const EString& str2)
    {
        const size_t lengthString1{ str1.size() };
        const size_t lengthString2{ str2.size() };

        if (lengthString1 == 0) return lengthString2;
        if (lengthString2 == 0) return lengthString1;

        std::vector<size_t> substitutionCost(lengthString2 + 1);
        std::iota(substitutionCost.begin(), substitutionCost.end(), 0);

        for (size_t indexString1{}; indexString1 < lengthString1; ++indexString1)
        {
            substitutionCost[0] = indexString1 + 1;
            size_t corner{ indexString1 };

            for (size_t indexString2{}; indexString2 < lengthString2; ++indexString2)
            {
                size_t upper{ substitutionCost[indexString2 + 1] };

                if (str1[indexString1] == str2[indexString2])
                {
                    substitutionCost[indexString2 + 1] = corner;
                }
                else
                {
                    substitutionCost[indexString2 + 1] = MIN(substitutionCost[indexString2], MAX(upper, corner)) + 1;
                }
                corner = upper;
            }
        }
        return substitutionCost[lengthString2];
    }

    inline const GuessedType GuessType() const
    {
        std::string strCpy = this->ToLowerCase();
        if (strCpy == "false" || strCpy == "true")
            return GT_BOOL;

        if (this->find_first_not_of("0123456789 .,") != std::string::npos)
            return GT_STRING;

        //int dim = std::count(this->begin(), this->end(), ',') + 1;

        if (this->find_first_of(",") != std::string::npos)
            return GT_VEC;
        //return "vec" + std::to_string(dim);

        if (this->find_first_of(".") != std::string::npos)
            return GT_FLOAT;

        return GT_INT;
    }

    inline float SimilarMin(const EString& word1, const EString& word2)
    {
        if (word1.length() < 1) return 0;
        if (word2.length() < 1) return 0;

        auto GetLowered = [](EString str) -> std::string
            {
                std::transform(str.begin(), str.end(), str.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                return str;
            };

        float distance = (float)LevenstheinDistance(GetLowered(word1), GetLowered(word2));
        float minDist = (float)MIN(word1.length(), word2.length());
        float maxDist = (float)MAX(word1.length(), word2.length());
        float deltaDist = maxDist - minDist;

        float wordLen = minDist;
        float res = 1.f - ((distance - deltaDist) / wordLen);
        return res;
    }

    inline float Similar(const EString& word1, const EString& word2)
    {
        if (word1.length() < 1) return 0;
        if (word2.length() < 1) return 0;

        auto GetLowered = [](std::string str) -> std::string
            {
                std::transform(str.begin(), str.end(), str.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                return str;
            };

        float distance = (float)LevenstheinDistance(GetLowered(word1), GetLowered(word2));
        float maxDist = (float)MAX(word1.length(), word2.length());

        float wordLen = maxDist;
        float res = 1.f - (distance / wordLen);
        return res;
    }
#pragma endregion

#pragma region Operator overload
    const char& operator[](size_t index) const
    {
        return std::string::operator[](index);
    }
    char& operator[](size_t index)
    {
        return std::string::operator[](index);
    }

    operator std::string& ()
    {
        return *reinterpret_cast<std::string*>(this);
    }
#pragma endregion

};