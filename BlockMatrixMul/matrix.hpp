#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>

typedef double matrix_t;

class Matrix {
    matrix_t* matrix_;

    size_t height_;
    size_t width_;

public:
    /// Constructors
    Matrix ();
    Matrix (const Matrix& other);
    Matrix (Matrix&& other);
    Matrix (size_t height, size_t width);
    Matrix (matrix_t *values, size_t height, size_t width);

    /// Getters
    size_t getHeight() const;
    size_t getWidth () const;

    void fillZeros();

    void serialFill();

    void fill (matrix_t* array);

    void setRow (size_t row, matrix_t* array);

    matrix_t* operator [] (int i);
    const matrix_t* operator [] (int i) const;

    const Matrix& operator = (const Matrix& other);
    const Matrix& operator = (Matrix&& other);

    bool operator == (const Matrix& other) const;

    Matrix operator * (const Matrix& other) const;

    Matrix operator + (const Matrix& other) const;
    const Matrix& operator += (const Matrix& other);

    /// First simple multiplication realization
    Matrix simpleMul (const Matrix& other) const;

    /// Second improved multiplication realization
    Matrix improvedMul (const Matrix& other) const;

    /// Multiprocessing multiplication realization
    Matrix multiThreadMul (const Matrix& other, size_t num_of_threads) const;

    friend std::ostream& operator<< (std::ostream& os, const Matrix& m);

    friend void threadMul(Matrix& result_matrix, size_t start_row, size_t finish_row, const Matrix& this_matrix, const Matrix& other_matrix);

    ~Matrix();

};


#endif // MATRIX_HPP
