## ![DeskbarWeather Logo](Assets/Icons/PartlyCloudy.svg) DeskbarWeather

![DeskbarWeather ScreenShot](Assets/Screenshots/DeskbarAndNotification.png)

_DeskbarWeather_ is a Haiku Deskbar replicant which displays weather information.

Latitude/Longitude geolookup uses [ip-api](http://ip-api.com/)

Weather is retrieved from [Open-Meteo](https://open-meteo.com)

------------------------------------------------------------


### Build Instructions

```
~/DeskbarWeather> cmake .
~/DeskbarWeather> make
```

You will need to have `sphinx` installed if you want to build the user guide.

```
~> pkgman install sphinx_python310
```
