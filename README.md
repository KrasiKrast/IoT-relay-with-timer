# IoT relay with timedelay
 IoT relay based on ESP8266 chip, with time delayed OFF.

This project is about IoT based relay with direct power line supply (220V).
It was developedwith with mindset for Bathroom FAN control, which is activated by the wall switch (220V), and is self swithed OFF after time.
There is a web application which allow to activate it and deactivate it manually, and also to set the time delay.
Next picture shows the web application:

![WebApp](https://user-images.githubusercontent.com/7886408/153831673-0235c424-69b5-452b-95c8-4875d357ca27.PNG)

The used hardware is a Chines relay with ESP8266, as showed on the next picture:
![IoTrelay](https://user-images.githubusercontent.com/7886408/153829923-29a67873-52ed-41e5-a563-c123d768af79.png)

To be able toconnect it directly to 220V we use cheap Chines tiny converter 220V/5Vdc, as showed on next picture;
![220V_5Vdc](https://user-images.githubusercontent.com/7886408/153830193-06c313ce-d19b-4166-8720-928812edca23.png)


This device can operate as WiFi AP and at the same time as WiFi Client. When operate as a client the WiFi nettwork to connect to it 
must be hardcoded inside together with the password for authentification. 

