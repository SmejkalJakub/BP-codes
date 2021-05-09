# Examples of Node-RED flows for a smart home

This repository contains an example flows for each room of a small home.

## Node-RED packages used in the flows

```sh
npm install @fetchbot/node-red-contrib-ikea-home-smart --save
npm install node-red-contrib-blynk-ws --save
npm install node-red-dashboard --save
npm install node-red-node-mysql --save
npm install https://github.com/hardwario/node-red-contrib-hardwario-power-module --save
```

## Flows

Flows are separated for a better organization. Each flow serves one room or one purpose. 

If some device is a bit to complicated so it will expand the room flow to much it is separated. (Thermostat With QR terminal)

### Home

General flow that serves for sending general informations to the other flows (rooms).

### Living Room

Nodes already in the flow in the example take care about motion detection. Next they take care about getting the temperature and voltage on the hardware device. All the data from the device are send to the dashboard and Blynk app and also into the database. 

In case of more devices presented the ``node/home/living-room`` prefix should be kept.

### Kitchen
Nodes in the included example takes care about the same things as the previous flow because the hardware device used is the same. 

In case of more devices presented the ``node/home/kitchen`` prefix should be kept.

### Bedroom
This example flow takes care about the things same as the previous examples. Also this example takes care about getting the CO2 concentration and relative humidity and send it to the dashboard and the database. 

In case of more devices presented the ``node/home/bedroom`` prefix should be kept.

### Workshop
This example flow takes care about getting the temperature, relative humidity, atmospheric pressure and VOC(Volatile Organic Compound). Also there is a possibility to control the smart LED strip and power relay. 

In case of more devices presented the ``node/climate-with-led-encoder:0`` prefix should be kept. 

### Balcony
This example flow takes care about getting the temperature, relative humidity, atmospheric pressure, illuminanceile Organic Compound) and voltage on the hardware device. All the data from the device are send to the dashboard and Blynk app and also into the database. 

In case of more devices presented the ``node/home/balcony`` prefix should be kept. 

### Bathroom
This example takes care about getting the temperature and voltage on the hardware device. On top of that there is an alarm for a water leak detection. All the data from the device are send to the dashboard and Blynk app and also into the database. 

In case of more devices presented the ``node/home/bathroom`` prefix should be kept.  

### Garage
This example shows how the garage can be set up. The flow takes care about temperature, humidity and VOC. Next to that there is a thermostat and movement sensor that can send the alarm.

In case of more devices presented the ``node/garage/`` prefix should be kept.  


### IKEA Smart Lights
This flow shows example on how to use Node-RED to control smart lights from IKEA. For correctness the lights that are in a specific room should be in a flow for that room. The flow takes care about getting the lights state and sending it to the Blynk app. It also gets the mqtt messages and set the desired state to the light.

### Thermostat With QR terminal
This is an example flow for specific device first because there is a lot of nodes just for this device and because the device is meant to be portable. This flow takes care about getting temperature, set-point for the temperature, button presses and voltage from the hardware thermostat device.

Another possibility of this flow is setting the QR code data and sending them to the hardware device for it to be shown on the LCD monitor.

In case of more devices presented the ``node/home/thermostat-with-qr-terminal`` prefix should be kept.  