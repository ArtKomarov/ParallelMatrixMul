#include <iostream>
#include <mpi.h>
#include <stdio.h>

#include "matrix.h"

int main(int argc, char **argv)
{
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int per_thread_A_height = -1;
    int per_thread_B_width = -1;
    int per_thread_A_height_rest = -1;

    int working_threads_number = size - 1;

    Matrix* A = nullptr;
    Matrix* B = nullptr;
    Matrix* C = nullptr;
    matrix_t* buff = nullptr;
    int A_height = -1;
    int A_width = -1;
    int& B_height = A_width;

    bool check_C = false;

    if (rank == 0)
    {
        A_height = 8 * 256;
        A_width = 8 * 256;
        int B_width = 8 * 256;

        A = new Matrix(A_height, A_width);
        B = new Matrix(A_width, B_width, false);
        C = new Matrix(A_height, B_width);

        A->serialFill();
        B->serialFill();
        C->fillZeros();

        per_thread_A_height = A_height / working_threads_number;
        per_thread_B_width  = B_width  / working_threads_number;
        per_thread_A_height_rest = A_height % working_threads_number;
        int last_per_thread_A_height = per_thread_A_height + per_thread_A_height_rest;

        buff = new matrix_t[(per_thread_A_height + per_thread_A_height_rest) * per_thread_B_width];

        for (int i = 1; i < size; ++i)
        {
            if (i == size - 1)
            {
                MPI_Send(&last_per_thread_A_height, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(&per_thread_A_height, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
            }

            MPI_Send(&per_thread_B_width,  1, MPI_INT, i, 2, MPI_COMM_WORLD);
            MPI_Send(&A_width,             1, MPI_INT, i, 3, MPI_COMM_WORLD);

            if (i == size - 1)
            {
                MPI_Send(A->getBuffer() + (i - 1) * per_thread_A_height * A_width, last_per_thread_A_height * A_width, MPI_DOUBLE, i, 4, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(A->getBuffer() + (i - 1) * per_thread_A_height * A_width, per_thread_A_height * A_width, MPI_DOUBLE, i, 4, MPI_COMM_WORLD);
            }
            //MPI_Send(B.getBuffer() + i * per_thread_B_width * B_width, per_thread_B_width * A_width, MPI_DOUBLE, i, 5, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Status status;
        MPI_Recv(&per_thread_A_height, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&per_thread_B_width,  1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&A_width,             1, MPI_INT, 0, 3, MPI_COMM_WORLD, &status);

        A = new Matrix(per_thread_A_height,            A_width, true);
        B = new Matrix(           B_height, per_thread_B_width, false);

        MPI_Recv(A->getBuffer(), A->size(), MPI_DOUBLE, 0, 4, MPI_COMM_WORLD, &status);
        //printf("%d get A buffer!\n", rank);
    }

    double start_time, end_time;
    start_time = MPI_Wtime();

    // Get new B part and multiply
    for (int i = 0; i < working_threads_number; ++i)
    {
        if (rank == 0)
        {
            for (int thread_num = 1; thread_num < size; ++thread_num)
            {
                int B_part_num = (i + thread_num - 1) % working_threads_number;
                MPI_Send(
                    B->getBuffer() + B_part_num * per_thread_B_width * B_height,
                    per_thread_B_width * B_height,
                    MPI_DOUBLE,
                    thread_num,
                    5,
                    MPI_COMM_WORLD
                );
            }

            // Receive answers
            for (int thread_num = 1; thread_num < size; ++thread_num)
            {
                int per_thread_A_height_curr = per_thread_A_height;
                MPI_Status status;

                if (thread_num == size - 1)
                    per_thread_A_height_curr += per_thread_A_height_rest;

                MPI_Recv(buff, per_thread_A_height_curr * per_thread_B_width, MPI_DOUBLE, thread_num, 6, MPI_COMM_WORLD, &status);

                //std::cout << "C->getBuffer()[c_j]: " << std::endl;
                for (int j = 0; j < per_thread_A_height_curr * per_thread_B_width; j++)
                {
                    int B_part_num = (i + thread_num - 1) % working_threads_number;
                    int c_j = (thread_num - 1) * per_thread_A_height * B->getWidth() + (j / per_thread_B_width) * C->getWidth() + (B_part_num * per_thread_B_width) + j % per_thread_B_width;
                    C->getBuffer()[c_j] = buff[j];
                    //std::cout << C->getBuffer()[c_j] << " ";
                }
            }
            //std::cout << "C->getBuffer()[ij]: " << std::endl;
            //for (int ij = 0; ij < C->size(); ij++)
            //    std::cout << C->getBuffer()[ij] << " ";
            //std::cout << std::endl;
            //std::cout << "C from rank 0: " << *C;
        }
        else
        {
            MPI_Status status;
            MPI_Recv(B->getBuffer(), B->size(), MPI_DOUBLE, 0, 5, MPI_COMM_WORLD, &status);
            //printf("%d get B buffer!\n", rank);
            //std::cout << "A: " << A->size() << ": " << *A << std::endl;
            //std::cout << "B: " << *B << std::endl;
            C = new Matrix((*A) * (*B));
            //std::cout << "C: " << C->size() << ":" << *C << std::endl;
            MPI_Send(C->getBuffer(), C->size(), MPI_DOUBLE, 0, 6, MPI_COMM_WORLD);
            //printf("%d sent C buffer!\n", rank);
        }
    }

    end_time = MPI_Wtime();

    if (rank == 0)
    {
        if (check_C)
            std::cout << "Check A * B == C: " << (((*A) * (*B) == *C) == 1 ? "true" : "false") << std::endl;
        //std::cout << "C:" << *C << std::endl;
        //std::cout << "A * B:" << (*A) * (*B) << std::endl;
        std::cout << "Time: " << end_time - start_time << std::endl;
    }

    MPI_Finalize();
    return 0;
}
