#include <functional>
#include <cmath>
#include <vector>
#include <thread>

#include "matrix.hpp"

Matrix::Matrix() : matrix_(nullptr),
                   height_(0),
                   width_(0) {}

Matrix::Matrix(const Matrix &other) : Matrix(other.matrix_, other.height_, other.width_) {}

Matrix::Matrix(Matrix &&other) : matrix_(other.matrix_),
                                 height_(other.height_),
                                 width_(other.width_)
{

    other.matrix_ = nullptr;
    other.width_ = 0;
    other.height_ = 0;
}

Matrix::Matrix(size_t height, size_t width) : matrix_(new matrix_t[height * width]),
                                              height_(height),
                                              width_(width)
{

    for (size_t i = 0; i < height_ * width_; i++)
    {
        matrix_[i] = 0;
    }
}

Matrix::Matrix(matrix_t *values, size_t height, size_t width) : matrix_(new matrix_t[height * width]),
                                                                height_(height),
                                                                width_(width)
{

    for (size_t i = 0; i < height_ * width_; i++)
    {
        matrix_[i] = values[i];
    }
}

size_t Matrix::getHeight() const
{
    return height_;
}

size_t Matrix::getWidth() const
{
    return width_;
}

void Matrix::fillZeros()
{
    for (size_t i = 0; i < width_ * height_; ++i)
        matrix_[i] = 0;
}

void Matrix::serialFill()
{
    for (size_t i = 0; i < width_ * height_; ++i)
        matrix_[i] = i;
}

void Matrix::fill(matrix_t *array)
{
    for (size_t i = 0; i < height_ * width_; ++i)
        matrix_[i] = array[i];
}

void Matrix::setRow(size_t row, matrix_t *array)
{
    matrix_t *this_row = (*this)[row];
    for (size_t i = 0; i < width_; ++i)
        this_row[i] = array[i];
}

matrix_t *Matrix::operator[](int i)
{
    return matrix_ + width_ * i;
}

const matrix_t *Matrix::operator[](int i) const
{
    return matrix_ + width_ * i;
}

const Matrix &Matrix::operator=(const Matrix &other)
{
    if (width_ != other.width_ || height_ != other.height_)
    {
        delete[] matrix_;
        width_ = other.width_;
        height_ = other.height_;
        matrix_ = new matrix_t[height_ * width_];
    }

    for (size_t i = 0; i < width_ * height_; ++i)
    {
        matrix_[i] = other.matrix_[i];
    }

    return *this;
}

const Matrix &Matrix::operator=(Matrix &&other)
{
    new (this) Matrix(std::move(other));
    return *this;
}

bool Matrix::operator==(const Matrix &other) const
{
    if (width_ != other.width_ || height_ != other.height_)
        return false;

    for (size_t i = 0; i < width_ * height_; i++)
        if (matrix_[i] != other.matrix_[i])
            return false;

    return true;
}

Matrix Matrix::operator*(const Matrix &other) const
{
    return this->improvedMul(other);
}

Matrix Matrix::operator+(const Matrix &other) const
{
    if (width_ != other.width_ || height_ != other.height_)
    {
        std::cout << __PRETTY_FUNCTION__ << ": sizes are not equal: (" << height_ << " x " << width_ << ") and (" << other.height_ << " x " << other.width_ << ")" << std::endl;

        return Matrix();
    }

    Matrix res = *this;

    return res += other;
}

const Matrix &Matrix::operator+=(const Matrix &other)
{
    if (width_ != other.width_ || height_ != other.height_)
    {
        std::cout << __PRETTY_FUNCTION__ << ": sizes are not equal: (" << height_ << " x " << width_ << ") and (" << other.height_ << " x " << other.width_ << ")" << std::endl;

        return *this;
    }

    for (size_t i = 0; i < width_ * height_; ++i)
    {
        matrix_[i] += other.matrix_[i];
    }

    return *this;
}

// First simple multiplication realization
Matrix Matrix::simpleMul(const Matrix &other) const
{
    if (width_ != other.getHeight())
    {
        std::cout << __PRETTY_FUNCTION__ << ": sizes are not equal: (" << height_ << " x " << width_ << ") and (" << other.height_ << " x " << other.width_ << ")" << std::endl;
        return Matrix();
    }

    Matrix result(height_, other.getWidth());

    for (size_t row = 0; row < height_; ++row)
    {
        for (size_t other_column = 0; other_column < other.getWidth(); ++other_column)
        {
            for (size_t column = 0; column < width_; ++column)
            {
                result[row][other_column] += (*this)[row][column] * other[column][other_column];
            }
        }
    }

    return result;
}

// Second improved multiplication realization
Matrix Matrix::improvedMul(const Matrix &other) const
{
    if (width_ != other.getHeight())
    {
        std::cout << __PRETTY_FUNCTION__ << ": sizes are not equal: (" << height_ << " x " << width_ << ") and (" << other.height_ << " x " << other.width_ << ")" << std::endl;
        return Matrix();
    }

    Matrix result(height_, other.getWidth());

    for (size_t row = 0; row < height_; ++row)
    {
        matrix_t *result_row = result[row];
        const matrix_t *this_row = (*this)[row];

        for (size_t column = 0; column < width_; ++column)
        {
            const matrix_t this_row_column = this_row[column];
            const matrix_t *other_row = other[column];

            for (size_t other_column = 0; other_column < other.getWidth(); ++other_column)
            {
                result_row[other_column] += this_row_column * other_row[other_column];
            }
        }
    }

    return result;
}

void threadMul(Matrix &result_matrix, size_t start_row, size_t finish_row, const Matrix &this_matrix, const Matrix &other_matrix)
{
    for (size_t row = start_row; row < finish_row; ++row)
    {
        matrix_t *result_row = result_matrix[row];
        const matrix_t *this_row = this_matrix[row];

        for (size_t column = 0; column < this_matrix.width_; ++column)
        {
            const matrix_t this_row_column = this_row[column];
            const matrix_t *other_row = other_matrix[column];

            for (size_t other_column = 0; other_column < other_matrix.getWidth(); ++other_column)
            {
                result_row[other_column] += this_row_column * other_row[other_column];
            }
        }
    }
}

// Multiprocessing multiplication realization
Matrix Matrix::multiThreadMul(const Matrix &other, size_t num_of_threads) const
{
    if (width_ != other.getHeight())
        return Matrix();

    Matrix result(height_, other.getWidth());

    std::vector<std::thread> threads(num_of_threads);

    for (size_t i = 0; i < num_of_threads; ++i)
        threads[i] = std::thread(threadMul,
                                 std::ref(result),
                                 i * this->height_ / num_of_threads,
                                 (i + 1) * this->height_ / num_of_threads,
                                 std::cref(*this),
                                 std::cref(other));

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        if (threads[i].joinable())
            threads[i].join();
    }

    return result;
}

Matrix::~Matrix()
{
    height_ = 0;
    width_ = 0;
    delete[] matrix_;
}

std::ostream &operator<<(std::ostream &os, const Matrix &m)
{
    for (size_t i = 0; i < m.height_; ++i)
    {
        for (size_t j = 0; j < m.width_; ++j)
        {
            os << m.matrix_[i * m.width_ + j] << " ";
        }
        os << std::endl;
    }

    return os;
}
