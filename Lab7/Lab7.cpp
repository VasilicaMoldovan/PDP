// Lab7.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <mpi.h>
#include <vector>
#include <thread>
#include <chrono>
#include "Polynomial.cpp"
#include "Operations.cpp"

#define POLYN_LENGTH 3

static Polynomial recreate_polynomial(std::vector<int> coeffs) {
    int degree = coeffs.size();
    Polynomial result = Polynomial(degree);
    result.set_all_coeff(coeffs);

    return result;
}

static Polynomial compute_final_result(std::vector<Polynomial> results)
{
    if (results.size() == 0) {
        return Polynomial(0);
    }
    else {
        Polynomial result = Polynomial(results[0].get_degree());

        for (int i = 0; i < result.get_size(); i++)
            for (int j = 0; j < results.size(); j++)
                result.set_coeff(i, result.get_coeff(i) + results[j].get_coeff(i));

        return result;
    }
}

static void MPI_multiplication_master(Polynomial polynomial1, Polynomial polynomial2)
{
    auto start = std::chrono::high_resolution_clock::now();

    int n;
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    int begin = 0;
    int end = 0;
    int length = polynomial1.get_size();
    if (n > 1) {
        length = length / (n - 1);
    }

    std::cout << "aici";
    for (int i = 1; i < n; i++)
    {
        begin = end;
        end = end + length;
        if (i == n - 1)
            end = polynomial1.get_size();

        std::cout << "aici1";

        // might not work for an instance of type polynomial => some other operations must be done before 
        //MPI_Ssend(&polynomial1, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
        std::vector<int> polyn1 = polynomial1.get_all_coeff();
        std::vector<int> polyn2 = polynomial2.get_all_coeff();
        MPI_Send(&polyn1, polyn1.size(), MPI_INT, 0, i, MPI_COMM_WORLD);
        //MPI_Ssend(&polynomial2, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
        MPI_Send(&polyn2, polyn2.size(), MPI_INT, 0, i, MPI_COMM_WORLD);
        MPI_Send(&begin, 1, MPI_INT, 0, i, MPI_COMM_WORLD);
        MPI_Send(&end, 1, MPI_INT, 0, i, MPI_COMM_WORLD);
    }

    std::vector<Polynomial> results = std::vector<Polynomial>(n - 1);
    std::vector<int> aux_coeff = std::vector<int>(2 * length);

    std::cout << "aici2";
    MPI_Status status;
    for (int i = 1; i < n; i++) {
        ///????
        MPI_Recv(&aux_coeff, 2 * POLYN_LENGTH, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
        results[i - 1] = recreate_polynomial(aux_coeff);
        aux_coeff.clear();
    }

    std::cout << "aici3";
    Polynomial result = compute_final_result(results);

    std::cout << "aic3";
    double time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "MPI Multiplication: " + result.to_string() + "\n" + "TIME: " + std::to_string(time) + " milliseconds";
}

static void MPI_multiplication_worker()
{
    //Console.WriteLine("Child");
    std::vector<int> aux_polyn = std::vector<int>(POLYN_LENGTH);
    MPI_Status status;
    MPI_Recv(&aux_polyn, POLYN_LENGTH, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    Polynomial polynomial1 = recreate_polynomial(aux_polyn);
    aux_polyn.clear();
    MPI_Recv(&aux_polyn, POLYN_LENGTH, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    Polynomial polynomial2 = recreate_polynomial(aux_polyn);

    int begin;
    int end;
    MPI_Recv(&begin, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&end, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    Operations op = Operations();
    Polynomial result = op.MPI_multiply(polynomial1, polynomial2, begin, end);
    aux_polyn = std::vector<int>(result.get_size());
    aux_polyn = result.get_all_coeff();

    MPI_Send(&aux_polyn, aux_polyn.size(), MPI_INT, 0, 3, MPI_COMM_WORLD);
}

static void MPI_karatsuba_master(Polynomial polynomial1, Polynomial polynomial2)
{
    auto start = std::chrono::high_resolution_clock::now();
    Operations op = Operations();

    Polynomial result = Polynomial(polynomial1.get_degree() * 2);
    int nrProcs;
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcs);
    if (nrProcs == 1)
    {
        result = op.asynchronous_karatsuba_multiply(polynomial1, polynomial2);
    }
    else
    {
        MPI_Ssend(0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Ssend(&polynomial1.get_all_coeff(), polynomial1.get_all_coeff().size(), MPI_INT, 0, 3, MPI_COMM_WORLD);
        MPI_Ssend(&polynomial2.get_all_coeff(), polynomial2.get_all_coeff().size(), MPI_INT, 0, 3, MPI_COMM_WORLD);
        if (nrProcs == 2)
            MPI_Ssend(0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
           /// ???  Communicator.world.Send<int[]>(new int[0], 1, 0);
        else {
            std::vector<int> aux = std::vector<int>(nrProcs - 4, 0);
            for (int i = 2; i < nrProcs - 2; i++)
                aux[i - 2] = i;
            MPI_Ssend(&aux, aux.size(), MPI_INT, 1, 3, MPI_COMM_WORLD);
        }
            
        std::vector<int> coeffs;
        /// <summary>
        ///  ?????
        /// </summary>
        MPI_Status status;
        MPI_Recv(&coeffs, nrProcs, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        result.set_all_coeff(coeffs);
    }

    double time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "MPI Karatsuba: " + result.to_string() + "\n" + "TIME: " + std::to_string(time) + " milliseconds";
 }

static void MPI_karatsuba_worker()
{
    Operations op = Operations();
    op.MPI_karatsuba_multiply();
}

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    int me;
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &me);

    if (me == 0)
    {
        //master process
        int totalProcessors = size - 1;

        int firstLength = 3;
        int secondLength = 3;
        Polynomial polynomial1 = Polynomial(firstLength);
        polynomial1.generate_polynomial();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        Polynomial polynomial2 = Polynomial(secondLength);
        polynomial2.generate_polynomial();

        if (firstLength > secondLength)
            polynomial2 = polynomial2.add_zeros_left(firstLength - secondLength);
        else if (secondLength > firstLength)
            polynomial1 = polynomial1.add_zeros_left(secondLength - firstLength);

        std::cout << "p1 { size = " << polynomial1.get_size() << " }, degree = " << polynomial1.get_degree() << "}: \n" << polynomial1.to_string() << "\n";
        std::cout << "p2 { size = " << polynomial2.get_size() << " }, degree = " << polynomial2.get_degree() << "}: \n" << polynomial2.to_string() << "\n";

        MPI_multiplication_master(polynomial1, polynomial2);
        std::cout << "\n";
        //MPI_karatsuba_master(polynomial1, polynomial2);
    }
    else
    {
        //child process
        std::cout << "\nworker\n";
        MPI_multiplication_worker();
        //MPI_karatsuba_worker();
    }

    MPI_Finalize();
}
