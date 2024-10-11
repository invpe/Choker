# Choker : Serial2Wireshark interface
# https://github.com/invpe/Choker/
import serial
import struct
import subprocess
import time
import argparse

# PCAP Global Header fields
magic_number = 0xa1b2c3d4  # Magic number for PCAP files
version_major = 2           # Major version number for PCAP
version_minor = 4           # Minor version number for PCAP
thiszone = 0                # GMT to local correction
sigfigs = 0                 # Accuracy of timestamps
snaplen = 65535             # Maximum length of captured packets
network = 1                 # Data link type (1 = Ethernet)

# Set up command-line argument parsing
parser = argparse.ArgumentParser(description="Capture serial data and stream it to Wireshark.")
parser.add_argument("serial_port", help="The serial port to read data from (e.g., /dev/ttyUSB0)")
parser.add_argument("--baud_rate", type=int, default=1500000, help="Baud rate for the serial connection (default: 1500000)")
args = parser.parse_args()

# Replace with your serial port and baud rate from arguments
SERIAL_PORT = args.serial_port
BAUD_RATE = args.baud_rate
WIRESHARK_CMD = "wireshark"

# Open the serial port in blocking mode
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)

print("Starting Wireshark...")
# Start Wireshark with stdin input from the script
wireshark_process = subprocess.Popen(
    [WIRESHARK_CMD, "-k", "-i", "-"],  # Read from stdin
    stdin=subprocess.PIPE
)
print("Wireshark started. Waiting for data...")

# Construct the PCAP global header
global_header = struct.pack(
    '<IHHIIII',
    magic_number,  # Magic number
    version_major,  # Major version number
    version_minor,  # Minor version number
    thiszone,  # GMT to local correction
    sigfigs,  # Accuracy of timestamps
    snaplen,  # Max length of captured packets, in octets
    network  # Data link type
)

# Send the global header directly to Wireshark
wireshark_process.stdin.write(global_header)
wireshark_process.stdin.flush()

print("Global header sent to Wireshark.")

# Read and process packets in a loop
try:
    while True:
        # Read the length of the next packet (4 bytes) in a blocking manner
        length_data = b''
        while len(length_data) < 4:
            length_data += ser.read(4 - len(length_data))

        # Extract packet length from the 4-byte length header
        packet_length = struct.unpack('<I', length_data)[0]

        # Validate packet length
        if packet_length <= 0 or packet_length > 5000:
            print(f"Invalid packet length: {packet_length}. Skipping...")
            continue

        # Calculate remaining bytes to read (excluding the 4-byte length header)
        remaining_bytes = packet_length  

        # Read the rest of the packet (remaining bytes)
        data = b''
        while len(data) < remaining_bytes:
            data += ser.read(remaining_bytes - len(data))

        # Check if we have read all expected bytes
        if len(data) == remaining_bytes:
            print(f"Packet received (length: {packet_length}): {data[:20].hex()}...")  # Debug output

            # Get the current timestamp for the packet header
            ts_sec = int(time.time())
            ts_usec = int((time.time() - ts_sec) * 1e6)

            # Construct the PCAP packet header
            incl_len = len(data)
            orig_len = len(data)
            packet_header = struct.pack(
                '<IIII',
                ts_sec,    # Timestamp seconds
                ts_usec,   # Timestamp microseconds
                incl_len,  # Number of octets of packet saved in file
                orig_len   # Actual length of packet
            )

            # Write the packet header and data to Wireshark
            wireshark_process.stdin.write(packet_header)
            wireshark_process.stdin.write(data)
            wireshark_process.stdin.flush()

        else:
            print(f"Incomplete packet read. Expected {remaining_bytes} bytes, but got {len(data)} bytes.")

except KeyboardInterrupt:
    print("\nInterrupted by user. Exiting...")
except serial.SerialException as e:
    print(f"Serial error: {e}")
except Exception as e:
    print(f"Unexpected error: {e}")
finally:
    ser.close()
    print("Serial port closed.")
