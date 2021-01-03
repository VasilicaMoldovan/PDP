#include "Polynomial.cpp"
#include <mpi.h>
#include <algorithm>
#include <mutex>
#include <future>

class Operations {
public:
    static Polynomial MPI_multiply(Polynomial polynomial1, Polynomial polynomial2, int begin, int end)
    {
        int maxDegree = std::max(polynomial1.get_degree(), polynomial2.get_degree());
        Polynomial result = Polynomial(maxDegree * 2);

        for (int i = begin; i < end; i++)
            for (int j = 0; j < polynomial2.get_size(); j++) {
                int aux = result.get_coeff(i + j);
                aux += polynomial1.get_coeff(i) * polynomial2.get_coeff(j);
                result.set_coeff(i + j, aux);
            }

        return result;
    }

    static std::vector<int> asynchronous_karatsuba_multiply_recursive(std::vector<int> coefficients1, std::vector<int> coefficients2)
    {
        //might need to lock a mutex here
        std::vector<int> product = std::vector<int>(2 * coefficients1.size(), 0);

        //Handle the base case where the polynomial has only one coefficient
        if (coefficients1.size() == 1)
        {
            product[0] = coefficients1[0] * coefficients2[0];
            return product;
        }

        int halfArraySize = coefficients1.size() / 2;

        //Declare arrays to hold halved factors
        std::vector<int> coefficients1Low = std::vector<int>(halfArraySize);
        std::vector<int> coefficients1High = std::vector<int>(halfArraySize);
        std::vector<int> coefficients2Low = std::vector<int>(halfArraySize);
        std::vector<int> coefficients2High = std::vector<int>(halfArraySize);

        std::vector<int> coefficients1LowHigh = std::vector<int>(halfArraySize);
        std::vector<int> coefficients2LowHigh = std::vector<int>(halfArraySize);

        //Fill in the low and high arrays
        for (int halfSizeIndex = 0; halfSizeIndex < halfArraySize; halfSizeIndex++)
        {

            coefficients1Low[halfSizeIndex] = coefficients1[halfSizeIndex];
            coefficients1High[halfSizeIndex] = coefficients1[halfSizeIndex + halfArraySize];
            coefficients1LowHigh[halfSizeIndex] = coefficients1Low[halfSizeIndex] + coefficients1High[halfSizeIndex];

            coefficients2Low[halfSizeIndex] = coefficients2[halfSizeIndex];
            coefficients2High[halfSizeIndex] = coefficients2[halfSizeIndex + halfArraySize];
            coefficients2LowHigh[halfSizeIndex] = coefficients2Low[halfSizeIndex] + coefficients2High[halfSizeIndex];

        }

        //Recursively call method on smaller arrays and construct the low and high parts of the product
        auto t1 = std::async(std::launch::async, asynchronous_karatsuba_multiply_recursive, coefficients1Low, coefficients2Low);
        auto t2 = std::async(std::launch::async, asynchronous_karatsuba_multiply_recursive, coefficients1High, coefficients2High);
        auto t3 = std::async(std::launch::async, asynchronous_karatsuba_multiply_recursive, coefficients1LowHigh, coefficients2LowHigh);

        std::vector<int> productLow = t1.get();
        std::vector<int> productHigh = t2.get();
        std::vector<int> productLowHigh = t3.get();

        //Construct the middle portion of the product
        std::vector<int> productMiddle = std::vector<int>(coefficients1.size(), 0);
        for (int halfSizeIndex = 0; halfSizeIndex < coefficients1.size(); halfSizeIndex++)
            productMiddle[halfSizeIndex] = productLowHigh[halfSizeIndex] - productLow[halfSizeIndex] - productHigh[halfSizeIndex];

        //Assemble the product from the low, middle and high parts. Start with the low and high parts of the product.
        for (int halfSizeIndex = 0, middleOffset = coefficients1.size() / 2; halfSizeIndex < coefficients1.size(); ++halfSizeIndex)
        {
            product[halfSizeIndex] += productLow[halfSizeIndex];
            product[halfSizeIndex + coefficients1.size()] += productHigh[halfSizeIndex];
            product[halfSizeIndex + middleOffset] += productMiddle[halfSizeIndex];
        }

        return product;
    }

