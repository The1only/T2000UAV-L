import socket
import struct

MCAST_GRP = "239.255.0.1"   # your ESP32 multicast address
MCAST_PORT = 4210           # your ESP32 multicast port

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# Allow multiple listeners on the same machine
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

# Bind to all interfaces on the given port
sock.bind(("", MCAST_PORT))

# Ask the OS to join the multicast group
mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

print(f"Listening for multicast on {MCAST_GRP}:{MCAST_PORT} ...")

while True:
    data, addr = sock.recvfrom(1024)
    print(f"[{addr[0]}] {data.decode(errors='ignore')}")

