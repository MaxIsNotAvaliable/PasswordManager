#pragma once

#include <cmath>
#include <algorithm>

#include <vector>
#include <stdexcept>

typedef std::vector<std::vector<float>> vector_matrix_t;

class Matrix 
{
private:
    vector_matrix_t data; // [row][col]

    bool DimensionCheck(const vector_matrix_t& matrix)
    {
        for (size_t i = 0; i < matrix.size() - 1; i++)
        {
            if (matrix[i].size() != matrix[i + 1].size())
                return false;
        }
        return true;
    }

public:
    /// <summary>
    /// Конструктор для создания матриц размером rows x cols
    /// </summary>
    /// <param name="rows"> - Количество строк</param>
    /// <param name="cols"> - Количество колонок</param>
    Matrix(size_t rows, size_t cols)
    { 
        data = vector_matrix_t(rows, std::vector<float>(cols));
    }

    /// <summary>
    /// Конструктор для создания матриц на базе структуры vector_matrix_t
    /// </summary>
    /// <param name="matrix"> - Созданная матрица в двумерном массиве</param>
    Matrix(const vector_matrix_t& matrix)
    { 
        if (!DimensionCheck(matrix))
            throw std::invalid_argument("Matrix dimensions do not match.");
        data = matrix;
    }

    /// <summary>
    /// Конструктор для создания матриц на основе одномерного массива, содержащего все элементы матрицы
    /// </summary>
    /// <param name="matrix"> - Массив с данными</param>
    /// <param name="rows"> - Количество строк</param>
    /// <param name="cols"> - Количество колонок</param>
    Matrix(const std::vector<float>& matrix, size_t rows, size_t cols)
    {
        if (rows * cols != matrix.size())
            throw std::invalid_argument("Matrix dimensions do not match.");

        vector_matrix_t vecMatrix;
        for (size_t i = 0; i < rows; i++)
        {
            std::vector<float> row;
            for (size_t j = 0; j < cols; j++)
            {
                row.push_back(matrix[i * rows + cols]);
            }
            vecMatrix.push_back(row);
        }
        data = vecMatrix;
    }

    /// <summary>
    /// Конструктор для создания матриц на основе одномерного массива, содержащего все элементы матрицы
    /// </summary>
    /// <param name="matrix"> - Массив с данными</param>
    /// <param name="size"> - Длинна массива</param>
    /// <param name="rows"> - Количество строк</param>
    /// <param name="cols"> - Количество колонок</param>
    Matrix(const float* matrix, size_t size, size_t rows, size_t cols)
    {
        if (rows * cols != size)
            throw std::invalid_argument("Matrix dimensions do not match.");

        vector_matrix_t vecMatrix;
        for (size_t i = 0; i < rows; i++)
        {
            std::vector<float> row;
            for (size_t j = 0; j < cols; j++)
            {
                row.push_back(matrix[i * rows + cols]);
            }
            vecMatrix.push_back(row);
        }
        data = vecMatrix;
    }


    ~Matrix(void) { }

    /// <summary>
    /// Возвращает количество строк в матрице
    /// </summary>
    /// <returns>Количество строк в матрице</returns>
    size_t Rows(void) const { return data.size(); }

    /// <summary>
    /// Возвращает количество колонок в матрице
    /// </summary>
    /// <returns>Количество колонок в матрице</returns>
    size_t Cols(void) const { return data.front().size(); }

    std::vector<float>& operator[](size_t i)
    { 
        return data[i]; 
    }
    const std::vector<float>& operator[](size_t i) const
    { 
        return data[i];
    }

    /// <summary>
    /// Возвращает элемент на позиции matrix[i][j]
    /// </summary>
    /// <param name="i"> - номер строки</param>
    /// <param name="j"> - номер колонки</param>
    /// <returns></returns>
    float& Get(size_t i, size_t j)
    {
        return (*this)[i][j];
    }

    /// <summary>
    /// Возвращает элемент на позиции matrix[i][j]
    /// </summary>
    /// <param name="i"> - номер строки</param>
    /// <param name="j"> - номер колонки</param>
    /// <returns></returns>
    const float& Get(size_t i, size_t j) const
    { 
        return (*this)[i][j]; 
    }

    Matrix operator=(const vector_matrix_t&matrix)
    {
        data.clear();
        data = matrix;
        return *this;
    }

    Matrix operator*(const Matrix& mat) const 
    {
        if (this->Cols() != mat.Rows()) 
            throw std::invalid_argument("Matrix dimensions do not match.");
        

        Matrix result(this->Cols(), mat.Cols());

        for (size_t i = 0; i < this->Cols(); ++i) {
            for (size_t j = 0; j < mat.Cols(); ++j) {
                float sum = 0.0;
                for (size_t k = 0; k < this->Cols(); ++k) {
                    sum += this->Get(i, k) * mat[k][j];
                }
                result[i][j] = sum;
            }
        }

        return result;
    }

    Matrix operator*(float mul) const
    {
        Matrix matrix = *this;
        for (size_t i = 0; i < this->Rows(); i++)
        {
            for (size_t j = 0; j < this->Cols(); j++)
            {
                matrix[i][j] *= mul;
            }
        }
        return matrix;
    }

    //Matrix operator*(const Vec3 &vec) const
    //{
    //    return *this * vec.ToMatrix();
    //}

    /// <summary>
    /// Получение детерминанта (определителя)
    /// </summary>
    /// <returns></returns>
    float GetDeterminant() {
        size_t dimension = this->Cols();

        if (this->Cols() != this->Rows())
            throw std::runtime_error("Matrix is not quadratic.");
        
        if (dimension == 0) 
            return 0;
        
        if (dimension == 1) 
            return this->Get(0, 0);
        
        if (dimension == 2) 
            return this->Get(0, 0) * this->Get(1, 1) - this->Get(0, 1) * this->Get(1, 0);
        
        float det = 0.0;
        for (size_t i = 0; i < dimension; ++i)
        {
            Matrix submatrix = Matrix(dimension - 1, dimension - 1);
            for (size_t j = 1; j < dimension; ++j)
            {
                for (size_t k = 0; k < dimension; ++k)
                {
                    if (k < i) 
                        submatrix.Get(j - 1, k) = this->Get(j, k);
                    else if (k > i) 
                        submatrix.Get(j - 1, k - 1) = this->Get(j, k);
                }
            }
            det += powf(-1.0f, (float)i) * this->Get(0, i) * submatrix.GetDeterminant();
        }
        return det;
    }

    /// <summary>
    /// Выполняет скалярное произведение (dot product / scalar product)
    /// </summary>
    /// <param name="mat"> - другая матрица с которой будет выполнено скалярное произведение</param>
    /// <returns></returns>
    Matrix DotProduct(const Matrix& mat) const 
    {
        return *this * mat;
    }

    
    
};

// 2x2 matrix
static const Matrix matrixIdentity2d = vector_matrix_t{
    {1, 0}, 
    {0, 1},
};

// 3x3 matrix
static const Matrix matrixIdentity3d = vector_matrix_t{
    {1, 0, 0}, 
    {0, 1, 0},
    {0, 0, 1},
};

// 4x4 matrix
static const Matrix matrixIdentity4d = vector_matrix_t{
    {1, 0, 0, 0}, 
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
};