    static void MPI_karatsuba_multiply()
    {
        MPI_Status status;
        int from;
        MPI_Recv(&from, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        std::vector<int> coefficients1;
        MPI_Recv(&coefficients1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        std::vector<int> coefficients2;
        MPI_Recv(&coefficients2, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        std::vector<int> sendTo;
        MPI_Recv(&sendTo, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

        std::vector<int> product = std::vector<int>(2 * coefficients1.size());

        //Handle the base case where the polynomial has only one coefficient
        if (coefficients1.size() == 1)
        {
            product[0] = coefficients1[0] * coefficients2[0];

            MPI_Ssend(&product, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
            // from variable
            return;
        }

        int halfArraySize = coefficients1.size() / 2;

        //Declare arrays to hold halved factors
        std::vector<int> coefficients1Low = std::vector<int>(halfArraySize);
        std::vector<int> coefficients1High = std::vector<int>(halfArraySize);
        std::vector<int> coefficients2Low = std::vector<int>(halfArraySize);
        std::vector<int> coefficients2High = std::vector<int>(halfArraySize);

        std::vector<int> coefficients1LowHigh = std::vector<int>(halfArraySize);
        std::vector<int> coefficients2LowHigh = std::vector<int>(halfArraySize);
        //Fill in the low and high arrays
        for (int halfSizeIndex = 0; halfSizeIndex < halfArraySize; halfSizeIndex++)
        {

            coefficients1Low[halfSizeIndex] = coefficients1[halfSizeIndex];
            coefficients1High[halfSizeIndex] = coefficients1[halfSizeIndex + halfArraySize];
            coefficients1LowHigh[halfSizeIndex] = coefficients1Low[halfSizeIndex] + coefficients1High[halfSizeIndex];

            coefficients2Low[halfSizeIndex] = coefficients2[halfSizeIndex];
            coefficients2High[halfSizeIndex] = coefficients2[halfSizeIndex + halfArraySize];
            coefficients2LowHigh[halfSizeIndex] = coefficients2Low[halfSizeIndex] + coefficients2High[halfSizeIndex];

        }

        //Recursively call method on smaller arrays and construct the low and high parts of the product
        std::vector<int> productLow, productHigh, productLowHigh;

        if (sendTo.size() == 0)
        {
            productLow = asynchronous_karatsuba_multiply_recursive(coefficients1Low, coefficients2Low);
            productHigh = asynchronous_karatsuba_multiply_recursive(coefficients1High, coefficients2High);
            productLowHigh = asynchronous_karatsuba_multiply_recursive(coefficients1LowHigh, coefficients2LowHigh);
        }
        else if (sendTo.size() == 1)
        {
            int me;
            MPI_Comm_rank(MPI_COMM_WORLD, &me);
            MPI_Ssend(&me, 1, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            /// <summary>
            /// ?????
            ///  Communicator.world.Send<int[]>(new int[0], sendTo[0], 0);
            /// </summary>

            productHigh = asynchronous_karatsuba_multiply_recursive(coefficients1High, coefficients2High);
            productLowHigh = asynchronous_karatsuba_multiply_recursive(coefficients1LowHigh, coefficients2LowHigh);

            MPI_Recv(&productLow, productLow.size(), MPI_INT, sendTo[0], 1, MPI_COMM_WORLD, &status);

            //productLow = Communicator.world.Receive<int[]>(sendTo[0], 0);
        }
        else if (sendTo.size() == 2)
        {
            int me;
            MPI_Comm_rank(MPI_COMM_WORLD, &me);
            MPI_Ssend(&me, 1, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            /// ??? Communicator.world.Send<int[]>(new int[0], sendTo[0], 0);

            MPI_Ssend(&me, 1, MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            /// ??? Communicator.world.Send<int[]>(new int[0], sendTo[1], 0);

            productLowHigh = asynchronous_karatsuba_multiply_recursive(coefficients1LowHigh, coefficients2LowHigh);

            MPI_Recv(&productLow, productLow.size(), MPI_INT, sendTo[0], 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&productHigh, productHigh.size(), MPI_INT, sendTo[1], 1, MPI_COMM_WORLD, &status);
        }
        else if (sendTo.size() == 3)
        {
            int me;
            MPI_Comm_rank(MPI_COMM_WORLD, &me);
            MPI_Ssend(&me, 1, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            /// ??? Communicator.world.Send<int[]>(new int[0], sendTo[0], 0);

            MPI_Ssend(&me, 1, MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            /// ??? Communicator.world.Send<int[]>(new int[0], sendTo[1], 0);

            MPI_Ssend(&me, 1, MPI_INT, sendTo[2], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[2], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[2], 3, MPI_COMM_WORLD);
            /// ??? Communicator.world.Send<int[]>(new int[0], sendTo[2], 0);

            MPI_Recv(&productLow, productLow.size(), MPI_INT, sendTo[0], 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&productHigh, productHigh.size(), MPI_INT, sendTo[1], 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&productLowHigh, productLowHigh.size(), MPI_INT, sendTo[2], 1, MPI_COMM_WORLD, &status);
        }
        else
        {
            std::vector<int> auxSendTo = std::vector<int>(sendTo.size() - 3, 0);
            for (int i = 3; i < sendTo.size(); i++) {
                auxSendTo[i - 3] = sendTo[i];
            }
            int auxLength = auxSendTo.size() / 3;

            int me;
            MPI_Comm_rank(MPI_COMM_WORLD, &me);
            MPI_Ssend(&me, 1, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1Low, coefficients1Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2Low, coefficients2Low.size(), MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            std::vector<int> aux = std::vector<int>(auxLength, 0);
            std::copy_n(auxSendTo.begin(), auxLength, aux.begin());
            MPI_Ssend(&aux, auxLength, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);

            MPI_Ssend(&me, 1, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1High, coefficients1High.size(), MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2High, coefficients2High.size(), MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);
            aux = std::vector<int>(auxSendTo.size(), 0);
            std::copy_n(auxSendTo.begin() + auxLength, auxSendTo.size() - auxLength, aux.begin());
            std::copy_n(aux.begin(), auxLength, aux.begin());
            MPI_Ssend(&aux, auxLength, MPI_INT, sendTo[1], 3, MPI_COMM_WORLD);

            MPI_Ssend(&me, 1, MPI_INT, sendTo[0], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients1LowHigh, coefficients1LowHigh.size(), MPI_INT, sendTo[2], 3, MPI_COMM_WORLD);
            MPI_Ssend(&coefficients2LowHigh, coefficients2LowHigh.size(), MPI_INT, sendTo[2], 3, MPI_COMM_WORLD);
            aux = std::vector<int>(auxSendTo.size() - 2 * auxLength, 0);
            std::copy_n(auxSendTo.begin() + 2 * auxLength, auxSendTo.size() - 2 * auxLength, aux.begin());
            MPI_Ssend(&aux, auxLength, MPI_INT, sendTo[2], 3, MPI_COMM_WORLD);

            MPI_Recv(&productLow, productLow.size(), MPI_INT, sendTo[0], 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&productHigh, productHigh.size(), MPI_INT, sendTo[1], 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&productLowHigh, productLowHigh.size(), MPI_INT, sendTo[2], 1, MPI_COMM_WORLD, &status);

        }

        //Construct the middle portion of the product
        std::vector<int> productMiddle = std::vector<int>(coefficients1.size());
        for (int halfSizeIndex = 0; halfSizeIndex < coefficients1.size(); halfSizeIndex++)
        {
            productMiddle[halfSizeIndex] = productLowHigh[halfSizeIndex] - productLow[halfSizeIndex] - productHigh[halfSizeIndex];
        }

        //Assemble the product from the low, middle and high parts. Start with the low and high parts of the product.
        for (int halfSizeIndex = 0, middleOffset = coefficients1.size() / 2; halfSizeIndex < coefficients1.size(); ++halfSizeIndex)
        {
            product[halfSizeIndex] += productLow[halfSizeIndex];
            product[halfSizeIndex + coefficients1.size()] += productHigh[halfSizeIndex];
            product[halfSizeIndex + middleOffset] += productMiddle[halfSizeIndex];
        }

        MPI_Ssend(&product, product.size(), MPI_INT, from, 3, MPI_COMM_WORLD);
    }


    static Polynomial asynchronous_karatsuba_multiply(Polynomial p1, Polynomial p2)
    {
        Polynomial result = Polynomial(p1.get_degree() + p2.get_degree());
        result.set_all_coeff(asynchronous_karatsuba_multiply_recursive(p1.get_all_coeff(), p2.get_all_coeff()));

        return result;
    }
};