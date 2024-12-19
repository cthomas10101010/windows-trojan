#!/bin/bash

# Install required packages
echo "Installing Python, Netcat, and Git..."
apt-get update
apt-get install -y python3 python3-pip python3-venv netcat git

# --- Setup dataexfil-server ---

echo "Setting up dataexfil-server directory..."
mkdir -p dataexfil-server
cd dataexfil-server || exit

# Create and activate virtual environment
echo "Creating virtual environment for dataexfil-server..."
python3 -m venv venv
source venv/bin/activate

# Install required packages for dataexfil-server (none needed for basic socket use)
echo "Virtual environment for dataexfil-server is ready."

# Create TCP server script
cat <<EOF > tcp_server.py
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
EOF

# Deactivate virtual environment
deactivate
cd ..

# --- Setup webserver ---

echo "Setting up webserver directory..."
mkdir -p webserver
cd webserver || exit

# Create and activate virtual environment
echo "Creating virtual environment for webserver..."
python3 -m venv venv
source venv/bin/activate

# Install Flask
echo "Installing Flask in the webserver virtual environment..."
pip install Flask

# Deactivate virtual environment
deactivate
cd ..

echo "Setup complete. Directories 'dataexfil-server' and 'webserver' are ready with virtual environments."
