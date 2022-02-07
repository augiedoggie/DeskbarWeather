## ![DeskbarWeather Logo](Assets/Icons/PartlyCloudy.svg) DeskbarWeather

![DeskbarWeather ScreenShot](Assets/Screenshots/DeskbarAndNotification.png)

_DeskbarWeather_ is a Haiku Deskbar replicant which displays weather information.

Information is currently retrieved from [openweathermap](https://www.openweathermap.org) and requires a free API key from them.

------------------------------------------------------------


### Build Instructions

```
~/DeskbarWeather> cmake .
~/DeskbarWeather> make
```

You will need to have `sphinx` installed if you want to build the user guide.  It can be insalled with the python `pip` command.  Depending on which python package you have installed the name of the `pip` command may be slightly different.  For example, to use python 3.8...

```
~> pkgman install pip_python38
~> pip3.8 install sphinx
```
