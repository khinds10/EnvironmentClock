import Adafruit_DHT, subprocess
sensor = sensor = Adafruit_DHT.DHT11

# Example using a Raspberry Pi with DHT sensor - connected to GPIO16.
pin = 16


# get 10 readings and average, this is jumping around
humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
if humidity is not None and temperature is not None:
    temperature = temperature * 9/5 + 32
    temperature = int(temperature)
    humidity = int(humidity)
    print('Temp={0:0.1f}*C  Humidity={1:0.1f}%'.format(temperature, humidity))
    
subprocess.call(["./digole", "clear"])
subprocess.call(["./digole", "setColor", "255"])
subprocess.call(["./digole", "setRot90"])
subprocess.call(["./digole", "clear-night"])
subprocess.call(["./digole", "setFont", "51"])
subprocess.call(["./digole", "printxy_abs", "10", "170", "Clear for the hour"])
subprocess.call(["./digole", "printxy_abs", "150", "90", "LOW\n41*F"])
subprocess.call(["./digole", "setFont", "120"])

subprocess.call(["./digole", "printxy_abs", "150", "40", "41*F [45%]"])
subprocess.call(["./digole", "printxy_abs", "70", "220", "IN: " + str(temperature) + "* F [" + str(humidity) + " %]"])


#weather.api.kevinhinds.net


#{"latitude":40.7127,"longitude":-74.0059,"timezone":"America/New_York","offset":-5,
#"currently":{"time":1478918886,"summary":"Clear","icon":"clear-night",
#"nearestStormDistance":409,"nearestStormBearing":32,"precipIntensity":0,"precipProbability":0,"temperature":43.09,
#"apparentTemperature":36.41,"dewPoint":23.67,"humidity":0.46,"windSpeed":12.99,"windBearing":330,"visibility":10,
#"cloudCover":0.03,"pressure":1017.62,"ozone":288.1},"minutely":{"summary":"Clear for the hour.","icon":"clear-night","data":[{"time":1478918880,"precipIntensity":0,"precipProbability":0},{"time":1478918940,"precipIntensity":0,"precipProbability":0},
