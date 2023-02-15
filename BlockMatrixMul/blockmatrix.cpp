#include <functional>
#include <vector>
#include <thread>
#include <stdexcept>

#include "blockmatrix.hpp"

BlockMatrix::BlockMatrix() : matrices_(nullptr),
                             height_(0),
                             width_(0),
                             internal_matrix_size_(0) {}

BlockMatrix::BlockMatrix(const BlockMatrix &other) : matrices_(new Matrix *[other.width_ * other.height_]),
                                                     height_(other.height_),
                                                     width_(other.width_),
                                                     internal_matrix_size_(other.internal_matrix_size_)
{

    for (size_t i = 0; i < height_ * width_; ++i)
    {
        matrices_[i] = new Matrix(const_cast<const Matrix &>(*other.matrices_[i]));
    }
}

static matrix_t *support_array_for_constructor_ = nullptr;
BlockMatrix::BlockMatrix(size_t height, size_t width, size_t internal_matrix_size) : BlockMatrix(support_array_for_constructor_ = new matrix_t[height * width] /*(matrix_t*)std::calloc(height * width, sizeof(matrix_t))*/, height, width, internal_matrix_size)
{
    delete[] support_array_for_constructor_;
    support_array_for_constructor_ = nullptr;
}

BlockMatrix::BlockMatrix(matrix_t *values, size_t height, size_t width, size_t internal_matrix_size) : internal_matrix_size_(internal_matrix_size)
{

    size_t addition_matrix_right = (width % internal_matrix_size != 0);
    size_t addition_matrix_bot = (height % internal_matrix_size != 0);

    width_ = width / internal_matrix_size + addition_matrix_right;
    height_ = height / internal_matrix_size + addition_matrix_bot;

    matrices_ = new Matrix *[width_ * height_];

    size_t last_columns_counter = 0;

    for (size_t i = 0; i < (height_ - addition_matrix_bot) * width_; ++i)
    {
        if (addition_matrix_right && i % width_ == width_ - 1)
        { // for last column
            matrices_[i] = new Matrix(internal_matrix_size, width % internal_matrix_size);
            last_columns_counter++;
        }
        else
        {
            matrices_[i] = new Matrix(internal_matrix_size, internal_matrix_size);
        }

        for (size_t j = 0; j < internal_matrix_size; ++j)
        {
            matrices_[i]->setRow(j, values + (i / width_) * width * internal_matrix_size + j * width + (i % width_) * internal_matrix_size);
        }
    }

    // For last row
    for (size_t i = (height_ - addition_matrix_bot) * width_; i < height_ * width_; ++i)
    {
        if (addition_matrix_right && i % width_ == width_ - 1)
        { // for last column
            matrices_[i] = new Matrix(height % internal_matrix_size, width % internal_matrix_size);
        }
        else
        {
            matrices_[i] = new Matrix(height % internal_matrix_size, internal_matrix_size);
        }

        for (size_t j = 0; j < matrices_[i]->getHeight(); ++j)
        {
            matrices_[i]->setRow(j, values + (i / width_) * width * internal_matrix_size + j * width + (i % width_) * internal_matrix_size);
        }
    }
}

void BlockMatrix::fillZeros()
{
    for (size_t i = 0; i < width_ * height_; ++i)
        matrices_[i]->fillZeros();
}

void BlockMatrix::serialFill()
{
    size_t dataWidth = this->getDataWidth();

    for (size_t block_row = 0; block_row < height_; ++block_row)
    {
        Matrix **this_row = (*this)[block_row];

        for (size_t block_col = 0; block_col < width_; ++block_col)
        {
            Matrix *this_matrix = this_row[block_col];

            for (size_t matrix_row = 0; matrix_row < this_matrix->getHeight(); ++matrix_row)
            {
                for (size_t matrix_col = 0; matrix_col < this_matrix->getWidth(); ++matrix_col)
                {
                    (*this_matrix)[matrix_row][matrix_col] = (block_row * internal_matrix_size_ + matrix_row) * dataWidth + (block_col * internal_matrix_size_ + matrix_col);
                }
            }
        }
    }
}

size_t BlockMatrix::getDataWidth() const
{
    size_t counter = 0;

    if (height_ > 0)
    {
        for (size_t i = 0; i < width_; ++i)
        {
            counter += matrices_[i]->getWidth();
        }
    }

    return counter;
}

size_t BlockMatrix::getDataHeight() const
{
    size_t counter = 0;

    if (width_ > 0)
    {
        for (size_t i = 0; i < height_; ++i)
        {
            counter += matrices_[i * width_]->getHeight();
        }
    }

    return counter;
}

