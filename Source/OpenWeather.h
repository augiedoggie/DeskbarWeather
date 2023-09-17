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
namespace BPrivate
{
namespace Network
{
	class BUrlRequest;
}
} // namespace BPrivate


using namespace BPrivate::Network;


class OpenWeather {
public:

						OpenWeather(const char* apikey, double latitude, double longitude, bool imperial, BInvoker* invoker);
						~OpenWeather();

	status_t			Refresh();
	void				RebuildRequestUrl(const char* apikey, double latitude, double longitude, bool imperial);
	BInvoker*			Invoker();
	Condition*			Current();
	status_t			LastUpdate(BString& output, bool longFormat = false);
	BObjectList<Condition>*	Forecast();
	status_t			ParseResult(BMessage& data, bool imperial);

private:

	status_t			_ParseCurrent(BMessage& data);
	Condition*			_ParseDay(BMessage& data, bool imperial);
	status_t			_ParseForecast(BMessage& data, bool imperial);
	void				_ResetConditions();
	void				_SetUpdateTime(bigtime_t);

	Condition*				fCurrent;
	BObjectList<Condition>*	fForecastList;
	BInvoker*				fInvoker;
	time_t					fLastUpdateTime;
	BUrl*					fOneUrl;
	BMessage*				fOpenWeatherMessage;
	BUrlRequest*			fUrlRequest;
};


#endif // _OPENWEATHER_H_
