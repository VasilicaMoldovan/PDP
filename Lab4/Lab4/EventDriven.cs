using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace Lab4
{
    class EventDriven
    {
        private static List<string> hosts;
        private static int idx = -1;
        private static WebClient client = new WebClient();

        private static void do_start(object obj_id)
        {
            var id = (int)obj_id;
            idx++;

            start(hosts[id], id);
        }

        private static void start(string host, int id)
        {
            var ip_host = Dns.GetHostEntry(host.Split('/')[2]);
            var ip_address = ip_host.AddressList[0];
            var remoteEndpoint = new IPEndPoint(ip_address, Utils.HTTP_PORT);
            
            var client = new Socket(ip_address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            var obj = new HttpObject
            {
                socket = client,
                hostname = host.Split('/')[2],
                endpoint = host.Contains("/") ? host.Substring(host.IndexOf("/")) : "/",
                remote_endpoint = remoteEndpoint,
                id = id
            };

            obj.socket.BeginConnect(obj.remote_endpoint, make_connection, obj);
        }


        private static void make_connection(IAsyncResult ar)
        {
            var obj = (HttpObject)ar.AsyncState;
            var client_socket = obj.socket;
            var client_id = obj.id;
            var hostname = obj.hostname;

            client_socket.EndConnect(ar);
            Console.WriteLine("{0} --> Socket connected to {1} ({2})", client_id, hostname, client_socket.RemoteEndPoint);

            var data = Encoding.ASCII.GetBytes(Utils.get_request_string(obj.hostname, obj.endpoint));

            obj.socket.BeginSend(data, 0, data.Length, 0, send_info, obj);
        }

        private static void send_info(IAsyncResult ar)
        {
            var obj = (HttpObject)ar.AsyncState;
            var client_socket = obj.socket;
            var client_id = obj.id;

            var bytes = client_socket.EndSend(ar);
            Console.WriteLine("{0} --> Sent {1} bytes to server.", client_id, bytes);

            obj.socket.BeginReceive(obj.buffer, 0, HttpObject.BUFF_SIZE, 0, receive_info, obj);
        }


        private static void receive_info(IAsyncResult ar)
        {
            var obj = (HttpObject)ar.AsyncState;
            var client_socket = obj.socket;
            var client_id = obj.id;

            try
            {
                var bytes_received = client_socket.EndReceive(ar);

                obj.response.Append(Encoding.ASCII.GetString(obj.buffer, 0, bytes_received));

                if (!Utils.check_response_header(obj.response.ToString()))
                {
                    client_socket.BeginReceive(obj.buffer, 0, HttpObject.BUFF_SIZE, 0, receive_info, obj);
                }
                else
                {
                    var response_body = Utils.get_response(obj.response.ToString());

                    var header_length = Utils.get_content_length(obj.response.ToString());
                    if (response_body.Length < header_length)
                    {
                        client_socket.BeginReceive(obj.buffer, 0, HttpObject.BUFF_SIZE, 0, receive_info, obj);
                    }
                    else
                    {
                        //string file_name = @"C:\Users\Vasilica\Desktop\docs\PDP\PDP\Lab4\Lab4\file";
                        string fileName = "C:\\Users\\Vasilica\\Desktop\\docs\\PDP\\PDP\\Lab4\\Lab4\\file";
                        string ext = ".cs";
                        string[] responses = obj.response.ToString().Split('\r', '\n');

                        //System.IO.File.WriteAllLines(file_name + idx.ToString() + ext, responses);
                        client.DownloadFile(hosts[idx], fileName + idx.ToString() + ext);

                        client_socket.Shutdown(SocketShutdown.Both);
                        client_socket.Close();
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public static void run(List<string> hostnames)
        {
            hosts = hostnames;

            for (var i = 0; i < hosts.Count; i++)
            {
                do_start(i);
                Thread.Sleep(5000);
            }
        }

    }
}
