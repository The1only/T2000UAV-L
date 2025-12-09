# ipconfig getifaddr en0   # usually Wi-Fi
import socket
import time

MCAST_GRP = "239.255.0.1"
MCAST_PORT = 4210

# CHANGE THIS to whatever ipconfig printed (your Wi-Fi IP)
INTERFACE_IP = "192.168.10.144"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

# Tell the OS which interface to use for multicast
sock.setsockopt(
    socket.IPPROTO_IP,
    socket.IP_MULTICAST_IF,
    socket.inet_aton(INTERFACE_IP)
)

# TTL 1 = stay in local network
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)

print(f"Sending multicast to {MCAST_GRP}:{MCAST_PORT} from {INTERFACE_IP}")

while True:
    msg = b"Boot"
    sock.sendto(msg, (MCAST_GRP, MCAST_PORT))
    print("Sent:", msg)
    time.sleep(5)    
    sock.sendto(msg, (MCAST_GRP, MCAST_PORT))
    print("Sent:", msg)
    time.sleep(5)
    sock.sendto(msg, (MCAST_GRP, MCAST_PORT))
    print("Sent:", msg)
    time.sleep(8)
