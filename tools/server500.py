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
        self.send_error(500)
    def do_POST(self):
        self.send_error(500)
    def do_PUT(self):
        self.send_error(500)

httpd = http.server.HTTPServer(("", args.port), httphandler)
httpd.serve_forever()

