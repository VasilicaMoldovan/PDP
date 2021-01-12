using MPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab8
{
    class DSM
    {
        public int a = 1, b = 2, c = 3;
        public Dictionary<String, List<int>> subscribers = new Dictionary<string, List<int>>();

        public DSM()
        {
            subscribers.Add("a", new List<int>());
            subscribers.Add("b", new List<int>());
            subscribers.Add("c", new List<int>());
        }

        public void update_variable(string var, int val)
        {

            this.set_variable(var, val);
            UpdateMsg update_msg = new UpdateMsg(var, val);
            Msg msg = new Msg(update_msg);

            this.send_to_subscribers(var, msg);
        }

        public void close()
        {
            this.send_all(new Msg(true));
        }

        public void send_all(Msg msg)
        {
            for (int i = 0; i < Communicator.world.Size; i++)
            {
                if (Communicator.world.Rank == i) continue;
                Communicator.world.Send(msg, i, 0);
            }
        }

        public void set_variable(string var, int val)
        {
            if (var == "a") a = val;
            if (var == "b") b = val;
            if (var == "c") c = val;
        }

        public void subscribe(string var)
        {
            this.subscribers[var].Add(Communicator.world.Rank);

            this.send_all(new Msg(new SubscribeMsg(var, Communicator.world.Rank)));
        }

        public void subscribe_other(string var, int rank)
        {
            this.subscribers[var].Add(rank);
        }

        public void send_to_subscribers(string var, Msg msg)
        {
            for (int i = 0; i < Communicator.world.Size; i++)
            {
                if (Communicator.world.Rank == i) continue;
                if (!is_subscribed_to(var, i)) continue;

                Communicator.world.Send(msg, i, 0);
            }
        }

        public bool is_subscribed_to(string var, int rank)
        {
            if (subscribers[var].Contains(rank))
            {
                return true;
            }

            return false;
        }

        internal void check_and_replace(string var, int val, int new_val)
        {
            if (var == "a")
            {
                if (a == val)
                {
                    update_variable("a", new_val);
                }
            }

            if (var == "b")
            {
                if (b == val)
                {
                    update_variable("b", new_val);
                }
            }

            if (var == "c")
            {
                if (a == val)
                {
                    update_variable("c", new_val);
                }
            }
        }
    }
}
