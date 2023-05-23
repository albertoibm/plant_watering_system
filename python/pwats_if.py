import time
import serial
import threading

class PwatsIf:
    def __init__(self, serial_port):
        self.serial_port = serial_port
        self.running = False
        self.ser = None
        self.serial_thread = None

    def open(self):
        self.ser = serial.Serial(self.serial_port, 9600)  # Replace with the real serial port and baud rate

    def read_serial(self):
        # Open the serial port
        self.open()

        # Continuously read inputs from the serial port while running is True
        while self.running:
            if self.ser.in_waiting > 0:
                # Read the incoming data from the serial port
                data = self.ser.readline().decode().strip()
                print(data)

                # React to the incoming data (placeholder)
                if data.startswith('Moisture'):
                    # Perform actions for reading moisture level
                    pass
            time.sleep(0.05)

    def start(self):
        # Create a thread for reading serial inputs
        self.serial_thread = threading.Thread(target=self.read_serial)

        # Set running to True
        self.running = True

        # Start the serial thread
        self.serial_thread.start()

    def stop(self):
        # Set running to False to stop the loop
        self.running = False

        # Wait for the serial thread to finish
        self.serial_thread.join()

        # Close the serial port
        self.ser.close()
        self.ser = None

    def add(self, analog_pin, digital_pin, dry_threshold, pot_diameter):
        # Perform actions for adding a pot (placeholder)
        command = f"add,{analog_pin},{digital_pin},{dry_threshold},{pot_diameter}\n"
        self.ser.write(command.encode())

    def delete(self, pot_id):
        # Perform actions for deleting a pot (placeholder)
        command = f"del,{pot_id}\n"
        self.ser.write(command.encode())

    def read(self, pot_id):
        # Perform actions for reading pot details (placeholder)
        command = f"read,{pot_id}\n"
        self.ser.write(command.encode())

    def moisture(self, pot_id):
        # Perform actions for reading moisture level (placeholder)
        command = f"moisture,{pot_id}\n"
        self.ser.write(command.encode())

    def water(self, pot_id, water_volume):
        # Perform actions for watering a pot (placeholder)
        command = f"water,{pot_id},{water_volume}\n"
        self.ser.write(command.encode())

# Create an instance of PwatsIf
pwats_if = PwatsIf('/dev/ttyUSB0')  # Replace with your actual serial port

# Start the PwatsIf instance
pwats_if.start()

# Keep the program running until user interrupts (e.g., Ctrl+C)
try:
    while True:
        user_input = input('> ')

        # Split the user input into command and arguments
        command, *args = user_input.split()

        # Evaluate the command and call the corresponding function
        if command == 'add':
            if len(args) == 4:
                try:
                    analog_pin, digital_pin, dry_threshold, pot_diameter = map(int, args)
                    pwats_if.add(analog_pin, digital_pin, dry_threshold, pot_diameter)
                except:
                    print('Invalid number of arguments for add command.')

            else:
                print('Invalid number of arguments for add command.')
        elif command == 'del':
            if len(args) == 1:
                try:
                    pot_id = int(args[0])
                    pwats_if.delete(pot_id)
                except:
                    print('Invalid number of arguments for del command.')
            else:
                print('Invalid number of arguments for del command.')
        elif command == 'read':
            if len(args) == 1:
                try:
                    pot_id = int(args[0])
                    pwats_if.read(pot_id)
                except:
                    print('Invalid number of arguments for read command.')
            else:
                print('Invalid number of arguments for read command.')
        elif command.startswith('moist'):
            if len(args) == 1:
                try:
                    pot_id = int(args[0])
                    pwats_if.moisture(pot_id)
                except:
                    print('Invalid number of arguments for moisture command.')
            else:
                print('Invalid number of arguments for moisture command.')
        elif command.startswith('wat'):
            if len(args) == 2:
                try:
                    pot_id, water_volume = map(int, args)
                    pwats_if.water(pot_id, water_volume)
                except:
                    print('Invalid number of arguments for water command.')
            else:
                print('Invalid number of arguments for water command.')
        elif command == 'exit':
            # Stop the PwatsIf instance and break the loop
            pwats_if.stop()
            break
        else:
            print('Invalid command. Please try again.')

except KeyboardInterrupt:
    # Stop the PwatsIf instance
    pwats_if.stop()
