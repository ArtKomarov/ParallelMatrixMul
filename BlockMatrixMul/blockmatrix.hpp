#ifndef BLOCKMATRIX_HPP
#define BLOCKMATRIX_HPP

#include "matrix.hpp"


class BlockMatrix {
    Matrix** matrices_;

    size_t height_;
    size_t width_;

    size_t internal_matrix_size_;

public:
    /// Constructors
    BlockMatrix ();
    BlockMatrix (const BlockMatrix& other);
    BlockMatrix (size_t height, size_t width, size_t internal_matrix_size);
    BlockMatrix (matrix_t *values, size_t height, size_t width, size_t internal_matrix_size);

    void fillZeros();

    void serialFill();

    size_t getDataWidth() const;

    size_t getDataHeight() const;

    /// Multiprocessing multiplication realization
    BlockMatrix multiThreadMul (const BlockMatrix& other, size_t num_of_threads) const;
    BlockMatrix multiThreadMulSplitted (const BlockMatrix &other, size_t num_of_threads) const;

    const BlockMatrix& operator = (const BlockMatrix& other);

    BlockMatrix operator * (const BlockMatrix& other) const;

    bool operator == (const Matrix& other);

          Matrix** operator [] (int i);
    const Matrix** operator [] (int i) const;

    friend std::ostream& operator<< (std::ostream& os, const BlockMatrix& m);

    friend void threadMul (Matrix** result_matrix, size_t start_row, size_t finish_row,
                           const BlockMatrix& this_matrix, const BlockMatrix& other_matrix);
    friend void threadMulSplitted (Matrix** result_matrix, size_t start_row, size_t finish_row,
                           const BlockMatrix& this_matrix, const BlockMatrix& other_matrix);

    ~BlockMatrix ();
};

#endif // BLOCKMATRIX_HPP
