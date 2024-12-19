import socket

def start_tcp_server(host='0.0.0.0', port=8080, output_file='received_data.zip'):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen(1)
        print(f"Listening on {host}:{port}...")

        conn, addr = server_socket.accept()
        with conn:
            print(f"Connection established with {addr}")
            with open(output_file, 'wb') as f:
                while True:
                    data = conn.recv(4096)
                    if not data:
                        break
                    f.write(data)

            print(f"Data received and saved to {output_file}")

if __name__ == "__main__":
    start_tcp_server()