const BlockMatrix &BlockMatrix::operator=(const BlockMatrix &other)
{
    new (this) BlockMatrix(other);
    return *this;
}

BlockMatrix BlockMatrix::operator*(const BlockMatrix &other) const
{
    if (width_ != other.height_ || internal_matrix_size_ != other.internal_matrix_size_)
    {
        std::cerr << __PRETTY_FUNCTION__ << ": error, matrices ca not be multiplied" << std::endl;
        return BlockMatrix();
    }

    BlockMatrix result(this->getDataHeight(), other.getDataWidth(), internal_matrix_size_);
    result.fillZeros();

    for (size_t row = 0; row < height_; ++row)
    {
        Matrix **result_row = result[row];
        const Matrix **this_row = (*this)[row];

        for (size_t column = 0; column < width_; ++column)
        {
            const Matrix &this_row_column = *this_row[column];
            const Matrix **other_row = other[column];

            for (size_t other_column = 0; other_column < other.width_; ++other_column)
            {
                (*result_row[other_column]) += this_row_column * (*other_row[other_column]);
            }
        }
    }

    return result;
}

bool BlockMatrix::operator==(const Matrix &other)
{
    size_t dataHeight = this->getDataHeight();
    size_t dataWidth = this->getDataWidth();

    if (dataHeight != other.getHeight() || dataWidth != other.getWidth())
        return false;

    for (size_t block_row = 0; block_row < height_; ++block_row)
    {
        Matrix **this_row = (*this)[block_row];

        for (size_t block_col = 0; block_col < width_; ++block_col)
        {
            Matrix *this_matrix = this_row[block_col];

            for (size_t i = 0; i < this_matrix->getHeight(); ++i)
            {
                size_t other_row_num = block_row * internal_matrix_size_ + i;
                const matrix_t *other_row = other[other_row_num];

                for (size_t j = 0; j < this_matrix->getWidth(); ++j)
                {
                    size_t other_col = block_col * internal_matrix_size_ + j;

                    if ((*this_matrix)[i][j] != other_row[other_col])
                        return false;
                }
            }
        }
    }

    return true;
}

void threadMul(Matrix **result_matrix, size_t start_row, size_t finish_row, const BlockMatrix &this_matrix, const BlockMatrix &other_matrix)
{
    char buff[128];
    std::sprintf(buff, "%ld %ld: I do it!\n", start_row, finish_row);
    std::cout << buff << std::flush;

    for (size_t row = start_row; row < finish_row; ++row)
    {
        Matrix **result_row = result_matrix + row * other_matrix.width_;
        const Matrix **this_row = this_matrix[row];

        for (size_t column = 0; column < this_matrix.width_; ++column)
        {
            const Matrix &this_row_column = *(this_row[column]);
            const Matrix **other_row = other_matrix[column];

            for (size_t other_column = 0; other_column < other_matrix.width_; ++other_column)
            {
                (*result_row[other_column]) += this_row_column.improvedMul(*other_row[other_column]);
            }
        }
    }
}

BlockMatrix BlockMatrix::multiThreadMul(const BlockMatrix &other, size_t num_of_threads) const
{
    if (width_ != other.height_ || internal_matrix_size_ != other.internal_matrix_size_)
    {
        std::cerr << __PRETTY_FUNCTION__ << ": error, matrices can not be multiplied" << std::endl;
        return BlockMatrix();
    }

    BlockMatrix result(this->getDataHeight(), other.getDataWidth(), internal_matrix_size_);
    result.fillZeros();

    std::vector<std::thread> threads(num_of_threads);

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        threads[i] = std::thread(threadMul,
                                 result.matrices_,
                                 i * height_ / num_of_threads,
                                 (i + 1) * height_ / num_of_threads,
                                 std::cref(*this),
                                 std::cref(other));
    }

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        if (threads[i].joinable())
            threads[i].join();
    }

    return result;
}

BlockMatrix::~BlockMatrix()
{
    for (size_t i = 0; i < height_ * width_; ++i)
    {
        delete matrices_[i];
    }

    delete[] matrices_;
    matrices_ = nullptr;

    height_ = 0;
    width_ = 0;
    internal_matrix_size_ = 0;
}

Matrix **BlockMatrix::operator[](int i)
{
    return matrices_ + i * width_;
}

const Matrix **BlockMatrix::operator[](int i) const
{
    return const_cast<const Matrix **>(matrices_ + i * width_);
}

std::ostream &operator<<(std::ostream &os, const BlockMatrix &m)
{
    for (size_t i = 0; i < m.height_; ++i)
    {
        for (size_t j = 0; j < m.width_; ++j)
        {
            os << *(m.matrices_[i * m.width_ + j]) << std::endl;
        }
    }

    return os;
}
