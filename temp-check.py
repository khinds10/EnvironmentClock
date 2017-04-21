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
pin = 16

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

# get current forecast from location
weatherInfo = json.loads(subprocess.check_output(['curl', settings.weatherAPIURL + settings.weatherAPIKey + '/' + str(settings.latitude) + ',' + str(settings.longitude) + '?lang=en']))
currentConditions = weatherInfo['currently']
icon = str(currentConditions['icon'])
apparentTemperature = str(int(currentConditions['apparentTemperature']))
humidity = str(int(currentConditions['humidity'] * 100))
windSpeed = str(int(currentConditions['windSpeed']))
cloudCover = str(int(currentConditions['cloudCover'] * 100))
precipProbability = str(int(currentConditions['precipProbability'] * 100))

# minutely conditions, limit the characters to 30 in the summary
minutelyConditions = weatherInfo['minutely']
summary = str(minutelyConditions['summary'])
summary = (summary[:27] + '...') if len(summary) > 29 else summary

# conditions for the day
dailyConditions = weatherInfo['daily']
dailyConditions = dailyConditions['data'][0]
apparentTemperatureMin = str(int(dailyConditions['apparentTemperatureMin']))
apparentTemperatureMax = str(int(dailyConditions['apparentTemperatureMax']))

# post to datahub
r = requests.post("http://" + settings.deviceLoggerAPI + "/api/log/", data={'device': 'weather-clock', 'value1': str(insideTemperature), 'value2': str(insideHumidity) , 'value3': str(currentConditions['apparentTemperature']), 'value4': str(int(currentConditions['humidity'] * 100)), 'value5': str(summary)})
print(r.status_code, r.reason)
print(r.text)
