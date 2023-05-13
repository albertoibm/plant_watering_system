import serial
import time

# Serial port configuration
port = '/dev/ttyUSB0'  # Replace with your actual serial port
baud_rate = 9600

# Open the serial port
ser = serial.Serial(port, baud_rate)
time.sleep(2)  # Allow time for the connection to establish

# File configuration
output_file = 'sensor_data.csv'

# Main loop to read data from serial and write to file
with open(output_file, 'w') as file:
    file.write('Timestamp,Value\n')  # Write header row

    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode().strip()  # Read a line from serial
            timestamp, value = line.split(',')  # Split timestamp and value
            file.write(f'{timestamp},{value}\n')  # Write data to file
            file.flush()  # Flush buffer to ensure immediate write

# Close the serial port
ser.close()
