// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _OPENWEATHER_H_
#define _OPENWEATHER_H_

#include <ObjectList.h>
#include <kernel/OS.h>

class Condition;

class BInvoker;
class BMessage;
class BString;
class BUrl;

class WeatherSettings;


class OpenWeather {

public:

						OpenWeather(WeatherSettings* prefs, BInvoker* invoker);
						~OpenWeather();

	status_t			Refresh();
	void				RebuildRequestUrl();
	BInvoker*			Invoker();
	Condition*			Current();
	status_t			LastUpdate(BString& output, bool longFormat = false);
	BObjectList<Condition>*	Forecast();
	status_t			ParseResult(BMessage& data);

private:

	status_t			_ParseCurrent(BMessage& data);
	Condition*			_ParseDay(BMessage& data);
	status_t			_ParseForecast(BMessage& data);
	void				_ResetConditions();
	void				_SetUpdateTime(bigtime_t);

	BMessage*				fGeoMessage;
	BMessage*				fOpenWeatherMessage;
	Condition*				fCurrent;
	BObjectList<Condition>*	fForecastList;
	BInvoker*				fInvoker;
	WeatherSettings*		fWeatherSettings;
	time_t					fLastUpdateTime;
	BUrl*					fOneUrl;
};


#endif	// _OPENWEATHER_H_
