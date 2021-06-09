#!/usr/bin/python
# Kevin Hinds http://www.kevinhinds.com
# License: GPL 2.0
import datetime as dt
import time, json, string, cgi, subprocess, json
import settings as settings
import Adafruit_DHT
import pprint
pp = pprint.PrettyPrinter(indent=4)
import requests

# Raspberry Pi with DHT sensor - connected to GPIO16 / Pin 36
sensor = sensor = Adafruit_DHT.DHT11
pin = 18

# get current date and time
date=dt.datetime.now()

# get 10 readings and average, in case the humidistat is inaccurate
count, readingCount, avgTemperature, avgHumidity = [ 0, 0, 0, 0 ]
while (count < 10):
    humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
    if humidity is not None and temperature is not None:
        avgTemperature = avgTemperature + temperature
        avgHumidity = avgHumidity + humidity
        readingCount = readingCount + 1
    count = count + 1
avgTemperature = avgTemperature / readingCount
insideTemperature = int(avgTemperature * 9/5 + 32)
avgHumidity = avgHumidity / readingCount
insideHumidity = int(avgHumidity)

# DHT11 adjust for this unit
insideTemperature = int(insideTemperature) - 4
insideHumidity = int(avgHumidity)

# get current forecast from location
weatherInfo = json.loads(subprocess.check_output(['curl', settings.weatherAPIURL]))
currentConditions = weatherInfo['current']
apparentTemperature = str(int(currentConditions['feels_like']))
humidity = str(int(currentConditions['humidity']))
windSpeed = str(int(currentConditions['wind_speed']))
cloudCover = str(int(currentConditions['clouds']))
summary = str(currentConditions['weather'][0]['description'])
summary = (summary[:27] + '...') if len(summary) > 29 else summary
summary = summary.title()

# conditions for the day
dailyConditions = weatherInfo['daily']
dailyConditions = dailyConditions[0]
apparentTemperatureMin = str(int(dailyConditions['temp']['min']))
apparentTemperatureMax = str(int(dailyConditions['temp']['max']))

# post to datahub
r = requests.post("http://" + settings.deviceLoggerAPI + "/api/log/", data={'device': 'weather-clock-red', 'value1': str(insideTemperature), 'value2': str(insideHumidity) , 'value3': str(apparentTemperature), 'value4': str(humidity), 'value5': str(summary)})
print(r.status_code, r.reason)
print(r.text)
