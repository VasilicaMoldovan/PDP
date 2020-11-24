using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace Lab4
{
    class AsyncAwait
    {
        private static List<string> hosts;
        private static int idx = -1;
        
        private static void do_start(object id_obj)
        {
            var id = (int)id_obj;

            start(hosts[id], id);
        }

        private static async void start(string host, int id)
        {
            // establish the remote endpoint of the server  
            var ip_host = Dns.GetHostEntry(host.Split('/')[2]);
            var ip_address = ip_host.AddressList[0];
            var remoteEndpoint = new IPEndPoint(ip_address, Utils.HTTP_PORT);

            // TCP/IP socket
            var client = new Socket(ip_address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            // create a wrapper for the connection information
            var obj = new HttpObject
            {
                socket = client,
                hostname = host.Split('/')[2],
                endpoint = host.Contains("/") ? host.Substring(host.IndexOf("/")) : "/",
                remote_endpoint = remoteEndpoint,
                id = id
            };

            // connect to the remote endpoint  
            await connect_wrapper(obj);

            // request data from the server
            await send_wrapper(obj, Utils.get_request_string(obj.hostname, obj.endpoint));

            // receive the response from the server
            await receive_wrapper(obj);

            // write the response details to the console
            Console.WriteLine(
                "{0} --> Response received : expected {1} chars in body, got {2} chars (headers + body)",
                id, Utils.get_content_length(obj.response.ToString()), obj.response.Length);

            client.Shutdown(SocketShutdown.Both);
            client.Close();
        }

        private static async Task connect_wrapper(HttpObject obj)
        {
            obj.socket.BeginConnect(obj.remote_endpoint, connect_callback, obj);

            await Task.FromResult<object>(obj.connect.WaitOne());
        }

        private static void connect_callback(IAsyncResult ar)
        {
            // retrieve the details from the connection information wrapper
            var obj = (HttpObject)ar.AsyncState;
            var client_socket = obj.socket;
            var client_id = obj.id;
            var hostname = obj.hostname;

            client_socket.EndConnect(ar);

            Console.WriteLine("{0} --> Socket connected to {1} ({2})", client_id, hostname, client_socket.RemoteEndPoint);

            // signal that the connection has been made 
            obj.connect.Set();
        }

        private static async Task send_wrapper(HttpObject obj, string data)
        {
            // convert the string data to byte data   
            var byteData = Encoding.ASCII.GetBytes(data);

            // begin sending the data to the server  
            obj.socket.BeginSend(byteData, 0, byteData.Length, 0, send_callback, obj);

            await Task.FromResult<object>(obj.send.WaitOne());
        }

        private static void send_callback(IAsyncResult ar)
        {
            var obj = (HttpObject)ar.AsyncState;
            var client_socket = obj.socket;
            var client_id = obj.id;

            var bytesSent = client_socket.EndSend(ar);
            Console.WriteLine("{0} --> Sent {1} bytes to server.", client_id, bytesSent);

            // signal that all bytes have been sent
            obj.send.Set();
        }

        private static async Task receive_wrapper(HttpObject obj)
        {
            // begin receiving the data from the server
            obj.socket.BeginReceive(obj.buffer, 0, HttpObject.BUFF_SIZE, 0, receive_callback, obj);

            await Task.FromResult<object>(obj.receive.WaitOne());
        }

        private static void receive_callback(IAsyncResult ar)
        {
            var obj = (HttpObject)ar.AsyncState;
            var client_socket = obj.socket;

            try
            {
                // read data from the remote device.  
                var bytesRead = client_socket.EndReceive(ar);

                // get from the buffer, a number of characters <= to the buffer size, and store it in the response
                obj.response.Append(Encoding.ASCII.GetString(obj.buffer, 0, bytesRead));

                // if the response header is not complete, get the next chunk of data
                if (!Utils.check_response_header(obj.response.ToString()))
                {
                    client_socket.BeginReceive(obj.buffer, 0, HttpObject.BUFF_SIZE, 0, receive_callback, obj);
                }
                else
                {
                    var response = Utils.get_response(obj.response.ToString());

                    // the custom header parser is being used to check if the data received so far has the length
                    // specified in the response headers
                    if (response.Length < Utils.get_content_length(obj.response.ToString()))
                    {
                        // if it isn't, than more data is to be retrieve
                        client_socket.BeginReceive(obj.buffer, 0, HttpObject.BUFF_SIZE, 0, receive_callback, obj);
                    }
                    else
                    {
                        // otherwise, all the data has been received  
                        // signal that all bytes have been received  
                        WebClient client = new WebClient();
                        string fileName = "C:\\Users\\Vasilica\\Desktop\\docs\\PDP\\PDP\\Lab4\\Lab4\\file";
                        string ext = ".cs";

                        string[] responses = obj.response.ToString().Split('\r', '\n');
                        string host = "http:/";
                        foreach (var line in responses)
                        {
                            string[] str = line.Split(':');

                            if (str[0] == "Location")
                            {
                                string[] aux = str[2].Split('/');
                                for (int i = 3; i < aux.Length; i++)
                                {
                                    host += "/";
                                    host += aux[i].Trim();
                                }
                                break;
                            }
                        }
                        idx++;
                        int file_idx = idx;
                        client.DownloadFile(host, fileName + file_idx.ToString() + ext);
                        obj.receive.Set();
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

            var tasks = new List<Task>();

            for (var i = 0; i < hosts.Count; i++)
            {
                tasks.Add(Task.Factory.StartNew(do_start, i));
            }

            Task.WaitAll(tasks.ToArray());
        }
    }
}
