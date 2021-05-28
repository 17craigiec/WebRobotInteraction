import serial
import math
import time
import sys

if __name__ == '__main__':
    if len(sys.argv) > 1:
        x_input = sys.argv[1]
        y_input = sys.argv[2]

        # Special Command Characters
        command_chars = "EDH"
        # Arm Geometry
        d1 = 20
        d2 = 25
        # Resting pose used for x, y offsets
        x_offset = 4.093196
        y_offset = 5.321752
        t1_offset = -0.07854
        t2_offset =  0.27925
        # Ticks per radian
        t_per_radian = 18907.607
        # Pixel to axis conversion
        x_pixels_per_inch = 18.0/200
        y_pixels_per_inch = 33.0/300

        # Send over UART
        USB_Port = '/dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0'
        # USB_Port = '/dev/serial/by-id/usb-Silicon_Labs_CP2102N_USB_to_UART_Bridge_Controller_b8bab87b9b68e911bb126ad63ca6d3af-if00-port0'
        ser = serial.Serial(port=USB_Port, baudrate=115200, timeout=1)
        ser.flush()

        # Check for state commands
        if(sys.argv[1] in command_chars or sys.argv[2] in command_chars):
            # Generate and send a command
            command = x_input+','+y_input+','
            print_str = command.encode('utf-8')
            ser.write(print_str)
        # Otherwise Run Inverse Kinematics
        else:
            px = float(sys.argv[1])*x_pixels_per_inch + x_offset
            py = float(sys.argv[2])*y_pixels_per_inch + y_offset

            print("X: "+str(float(sys.argv[1])*x_pixels_per_inch)+"   Y: "+str(float(sys.argv[2])*y_pixels_per_inch))

            # Solve for theta 2
            t2_xcomp = ( math.pow(px, 2)+math.pow(py, 2)-math.pow(d1, 2)-math.pow(d2, 2) )/( 2*d1*d2 )
            t2_ycomp = -1*math.sqrt( 1 - math.pow(t2_xcomp, 2) )
            theta_2 = math.atan2(t2_ycomp, t2_xcomp)

            # Solve for theta 1
            k1 = d1 + d2*math.cos(theta_2)
            k2 = d2*math.sin(theta_2)
            theta_1 = math.atan2(py, px)-math.atan2(k2, k1)

            # Redefine the angles to act as the correct output
            # Alpha (setpoint of the lower pivot)
            alpha = math.pi - theta_1
            # Beta (setpoint of the upper pivot)
            beta = theta_1 + theta_2

            # Calculate setpoints and transform them into ticks
            t1_setpoint = int((alpha - t1_offset)*t_per_radian)
            t2_setpoint = int((beta - t2_offset)*t_per_radian)

            # Edge Case
            if(float(sys.argv[1]) == 0 and float(sys.argv[2]) == 0):
                t1_setpoint = 1
                t2_setpoint = 1

            t1_setpoint = str(t1_setpoint)
            t2_setpoint = str(t2_setpoint)

            command = t1_setpoint+','+t2_setpoint+','
            print_str = command.encode('utf-8')
            ser.write(print_str)

            print("Theta1: "+str(theta_1)+"   Theta2: "+str(theta_2))
            print("Alpha: "+str(alpha*180/math.pi)+"   Beta: "+str(beta*180/math.pi))

        # What is arduino saying
        for i in range(2):
            data = ser.readline();
            print(data)
        ser.flush()
    else:
        print("ERROR: DriveInterface Invalid Args")
