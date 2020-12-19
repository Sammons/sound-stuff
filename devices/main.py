# For Adafruit T0 Micro
# Baud 9600
# Pin 1 and 2 are bluetooth (Transmit, Read)
# Pin 3 is the button with voltages 0-3.3

import board
from digitalio import DigitalInOut, Direction, Pull
from analogio import AnalogOut, AnalogIn
import adafruit_dotstar as dotstar
import busio
import time

# built-in LED
dot = dotstar.DotStar(board.APA102_SCK, board.APA102_MOSI, 1, brightness=0.2)
dot[0] = (0, 0, 128)

# read button
def getVoltage(pin):
    return (pin.value * 3.3) / 65536


enabled_pin = AnalogIn(board.D3)

ble = busio.UART(board.D0, board.D2, baudrate=9600)

pressed = False

dot_on = False

registered = False

incoming = bytearray()
name = bytearray("SAMMONS_BUTTON_1")
ble_sep = bytearray("^J")
while not registered:
  data = ble.read(32)
  if data is None:
      ble.write(name + "\n")
      print("attempting registration")
  time.sleep(1)
  data = ble.read(32)
  if data is not None:
      next = bytearray(data)
  else:
      next = None
  if next is not None:
      if name in next:
          next = next[len(name):]
      if ble_sep in next:
          next = next[len(ble_sep):]
      if len(next) > 0:
        incoming += next
      if incoming[-1:] == bytearray("\n"):
          id = bytearray()
          for i in range(len(incoming)):
              b = incoming[i:i+1]
              if b not in bytearray('\r\n'):
                  id += b
          ble.write("OK")
          ble.write(id + bytearray('\n'))
          incoming = bytearray()
          registered = True
          print("understood, I am", str(id))
      else:
          incoming  = bytearray()

while True:
    if not dot_on:
        dot[0] = (0, 128, 0)
    button = getVoltage(enabled_pin)
    if button > 3 and not pressed:
        ble.write("BTN[1]\n")
        pressed = True
    if button < 3 and pressed:
        ble.write("BTN[0]\n")
        pressed = False 
    time.sleep(0.01)