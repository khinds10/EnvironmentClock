#!/usr/bin/python
# Kevin Hinds http://www.kevinhinds.com
# License: GPL 2.0
import datetime as dt
import time, json, string, cgi, subprocess, json
import settings as settings
import Adafruit_DHT

# Raspberry Pi with DHT sensor - connected to GPIO16 / Pin 36
sensor = sensor = Adafruit_DHT.DHT11
pin = 18

def getIcon(openweatherCode):
    """get digole icon name from openweather map icon code"""
    if openweatherCode == '01d':
        return "clear-day"
    elif openweatherCode == '01n':
        return "clear-night"
    elif openweatherCode == '03d':
        return "cloudy"
    elif openweatherCode == '03n':
        return "cloudy"
    elif openweatherCode == '02d':
        return "partly-cloudy-day"
    elif openweatherCode == '04d':
        return "partly-cloudy-day"
    elif openweatherCode == '02n':
        return "partly-cloudy-night"
    elif openweatherCode == '04n':
        return "partly-cloudy-night"
    elif openweatherCode == '50d':
        return "fog"
    elif openweatherCode == '50n':
        return "fog"
    elif openweatherCode == '09d':
        return "rain"
    elif openweatherCode == '09n':
        return "rain"
    elif openweatherCode == '10d':
        return "rain"
    elif openweatherCode == '10n':
        return "rain"
    elif openweatherCode == '11d':
        return "rain"
    elif openweatherCode == '11n':
        return "rain"
    elif openweatherCode == '13d':
        return "snow"
    elif openweatherCode == '13n':
        return "snow" 
    else:
        return "cloudy"

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
        icon = getIcon(currentConditions['weather'][0]['icon'])
        summary = str(currentConditions['weather'][0]['description'])
        summary = (summary[:27] + '...') if len(summary) > 29 else summary
        summary = summary.title()

        # conditions for the day
        dailyConditions = weatherInfo['daily']
        dailyConditions = dailyConditions[0]
        apparentTemperatureMin = str(int(dailyConditions['temp']['min']))
        apparentTemperatureMax = str(int(dailyConditions['temp']['max']))
        
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
            subprocess.call(["/home/pi/EnvironmentClock/digole", "setColor", "222"])
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
