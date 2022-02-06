
.. title:: DeskbarWeather User Guide

.. toctree::
   :maxdepth: 2
   :hidden:

.. contents:: User Guide Contents
   :depth: 3
   :local:
   :backlinks: none


Deskbar Operation
-----------------

.. image:: ../Screenshots/Deskbar.png
   :alt: Deskbar replicant view
   :align: center
   :scale: 200



+----------------------------+-----------------------+-------------------------+
|  Left click to show or     | Middle click to start | Right click to show the |
|  hide the forecast window  | a weather refresh     | popup menu              |
+----------------------------+-----------------------+-------------------------+
|      |ForecastScreen|      |                       |      |PopUpMenu|        |
+----------------------------+-----------------------+-------------------------+
| *Note: The escape key will |                                                 |
| also hide the window*      |                                                 |
+----------------------------+-------------------------------------------------+

.. |ForecastScreen| image:: ../Screenshots/Forecast.png
   :alt: Forecast window
   :scale: 33

.. |PopUpMenu| image:: ../Screenshots/DeskbarMenu.png
   :alt: Deskbar popup menu
   :scale: 66



Preferences
-----------

.. image:: ../Screenshots/Preferences.png
   :alt: Preferences window
   :align: center



API Key
^^^^^^^

DeskbarWeather currently retrieves weather updates using the `OpenWeatherMap <https://www.openweathermap.org>`_ service.

You must `sign up <https://home.openweathermap.org/users/sign_up>`_ for a free account and `obtain an API key <https://home.openweathermap.org/api_keys>`_



Refresh Interval
^^^^^^^^^^^^^^^^

Set how often the weather conditions should be updated.

When "Manual refresh only" is selected then the updates will only happen by choosing the "Refresh Weather" menu item or by sending a BMessage.
Sending a BMessage using the `hey` command allows assigning a keyboard command using the Shortcuts app which will start a refresh.

.. code-block:: none
   :class: terminal

   hey Deskbar let View of Replicant "DeskbarWeatherView" of Shelf of View Status of Window Deskbar do 'FrGw'



Show notification after refresh
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../Screenshots/DeskbarAndNotification.png
   :alt: Weather notification

Show a system notification after each succesful weather refresh.

*Note: Error messages are always shown regardless of this setting.*



Clicking the notification opens the forecast
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../Screenshots/NotificationClick.png
   :alt: Notification mouse click



Location
^^^^^^^^

Override the displayed city/region name that is returned by OpenWeatherMap.



Use GeoLocation lookup
^^^^^^^^^^^^^^^^^^^^^^

Automatically use the `ip-api <https://ip-api.com>`_ geolocation service to look up latitude and longitude.

*Note: No system information is transmitted. Only what the Haiku HttpRequest uses to make the request.*



Show notification after lookup
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../Screenshots/DeskbarAndGeoNotification.png
   :alt: GeoLocation notification

*Note: The notification will indicate whether a cached location is being used.*



Units
^^^^^

**Imperial** diplay degrees fahrenheit and miles per hour.

**Metric** display degrees celsius and kilometers per hour.



Font
^^^^

Select a font other than the standard Deskbar font.



Use compact forecast window
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Reduce the margins and padding of the forecast window for smaller screen resolutions or people who like a cozy layout.

.. image:: ../Screenshots/CompactForecast.png
   :alt: Compact forecast window
   :scale: 33
