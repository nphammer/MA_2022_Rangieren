# Required Libraries are imported
import serial
import time
from enum import Enum

class Command(Enum):
    GO = 1
    TURNSTRAIGHT = 2
    TURNROUND = 3
    DECOUPLE = 4
    SETBACK = 5

# Arriving train; wagon C1 is the wagon closest to the locomotive.
train = ["C1","C2","C3","C4"]

 # The wagons to be filtered out
referenceTrain = ["C2","C3"]
track1 = []
track2 = []

 # Serial link is started
arduino = serial.Serial(port='COM4', baudrate=115200, timeout=.1)

# Function for sending commands to the Arduino
def sendCommand(cmd):
    
    arduino.write(bytes(str(cmd.value), 'utf-8'))

    while (arduino.in_waiting == 0):
        time.sleep(0.1)

    response = int(arduino.readline().decode())
    return response

# Function to determine the required position of the turnout.
# Thereby the list of the arriving train is shortened for each wagon
def decouple():
    if train[-1] in referenceTrain : track1.append( train.pop() ) ; sendCommand(Command.TURNSTRAIGHT)
    else : track2.append( train.pop() ) ; sendCommand(Command.TURNROUND)
    time.sleep(1)


# Wait until the Arduino is ready
print("Wait for Arduino to get ready...")
while (arduino.in_waiting == 0):
    time.sleep(0.1)
response = arduino.readline().decode('utf-8')

response = response.strip()
if (response != "Ready"):
    raise Exception("Wrong initial message from Arduino!")

print("Let's go!")

 # Command to start the train
sendCommand(Command.GO)

# As long as the train/list of the train still has wagons, the loop runs
# First the switch is set, then the wagon is uncoupled
while train :
    decouple()
    sendCommand(Command.DECOUPLE)
    time.sleep(2)

# Get wagons from track 1
sendCommand(Command.SETBACK)

# Check if all wagons are in the right place
print(track1)
print(track2)