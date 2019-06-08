import socket
import sys
import time
import select
import queue


class HttpServer(object):
    """ Basic socket server class using TCP"""

    def __init__(self, port=1234):
        self.port = port
        self.host = socket.gethostname()
        self.inputs = []
        self.outputs = []
        self.msg_q = {}

    def start(self):
        """ Starts new socket on specified port """
        self.localSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        print('#Socket created')
        try:
            self.localSocket.bind((self.host, self.port))
            print('#Bind successful')
        except Exception:
            print('#Bind failed')
            self.shutdown()
            sys.exit(1)

        try:
            self.inputs.append(self.localSocket)
            self.loop()
        except KeyboardInterrupt:
            self.shutdown()

    def shutdown(self):
        """ Shuts down the socket """
        try:
            self.localSocket.shutdown(socket.SHUT_RDWR)
        except Exception:
            pass  # Socket already closed

    def generate_headers(self, code, msgLength, contentType, refresh=0, url=None):
        """ Generates HTTP header. 
        
        Code        - response code (200, 400 or 404)
        msgLength   - the length of message body
        contentType - the Content-type header 
        [refresh]   - time (seconds) until browser refresh (does not work in command line)
        [url]       - redirect url for refresh
        """
        header = ''
        if code == 200:
            header += 'HTTP/1.1 200 OK\r\n'
        elif code == 405:
            header += 'HTTP/1.1 405 Method Not Allowed\r\n'
        else:
            header += 'HTTP/1.1 404 Not Found\r\n'

        curr_time = time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime())
        if refresh > 0:
            header += 'Refresh: {time}; url={url}\r\n'.format(time=refresh, url=url)
        header += 'Content-Type: {ctype}\r\n'.format(ctype=contentType)
        header += 'Content-length: {len}\r\n'.format(len=msgLength)
        header += 'Date: {now}\r\n'.format(now=curr_time)
        header += 'Server: IPK Python server. Author: Kamil Vojanec <xvojan00@stud.fit.vutbr.cz>\r\n'
        header += 'Connection: close\r\n\r\n'

        return header

    def generate_response(self, incomingHeader):
        """ Generates a HTTP message based on the incoming request header """
        headerList = incomingHeader.split(' ')
        message = ''
        body = ''
        contentType = ''
        accept = self.get_accept_header(incomingHeader)

        if accept == 'application/json':
            contentType = accept
        else:
            contentType = 'text/plain'

        if headerList[0] == 'GET':
            if headerList[1] == '/hostname':
                body = self.message_hostname(contentType)
                message = self.generate_headers(200, len(body), contentType)
                message += body
            elif headerList[1] == '/cpu-name':
                body = self.message_cputype(contentType)
                message = self.generate_headers(200, len(body), contentType)
                message += body
            elif headerList[1] == '/load':
                body = self.message_cpuload(contentType)
                message = self.generate_headers(200, len(body), contentType)
                message += body
            elif '/load?refresh=' in headerList[1]:
                timeToWait = headerList[1].split('=')[1]
                if not timeToWait:
                    message = self.generate_headers(404, 0, contentType)
                else:
                    body = self.message_cpuload(contentType)
                    message = self.generate_headers(200, len(body), contentType, int(timeToWait), 'http://' + str(self.host) + ':' + str(self.port) + '/load?refresh=' + str(timeToWait))
                    message += body
            else:
                message = self.generate_headers(404, 0, contentType)
        else:
            message = self.generate_headers(405, 0, contentType)

        return message

    def message_hostname(self, contentType):
        """ Generates message body with a hostname """
        if contentType == 'application/json':
            return '{'+'hostname:{host}'.format(host=socket.gethostname())+'}\r\n'
        else:
            return '{host}\r\n'.format(host=socket.gethostname())

    def message_cputype(self, contentType):
        """ Generates message body with cpu type """
        cpuName = 'not found'
        try:
            sysInfo = open('/proc/cpuinfo')
        except (FileExistsError, FileNotFoundError):
            raise
        sysInfoStr = sysInfo.readlines()
        for line in sysInfoStr:
            if 'model name' in line:
                cpuName = line.split(' ', 2)[2]
        sysInfo.close()

        if contentType == 'application/json':
            return '{'+'cputype:{cpu}'.format(cpu=cpuName)+'}\r\n'
        else:
            return cpuName

    def message_cpuload(self, contentType):
        """ Generates message body with calculated cpu load """
        try:
            f = open('/proc/stat')
        except (FileExistsError, FileNotFoundError):
            raise
        fields = [float(column) for column in f.readline().strip().split()[1:]]
        idle = fields[3]
        total = sum(fields)
        util = 100.0 * (1.0 - idle / total)
        util = '{util:.2f} %\r\n'.format(util=util)

        if contentType == 'application/json':
            return '{'+'cpuload:{load}'.format(cpu=util)+'}\r\n'
        else:
            return util

    def get_accept_header(self, incomingHeader):
        """ Parses the accept field from incoming HTTP request """
        accept = 'text/plain'
        for string in incomingHeader.splitlines():
            if 'Accept' in string:
                accept = string.split()[1]
        #print('Accept value is: {acc}'.format(acc=accept))
        return accept

    def parse_lines(self, data):
        """ Parses incoming HTTP request into lines """
        return data.splitlines()

    def loop(self):
        """ Main server loop using Select with queues to serve responses without blocking """

        self.localSocket.listen()

        while self.inputs:
            readable, writable, exceptional = select.select(
                self.inputs, self.outputs, self.inputs)

            # Handle inputs
            for s in readable:
                # Handle new connection
                if s is self.localSocket:
                    (client, address) = s.accept()
                    print('#Recieved connection from {addr}'.format(
                        addr=address))
                    client.setblocking(0)
                    self.inputs.append(client)
                    self.msg_q[client] = queue.Queue()
                # Handle existing connection
                else:
                    data = s.recv(1024).decode()
                    if data:  # Socket has data
                        print('#Received {d} from {c}'.format(
                            d=data, c=s.getpeername()))
                        message = self.generate_response(
                            self.parse_lines(data)[0])
                        self.msg_q[s].put(message.encode())
                        if s not in self.outputs:
                            self.outputs.append(s)
                    # Close socket without data
                    else:
                        print('#Closing connection: No data received')
                        if s in self.outputs:
                            self.outputs.remove(s)
                        self.inputs.remove(s)
                        s.close()
                        del self.msg_q[s]

            # Handle outputs
            for s in writable:
                try:
                    next_message = self.msg_q[s].get_nowait()
                except queue.Empty:
                    # No more messages
                    self.outputs.remove(s)
                else:
                    print('#Sending message {msg} to {addr}'.format(
                        msg=next_message.decode(), addr=s.getpeername()))
                    s.send(next_message)

            # Handle exceptions
            for s in exceptional:
                self.inputs.remove(s)
                if s in self.outputs:
                    self.outputs.remove(s)
                s.close()
                del self.msg_q[s]


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        arg = sys.argv[1]
        if arg.isdecimal():
            arg = int(arg)
            if arg >= 0 and arg <= 65535:
                test = HttpServer(int(sys.argv[1]))
                test.start()
            else:
                print(
                    'ERROR: Invalid port number, please enter a decimal number between 0 and 65535')
                sys.exit(1)
        else:
            print('ERROR: Invalid command line argument')
            sys.exit(1)
    else:
        print('ERROR: No command line argument')
        sys.exit(1)
