import serial
import time
import sys

if __name__ == '__main__':
    if len(sys.argv) > 1:
        left_wheel = sys.argv[1]
        right_wheel = sys.argv[2]

        # Send over UART
        USB_Port = '/dev/ttyUSB0'
        ser = serial.Serial(USB_Port, 115200, timeout=1)
        ser.flush()
        # Generate and send a command
        command = left_wheel+','+right_wheel+','
        ser.write(command.encode('utf-8'))
    else:
        print("ERROR: DriveInterface Invalid Args")
