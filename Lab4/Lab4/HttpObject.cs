using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace Lab4
{
    class HttpObject
    {
        public Socket socket = null;
        public int id;
        public string hostname;
        public string endpoint;
        public const int BUFF_SIZE = 512;
        public byte[] buffer = new byte[BUFF_SIZE];
        public StringBuilder response = new StringBuilder();
        public IPEndPoint remote_endpoint;
        public ManualResetEvent connect = new ManualResetEvent(false);
        public ManualResetEvent send = new ManualResetEvent(false);
        public ManualResetEvent receive = new ManualResetEvent(false);
    }
}
