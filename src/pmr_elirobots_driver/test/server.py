import json
from http.server import BaseHTTPRequestHandler, HTTPServer


class JsonRpcHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        print("do POST")

        content_length = int(self.headers["Content-Length"])
        post_data = self.rfile.read(content_length)

        try:
            request = json.loads(post_data)

            # Handle batch requests
            if isinstance(request, list):
                responses = []
                for req in request:
                    responses.append(self.handle_request(req))
                response_data = json.dumps(responses)
            else:
                response_data = json.dumps(self.handle_request(request))

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.end_headers()
            self.wfile.write(response_data.encode())

        except Exception as e:
            self.send_response(400)
            self.end_headers()
            self.wfile.write(str(e).encode())

    @staticmethod
    def add(params: dict):
        return params.get("a", 0) + params.get("b", 0)

    @staticmethod
    def subtract(params: dict):
        return params.get("a", 0) + params.get("b", 0)

    @staticmethod
    def server_info(params: dict):
        return {"name": "JSON-RPC Test Server", "version": "1.0"}

    @staticmethod
    def servo_status(param: dict):
        return True

    def handle_request(self, request):
        print("handle request")

        method = request.get("method")
        params = request.get("params", {})
        request_id = request.get("id")

        response = {"jsonrpc": "2.0"}

        methods_dict = {
            "add": self.add,
            "subtract": self.subtract,
            "getServerInfo": self.server_info,
            "get_servo_status": (lambda _: True),
            "get_joint_pos": (lambda _: [1.5] * 6),
        }

        if method in methods_dict.keys():
            response["result"] = methods_dict[method](params)
        else:
            response["error"] = {"code": -32601, "message": "Method not found"}  # type: ignore

        if request_id is not None:
            response["id"] = request_id

        return response


def run_server(port=8080):
    server = HTTPServer(("localhost", port), JsonRpcHandler)
    print(f"JSON-RPC server running on port {port}")
    server.serve_forever()


if __name__ == "__main__":
    run_server()
