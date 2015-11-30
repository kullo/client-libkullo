#!/usr/bin/env python3

import argparse
import http.server

parser = argparse.ArgumentParser(
             description='Dummy HTTP server that returns the same HTTP status code for every request')
parser.add_argument('--port',
                    type=int,
                    default=8000,
                    help='The port (default: 8000)')
args, unknown = parser.parse_known_args()

print("Starting server at localhost:%d" % args.port)

class httphandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        self.wfile.write("Thanks".encode("utf-8"))
        print(self.headers)
        length = int(self.headers['content-length']) if self.headers['content-length'] else 0
        print(self.rfile.read(length))

    def do_POST(self):
        self.send_response(200)
        self.end_headers()
        self.wfile.write("Thanks".encode("utf-8"))
        print(self.headers)
        length = int(self.headers['content-length']) if self.headers['content-length'] else 0
        print(self.rfile.read(length))

httpd = http.server.HTTPServer(("", args.port), httphandler)
httpd.serve_forever()

