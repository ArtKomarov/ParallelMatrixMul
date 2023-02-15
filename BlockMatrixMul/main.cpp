#include <iostream>

#include "matrix.hpp"
#include "blockmatrix.hpp"

#include <unistd.h>
#include <chrono>
#include <cmath>

void thread_func();

int main(int argc, char *argv[])
{
    //    const size_t num_of_threads = 4;
    const int matrix_size = std::pow(2, 12);

    if (argc < 2)
    {
        std::cerr << "Set number of threads!" << std::endl;
        return -1;
    }

    Matrix m1(matrix_size, matrix_size);
    Matrix m2(matrix_size, matrix_size);

    m1.serialFill();
    m2.serialFill();
    auto start_time = std::chrono::steady_clock::now();
    Matrix res = m1.multiThreadMul(m2, std::atoi(argv[1]));
    auto finish_time = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time);

    std::cout << "Elapsed time for simple matrix parallelizm: " << static_cast<double>(elapsed_ms.count()) / 1000 << std::endl;

    for (int block_size = 4; block_size <= 256; block_size *= 2)
    {

        BlockMatrix bm1 = BlockMatrix(matrix_size, matrix_size, block_size);
        bm1.serialFill();
        BlockMatrix bm2 = BlockMatrix(matrix_size, matrix_size, block_size);
        bm2.serialFill();

        start_time = std::chrono::steady_clock::now();

        BlockMatrix bm3 = bm1.multiThreadMul(bm2, std::atoi(argv[1]));

        finish_time = std::chrono::steady_clock::now();
        elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(finish_time - start_time);

        std::cout << "Elapsed time for block size = " << block_size << ": " << static_cast<double>(elapsed_ms.count()) / 1000 << std::endl;

        // if(improveRes == res)
        //     std::cout << "EQUAL" << std::endl;

        // usleep(2100000); // microseconds (10^(-6))
        if (bm3 == res)
            std::cout << "EQUALS!!" << std::endl;
    }
    // 25.593-27.668 for first realization, 2.361 for second! With -O2: 4.508 and 0.536

    return 0;
}

// void thread_func() {
//     std::cout << "Hello World!" << std::endl;
// }
