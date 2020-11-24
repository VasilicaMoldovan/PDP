using System;
using System.Collections.Generic;
using System.Net;

namespace Lab4
{
    class Program
    {
        private static readonly List<string> hosts = new List<string> {
            "https://www.cs.ubbcluj.ro/~rlupsa/edu/pdp/progs/srv-begin-end.cs",
            "https://www.cs.ubbcluj.ro/~rlupsa/edu/pdp/progs/srv-task.cs",
            "https://www.cs.ubbcluj.ro/~rlupsa/edu/pdp/progs/srv-await.cs",
        };

        static void Main(string[] args)
        {
            Console.WriteLine("Hello World!");

            //EventDriven.run(hosts);
            //TasksOperations.run(hosts);
            AsyncAwait.run(hosts);
            
        }
    }
}
