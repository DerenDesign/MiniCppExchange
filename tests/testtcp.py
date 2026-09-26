import socket
import struct

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('127.0.0.1', 12345))

# type=0 (AddOrder), orderId=1, price=100.0, quantity=50.0, side=0 (BUY), then 4 bytes padding
msg = struct.pack('<iiddi4x', 0, 1, 100.0, 50.0, 0)

print("Packed size:", len(msg))  # Expected 32 bytes

s.send(msg)
response = s.recv(1024)
print("Received:", response)