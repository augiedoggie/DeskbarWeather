// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _OPENMETEO_H_
#define _OPENMETEO_H_

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


class OpenMeteo {
public:

						OpenMeteo(double latitude, double longitude, bool imperial, BInvoker* invoker);
						~OpenMeteo();

	status_t			Refresh();
	void				RebuildRequestUrl(double latitude, double longitude, bool imperial);
	BInvoker*			Invoker();
	Condition*			Current();
	status_t			LastUpdate(BString& output, bool longFormat = false);
	BObjectList<Condition>*	Forecast();
	status_t			ParseResult(BMessage& data);
	bool				IsImperial();

private:

	status_t			_ParseCurrent(BMessage& data);
	status_t			_ParseForecast(BMessage& data);
	status_t			_ParseWeatherCode(Condition& condition, int32 weathercode);
	void				_ResetConditions();
	void				_SetUpdateTime(bigtime_t);

	Condition*				fCurrent;
	BObjectList<Condition>*	fForecastList;
	BInvoker*				fInvoker;
	time_t					fLastUpdateTime;
	BUrl*					fApiUrl;
	BMessage*				fOpenMeteoMessage;
	BUrlRequest*			fUrlRequest;
	bool					fImperial;
};


#endif // _OPENMETEO_H_
