# For Adafruit T0 Micro
# Baud 9600
# SAMMONS_MULTI_02

import board
from digitalio import DigitalInOut, Direction, Pull
from analogio import AnalogOut, AnalogIn
import adafruit_dotstar as dotstar
import busio
import time

# built-in LED
dot = dotstar.DotStar(board.APA102_SCK, board.APA102_MOSI, 1, brightness=0.2)
dot[0] = (0, 0, 128)

# read voltage
def getVoltage(pin):
    return (pin.value * 3.3) / 65536


button_pin = AnalogIn(board.D2)
dial_pin = AnalogIn(board.D1)

ble = busio.UART(board.D4, board.D3, baudrate=9600)

# D0 is soldered but nothing is bound to it

def atcmd(cmd):
    ble.write(cmd)
    read_ok = ble.read(32)
    print("read", read_ok)

# atcmd("AT")
# atcmd("AT+NAMESAMMONS_MULTI_02")
# atcmd("AT+PIN1234")
# atcmd("AT+VERSION")
# atcmd("AT+HELP")
# atcmd("AT")

pressed = False

dot_on = False

# observed
max = 329
min = 10

name = bytearray("SAMMONS_BUTTON_2")
prev = getVoltage(dial_pin)
readings = []
avg = prev
reading_count = 12
def get_dial_value():
    sum = 0
    for v in readings:
        sum += (v * 100)
    sum = sum / len(readings)
    sum = round(sum)
    if sum < min:
        sum = min
    sum = sum - min
    sum = ((sum / (max - min)) * 100)
    return sum

last_ping = 0

while True:
    last_ping = last_ping + 1
    if last_ping > 100:
        last_ping = 0
        # send data every so often in case it
        # helps keep latency low
        ble.write(name + ":PING:BTN_1:DIAL_1")
    if not dot_on:
        dot[0] = (0, 128, 0)
    button = getVoltage(button_pin)
    dial = getVoltage(dial_pin)
    readings.append(dial)
    if len(readings) > reading_count:
        readings.remove(readings[0])
    avg = get_dial_value()
    if button > 3 and not pressed:
        last_ping = 0
        ble.write(name + ":BTN_1[1]\n")
        pressed = True
    if button < 3 and pressed:
        last_ping = 0
        ble.write(name + ":BTN_1[0]\n")
        pressed = False
        # handle fluctuations, there is quite a lot of noise
    if abs(avg - prev) > 0.7:
        last_ping = 0
        ble.write(name + ":DIAL_1[" + str((round(avg))) + "]")
        prev = avg
    time.sleep(0.02)