import serial
import time
import sys

if __name__ == '__main__':
    if len(sys.argv) > 1:
        x_pixel = sys.argv[1]
        y_pixel = sys.argv[2]

        left_wheel = 0
        right_wheel = 0

        # Turn CW
        if int(x_pixel) == 1 and int(y_pixel) == 0:
            right_wheel = -1000
            left_wheel = 1000
        # Turn CCW
        elif int(x_pixel) == -1 and int(y_pixel) == 0:
            right_wheel = 1000
            left_wheel = -1000
        # Drive Forward
        elif int(y_pixel) > 0:
            if int(x_pixel) == 0:
                right_wheel = 5*int(y_pixel) + 1000
                left_wheel = right_wheel
        elif int(y_pixel) < 0:
            if int(x_pixel) == 0:
                right_wheel = -5*int(y_pixel) - 1000
                left_wheel = right_wheel
        else:
            right_wheel = 0
            left_wheel = 0

        left_wheel = str(left_wheel)
        right_wheel = str(right_wheel)
        print("Setpoints: "+left_wheel+" "+right_wheel)

        # Send over UART
        USB_Port = '/dev/ttyUSB0'
        ser = serial.Serial(USB_Port, 115200, timeout=1)
        ser.flush()
        # Generate and send a command
        command = left_wheel+','+right_wheel+','
        ser.write(command.encode('utf-8'))
    else:
        print("ERROR: DriveInterface Invalid Args")
