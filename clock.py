#!/usr/bin/python
# Kevin Hinds http://www.kevinhinds.com
# License: GPL 2.0
import time
import datetime
from Adafruit_LED_Backpack import SevenSegment

# 7 Segment Clock on i2c on address 0x70
segment = SevenSegment.SevenSegment(address=0x70)

# Initialize the display. Must be called once before using the display.
segment.begin()
segment.set_brightness(3)

# Continually update the time on a 4 char, 7-segment display
while(True):

  now = datetime.datetime.now()
  hour = now.hour
  
  # convert to 12 hour time
  if (hour > 12):
    hour = hour - 12
  
  minute = now.minute
  second = now.second
  segment.clear()
  
  # Set hours
  segment.set_digit(0, int(hour / 10)) # Tens
  segment.set_digit(1, hour % 10) # Ones
  
  # Set minutes
  segment.set_digit(2, int(minute / 10)) # Tens
  segment.set_digit(3, minute % 10) # Ones
  
  # Toggle colon
  segment.set_colon(second % 2) # Toggle colon at 1Hz

  # Write the display buffer to the hardware.  This must be called to update the actual display LEDs.
  segment.write_display()

  # Wait a quarter second (less than 1 second to prevent colon blinking)
  time.sleep(0.25)
