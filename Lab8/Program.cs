using MPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Lab8
{
    class Program
    {
        static void listener_thread(Object obj)
        {
            DSM dsm = (DSM)obj;

            while (true)
            {
                Console.WriteLine("Rank " + Communicator.world.Rank.ToString() + " waiting ");
                Msg msg = Communicator.world.Receive<Msg>(Communicator.anySource, Communicator.anyTag);

                if (msg.exit) break;

                if (msg.update_msg != null)
                {
                    Console.WriteLine("Rank " + Communicator.world.Rank + " received : " + msg.update_msg.var + " -> " + msg.update_msg.val);
                    dsm.set_variable(msg.update_msg.var, msg.update_msg.val);
                }

                if (msg.subscribe_msg != null)
                {
                    Console.WriteLine("Rank " + Communicator.world.Rank + " received : " + msg.subscribe_msg.rank + " sub to " + msg.subscribe_msg.var);
                    dsm.subscribe_other(msg.subscribe_msg.var, msg.subscribe_msg.rank);

                }
                write_variables(dsm);
            }
        }

        static void write_variables(DSM dsm)
        {
            Console.Write("Rank " + Communicator.world.Rank + " a = " + dsm.a + " b = " + dsm.b + " c = " + dsm.c + " subs: ");
            foreach (string var in dsm.subscribers.Keys)
            {
                Console.Write(var + ": [ ");
                foreach (int rank in dsm.subscribers[var])
                {
                    Console.Write(rank + " ");
                }

                Console.Write("] ");
            }

            Console.WriteLine();
        }

        static void Main(string[] args)
        {
            using (new MPI.Environment(ref args))
            {
                DSM dsm = new DSM();

                if (Communicator.world.Rank == 0)
                {

                    Thread thread = new Thread(listener_thread);
                    thread.Start(dsm);

                    bool exit = false;

                    dsm.subscribe("a");
                    dsm.subscribe("b");
                    dsm.subscribe("c");

                    while (!exit)
                    {
                        Console.WriteLine("1. Set variable");
                        Console.WriteLine("2. Check and change variable");
                        Console.WriteLine("0. Exit");

                        int answer;
                        int.TryParse(Console.ReadLine(), out answer);

                        if (answer == 0)
                        {
                            dsm.close();
                            exit = true;
                        }
                        else if (answer == 1)
                        {
                            Console.WriteLine("var (a, b, c) = ");
                            string var = Console.ReadLine();

                            Console.WriteLine("val (int) = ");
                            int val;
                            int.TryParse(Console.ReadLine(), out val);

                            dsm.update_variable(var, val);
                            write_variables(dsm);
                        }
                        else if (answer == 2)
                        {
                            Console.WriteLine("var to check (a, b, c) = ");
                            string var = Console.ReadLine();

                            Console.WriteLine("val to check (int) = ");
                            int val;
                            int.TryParse(Console.ReadLine(), out val);

                            Console.WriteLine("val to change (int) = ");
                            int newVal;
                            int.TryParse(Console.ReadLine(), out newVal);

                            dsm.check_and_replace(var, val, newVal);
                        }
                    }

                }
                else if (Communicator.world.Rank == 1)
                {

                    Thread thread = new Thread(listener_thread);
                    thread.Start(dsm);

                    dsm.subscribe("a");

                    thread.Join();

                }
                else if (Communicator.world.Rank == 2)
                {
                    Thread thread = new Thread(listener_thread);
                    thread.Start(dsm);

                    dsm.subscribe("b");

                    thread.Join();
                }
            }
        }
    }
}
