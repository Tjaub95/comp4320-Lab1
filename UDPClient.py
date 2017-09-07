#!/usr/bin/python


OUTPUT_STRING = """Response:
        Size:       {0} bytes
        Request ID: {1}
        Answer:     {2}
"""

from socket import *
import struct
import sys
import time

class UDPClient:
    req_id = 0

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = socket(AF_INET, SOCK_DGRAM)

    def cons_len(self, message):
        self.send_message(5, message)

        resp, addr = self.sock.recvfrom(2**12)
        rtml, rrid, rans = struct.unpack('<HHH',resp[:6])

        return rtml, rrid, rans

    def disemvowel(self, message):
        self.send_message(80, message)

        resp, addr = self.sock.recvfrom(2**12)
        rtml, rrid = struct.unpack('<HH', resp[:4])
        # Receive disemvoweled string
        rans = str(resp[4:])

        return rtml, rrid, rans

    def uppercasing(self, message):
        self.send_message(10, message)

        resp, addr = self.sock.recvfrom(2**12)
        rtml, rrid = struct.unpack('<HH', resp[:4])
        # Receive uppercased string
        rans = str(resp[4:])

        return rtml, rrid, rans

    def send_message(self, operation, message):
        total_message_len = 5 + len(message)
        req_id = UDPClient.req_id = UDPClient.req_id + 1
        header = struct.pack('!HHB', total_message_len, req_id, operation)
        message = str(header) + message

        sent = self.sock.sendto(message, (self.host, self.port))

        while sent < total_message_len:
            sent += self.sock.sendto(message[sent:], (self.host, self.port))

if __name__ == '__main__':
    client, host, port, operation, message = sys.argv
    port = int(port)
    operation = int(operation)
    client = UDPClient(host, port)
    if operation == 5:
        start = time.time()
        print "How many consonants are in \"{}\"?".format(message)
        result = client.cons_len(message)
        print OUTPUT_STRING.format(result[0], result[1], result[2])
        print "\tRound Trip Time: {}s".format(time.time()-start)
    elif operation == 10:
        start = time.time()
        print "Upper case the string \"{}\"".format(message)
        result = client.uppercasing(message)
        print OUTPUT_STRING.format(result[0], result[1], result[2])
        print "\tRound Trip Time: {}s".format(time.time()-start)
    elif operation == 80:
        start = time.time()
        print "Disemvowel the string \"{}\"".format(message)
        result = client.disemvowel(message)
        print OUTPUT_STRING.format(result[0], result[1], result[2])
        print "\tRound Trip Time: {}s".format(time.time()-start)
    else:
        print("--INVALID OPERATION REQUESTED--\n   User operation 85 or 170\n   to request vowel length\n      or disemvowelment")
