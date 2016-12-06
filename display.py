#!/usr/bin/python
# Kevin Hinds http://www.kevinhinds.com
# License: GPL 2.0
import datetime as dt
import time, json, string, cgi, subprocess, json
import settings as settings
import Adafruit_DHT

# Raspberry Pi with DHT sensor - connected to GPIO16 / Pin 36
sensor = sensor = Adafruit_DHT.DHT11
pin = 16

# begin the loop to get the current weather for display
while True:
    try:
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
        
        # clear and setup display to show basic info
        subprocess.call(["/home/pi/EnvironmentClock/digole", "clear"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setRot90"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", icon])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setFont", "18"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setColor", "255"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "printxy_abs", "10", "178", summary])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setFont", "51"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "printxy_abs", "150", "40", date.strftime('%a, %b %d')])
        
        # print the min daily temp in the evening and night, print the day max temp in the morning and daytime
        if dt.datetime.now().hour > 16 or dt.datetime.now().hour < 6:
            subprocess.call(["/home/pi/EnvironmentClock/digole", "setColor", "223"])
            subprocess.call(["/home/pi/EnvironmentClock/digole", "printxy_abs", "150", "130", "LOW\n" + apparentTemperatureMin + '*F'])
        else:
            subprocess.call(["/home/pi/EnvironmentClock/digole", "setColor", "250"])
            subprocess.call(["/home/pi/EnvironmentClock/digole", "printxy_abs", "150", "130", "HIGH\n" + apparentTemperatureMax + '*F'])
        
        # show indoor / outdoor temp
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setFont", "120"])
        
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setColor", "255"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "printxy_abs", "150", "85", apparentTemperature + "*F [" + humidity + "%]"])
        
        subprocess.call(["/home/pi/EnvironmentClock/digole", "setColor", "250"])
        subprocess.call(["/home/pi/EnvironmentClock/digole", "printxy_abs", "70", "230", "IN: " + str(insideTemperature) + "* F [" + str(insideHumidity) + " %]"])

        # wait 5 minutes
        time.sleep(300)

    except (Exception):
        # Network or other issue, wait 5 minutes
        time.sleep(300)
