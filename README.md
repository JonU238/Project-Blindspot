# Project Blindspot
Project Blindspot got its name as during the 2024 solar eclipse I was living in the very cloudy city of Rochester NY. The main goal of the mission was to image the eclipse so I didn't miss it due to clouds. Dean and I worked on this project together. I did much of the administrative work ie. securing funding, communicating with profs and using the funds to buy parts. Both Dean and I had a hand in design, I did the circuits and some of the structure while Dean picked cameras and also worked on structures.

A lovely [flight recap video](https://www.youtube.com/watch?v=tN8dJ-07hHI) can be found on  Deans' Youtube channel.

## System overview

![Blindspot Overview diagram](https://github.com/JonU238/Project-Blindspot/assets/59961512/49a4dd57-f754-4b26-a829-ec10cf177af1)

### The balloon
The balloon is a 800g Kaymont balloon, It was picked for its reliablity and was a very safe bet incase we went over our weight budget.

### The parachute
The parachute was a modified parachute recovered from a [NWS radiosonde launch](https://www.weather.gov/upperair/radiosonde) it was chosen over the alternative parachutes we were considering because we already owned it and we had consulted a few other HAB launchers who have had good experiences with them.


### Radiosonde Trackers
Radiosondes are small devices used to measure current atmospheric conditions to better predict meteorological systems. The NWS station in Buffalo NY launches the RS41. This Sonde has firmware written for it that allows us to reprogram them with our callsigns on the 70cm ham band. They are insanely reliable and very capable devices. We collect these devices once they have fallen from the sky and can use them for launches like this one. 

For the Primary radiosonde we had it transmitting HORUSv2 (a protocol like APRS but tailored for this use case) at a duty cycle of 50%

### The Payload
The payload has a few different subsystems
- Upwards imaging
	- The upwards facing camera is a GoPro 11 mini running at 5.3k 30fps
	- 512GB of storage(accidently way over speced)
	- That has a custom sun shield attachment that can be lifted off when commanded by the Datalogger board
- Downwards imaging
    - [A Runcam Split V4](https://shop.runcam.com/runcam-split-4-v2/) recording at 2.7k 60fps
    - 256GB sdcard (also way over speced for how quick our recovery was)
- Datalogger
	- The Datalogger was my custom control board and datalogger for the mission. 
	- It recorded 
		- Internal and external pressure, temperature, and humidity.
		- GPS location and alt
		- Orientation data to digitally reconstruct the flight
	- And it commanded the Sunshield to open with a duty cycle of  20% and 100% during totality
- The Cutdown system
    - Used a solinoid and guids to allow for the payload to cleanly fall away without any strings to get tangled.


***

## Talaria
Talaria is our Return To Home System that we tacked on to the project incase we had extra time before the eclipse(how naive we were) Is the only active part of the project at the moment.
Currently using: 

- BN-880 [GPS](https://store.beitian.com/products/beitian-compass-qmc5883l-amp2-6-pix4-pixhawk-gnss-gps-glonass-dual-flight-control-gps-module-bn-880q?variant=44696120295711)  with built in compass (HMC5883)
- [Seeed Studio XIAO Samd21](https://wiki.seeedstudio.com/Seeeduino-XIAO/)
- [BME280](https://www.adafruit.com/product/2652) For pressure 