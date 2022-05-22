#include <iostream>

#include "matrix.hpp"
#include "blockmatrix.hpp"

#include <unistd.h>
#include <chrono>

void thread_func();

int main(int argc, char* argv[]) {
    //    const size_t num_of_threads = 4;

    if (argc < 2) {
        std::cerr << "Set number of threads!" << std::endl;
        return -1;
    }

    //    matrix_t array_values [] = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10,
    //                                11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    //                                21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    //                                31, 32, 33, 34, 35 };

//    matrix_t array_values [] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//                                1, 1, 1, 1, 1};

    Matrix m1 (4, 4);
    Matrix m2 (4, 4);


    m1.serialFill();
    m2.serialFill();
    auto start_time = std::chrono::steady_clock::now();
    Matrix res = m1.multiThreadMul(m2, std::atoi(argv[1]));
    auto finish_time = std::chrono::steady_clock::now();
    auto elapsed_ms  = std::chrono::duration_cast<std::chrono::milliseconds> (finish_time - start_time);

    std::cout << "Elapsed time for simple matrix parallelizm: " <<  static_cast<double>(elapsed_ms.count()) / 1000 << std::endl;

    for (int block_size = 2; block_size <= 2; block_size *= 2) {

        BlockMatrix bm1 = BlockMatrix(4, 4, block_size);
        bm1.serialFill();
        BlockMatrix bm2 = BlockMatrix(4, 4, block_size);
        bm2.serialFill();

        start_time = std::chrono::steady_clock::now();

        BlockMatrix bm3  = bm1.multiThreadMul(bm2, std::atoi(argv[1]));

        finish_time = std::chrono::steady_clock::now();
        elapsed_ms  = std::chrono::duration_cast<std::chrono::milliseconds> (finish_time - start_time);

        std::cout << "Elapsed time for block size = " << block_size << ": " <<  static_cast<double>(elapsed_ms.count()) / 1000 << std::endl;

        //if(improveRes == res)
        //    std::cout << "EQUAL" << std::endl;

        //usleep(2100000); // microseconds (10^(-6))
        std::cout << bm3 << std::endl;
        std::cout << res << std::endl;
        if (bm3 == res)
            std::cout << "EQUALS!!" << std::endl;

    }
    // 25.593-27.668 for first realization, 2.361 for second! With -O2: 4.508 and 0.536

    return 0;
}

//void thread_func() {
//    std::cout << "Hello World!" << std::endl;
//}
