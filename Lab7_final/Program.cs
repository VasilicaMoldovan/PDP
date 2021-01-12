using MPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Lab7_final
{
    class Program
    {
        public static Polynomial give_final_result(Polynomial[] results)
        {
            if (results.Length > 0)
            {
                Polynomial result = new Polynomial(results[0].Degree);

                for (int i = 0; i < result.size; i++)
                    for (int j = 0; j < results.Length; j++)
                        result.Coefficients[i] += results[j].Coefficients[i];

                return result;
            } else
            {
                return new Polynomial(0);
            }
        }

        public static void MPI_multiplication_main(Polynomial polynomial1, Polynomial polynomial2)
        {
            DateTime start = DateTime.Now;

            int n = Communicator.world.Size;
            int begin = 0;
            int end = 0;
            int length = polynomial1.size;
            if (n > 1)
            {
               length = length / (n - 1);
            }

            for (int i = 1; i < n; i++)
            {
                begin = end;
                end = end + length;
                if (i == n - 1)
                    end = polynomial1.size;

                Communicator.world.Send(polynomial1, i, 0);
                Communicator.world.Send(polynomial2, i, 0);
                Communicator.world.Send(begin, i, 0);
                Communicator.world.Send(end, i, 0);
            }

            Polynomial[] results = new Polynomial[n - 1];

            for (int i = 1; i < n; i++)
                results[i - 1] = Communicator.world.Receive<Polynomial>(i, 0);

            Polynomial result = give_final_result(results);

            double time = (DateTime.Now - start).Milliseconds;
            Console.WriteLine("MPI Multiplication: " + result.ToString() + "\n" + "Time: " + time.ToString() + " milliseconds");
        }

        public static void MPI_multiplication_worker()
        {
            Polynomial polynomial1 = Communicator.world.Receive<Polynomial>(0, 0);
            Polynomial polynomial2 = Communicator.world.Receive<Polynomial>(0, 0);

            int begin = Communicator.world.Receive<int>(0, 0);
            int end = Communicator.world.Receive<int>(0, 0);

            Polynomial result = Operations.MPI_multiply(polynomial1, polynomial2, begin, end);

            Communicator.world.Send(result, 0, 0);
        }
        public static void MPI_karatsuba_main(Polynomial polynomial1, Polynomial polynomial2)
        {
            DateTime start = DateTime.Now;

            Polynomial result = new Polynomial(polynomial1.Degree * 2);
            if (Communicator.world.Size == 1)
            {
                result = Operations.asynchronous_karatsuba_multiply(polynomial1, polynomial2);
            }
            else
            {
                Communicator.world.Send<int>(0, 1, 0);
                Communicator.world.Send<int[]>(polynomial1.Coefficients, 1, 0);
                Communicator.world.Send<int[]>(polynomial2.Coefficients, 1, 0);
                if (Communicator.world.Size == 2)
                    Communicator.world.Send<int[]>(new int[0], 1, 0);
                else
                    Communicator.world.Send<int[]>(Enumerable.Range(2, Communicator.world.Size - 2).ToArray(), 1, 0);

                int[] coefs = Communicator.world.Receive<int[]>(1, 0);
                result.Coefficients = coefs;
            }

            double time = (DateTime.Now - start).Milliseconds;
            Console.WriteLine("MPI  Karatsuba: " + result.ToString() + "\n" + "Time: " + time.ToString() + " milliseconds");
        }

        public static void MPI_karatsuba_worker()
        {
            Operations.MPI_karatsuba_multiply();
        }

        static void Main(string[] args)
        {
            using (new MPI.Environment(ref args))
            {
                if (Communicator.world.Rank == 0)
                {
                    int totalProcessors = Communicator.world.Size - 1;

                    int firstLength = 7;
                    int secondLength = 7;
                    Polynomial polynomial1 = new Polynomial(firstLength);
                    polynomial1.generate_polynomial();
                    Thread.Sleep(500);
                    Polynomial polynomial2 = new Polynomial(secondLength);
                    polynomial2.generate_polynomial();

                    if (firstLength > secondLength)
                        polynomial2 = polynomial2.add_zeros(firstLength - secondLength);
                    else if (secondLength > firstLength)
                        polynomial1 = polynomial1.add_zeros(secondLength - firstLength);

                    Console.WriteLine("polyn1 { size = " + polynomial1.size + " }, degree = " + polynomial1.Degree + "}: \n" + polynomial1.ToString());
                    Console.WriteLine("\npolyn2 { size = " + polynomial2.size + " }, degree = " + polynomial2.Degree + "}:\n" + polynomial2.ToString() + "\n");

                    MPI_multiplication_main(polynomial1, polynomial2);
                    Console.WriteLine("\n");
                    MPI_karatsuba_main(polynomial1, polynomial2);
                }
                else
                {
                    MPI_multiplication_worker();
                    MPI_karatsuba_worker();
                }
            }
        }
    }
}
