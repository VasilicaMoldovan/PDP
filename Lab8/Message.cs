using MPI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab8
{
    [Serializable]
    class Msg
    {
        public UpdateMsg update_msg = null;
        public ChangeMsg change_msg = null;
        public SubscribeMsg subscribe_msg = null;

        public bool exit = false;

        public Msg(UpdateMsg update_msg)
        {
            this.update_msg = update_msg;
        }

        public Msg(ChangeMsg change_msg)
        {
            this.change_msg = change_msg;
        }

        public Msg(SubscribeMsg subscribe_msg)
        {
            this.subscribe_msg = subscribe_msg;
        }

        public Msg(bool exit)
        {
            this.exit = exit;
        }
    }

    [Serializable]
    class SubscribeMsg
    {
        public string var;
        public int rank;

        public SubscribeMsg(string var, int rank)
        {
            this.var = var;
            this.rank = rank;
        }
    }

    [Serializable]
    class UpdateMsg
    {
        public string var;
        public int val;

        public UpdateMsg(string var, int val)
        {
            this.var = var;
            this.val = val;
        }
    }

    [Serializable]
    class ChangeMsg
    {
        public string var;
        public int oldVal;
        public int newVal;

        public ChangeMsg(string var, int oldVal, int newVal)
        {
            this.var = var;
            this.oldVal = oldVal;
            this.newVal = newVal;
        }
    }
}
