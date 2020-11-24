using System;
using System.Collections.Generic;
using System.Text;

namespace Lab4
{
    class Utils
    {
        public static readonly int HTTP_PORT = 80;

        public static string get_request_string(string hostname, string endpoint)
        {
            return "GET " + endpoint + " HTTP/1.1\r\n" +
                   "Host: " + hostname + "\r\n" +
                   "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36\r\n" +
                   "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,#1#*;q=0.8\r\n" +
                   "Accept-Language: en-US,en;q=0.9,ro;q=0.8\r\n" +
                   "Accept-Encoding: gzip, deflate\r\n" +
                   "Connection: keep-alive\r\n" +
                   "Upgrade-Insecure-Requests: 1\r\n" +
                   "Pragma: no-cache\r\n" +
                   "Cache-Control: no-cache\r\n" +
                   "Content-Length: 0\r\n\r\n";
        }

        public static string get_response(string response)
        {
            var response_parts = response.Split(new[] { "\r\n\r\n" }, StringSplitOptions.RemoveEmptyEntries);

            return response_parts.Length > 1 ? response_parts[1] : "";
        }

        public static bool check_response_header(string response)
        {
            return response.Contains("\r\n\r\n");
        }

        public static int get_content_length(string response)
        {
            var length = 0;
            var lines = response.Split('\r', '\n');

            foreach (var line in lines)
            {
                var aux = line.Split(':');

                if (aux[0].CompareTo("Content-Length") == 0)
                {
                    length = int.Parse(aux[1]);
                }
            }

            return length;
        }

    }
}
