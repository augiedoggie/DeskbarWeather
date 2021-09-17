// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "OpenWeather.h"
#include "Condition.h"
#include "JsonRequest.h"
#include "WeatherSettings.h"

#include <Alert.h>
#include <AutoDeleter.h>
#include <DateTimeFormat.h>
#include <File.h>
#include <FindDirectory.h>
#include <Invoker.h>
#include <String.h>
#include <private/netservices/HttpRequest.h>
#include <private/netservices/HttpResult.h>
#include <private/netservices/UrlProtocolRoster.h>
#include <private/netservices/UrlRequest.h>
#include <private/shared/Json.h>


using namespace BPrivate;
using namespace BPrivate::Network;


OpenWeather::OpenWeather(WeatherSettings* settings, BInvoker* invoker)
	:
	fGeoMessage(new BMessage()),
	fOpenWeatherMessage(new BMessage()),
	fCurrent(NULL),
	fForecastList(NULL),
	fInvoker(invoker),
	fWeatherSettings(settings),
	fLastUpdateTime(-1),
	fOneUrl(NULL)
{
	RebuildRequestUrl();
}


OpenWeather::~OpenWeather()
{
	delete fCurrent;
	delete fForecastList;
	delete fGeoMessage;
	delete fInvoker;
	delete fOneUrl;
	delete fOpenWeatherMessage;
}


void
OpenWeather::RebuildRequestUrl()
{
	const char* weatherUrl = "https://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&exclude=minutely,hourly&units=%s&appid=%s";

	if (fWeatherSettings->ApiKey() == NULL)
		return;

	//TODO check if latitude/longitude is set

	bool needRefresh = false;
	BString urlStr;
	urlStr.SetToFormat(weatherUrl, fWeatherSettings->Latitude(), fWeatherSettings->Longitude(),
					   fWeatherSettings->ImperialUnits() ? "imperial" : "metric", fWeatherSettings->ApiKey());

	if (fOneUrl != NULL) {
		if (fOneUrl->UrlString() == urlStr)
			return; // no changes

		delete fOneUrl;
		needRefresh = true;
	}

	fOneUrl = new BUrl(urlStr);
	if (needRefresh)
		Refresh();
}


status_t
OpenWeather::Refresh()
{
	//TODO fix leak
	BUrlRequest* request = BUrlProtocolRoster::MakeRequest(*fOneUrl, new BMallocIO(), new JsonRequestListener(fInvoker));
	return request->Run() < B_OK ? B_ERROR : B_OK;
}


status_t
OpenWeather::LastUpdate(BString& output, bool longFormat)
{
	if (longFormat)
		BDateTimeFormat().Format(output, fLastUpdateTime, B_FULL_DATE_FORMAT, B_SHORT_TIME_FORMAT);
	else
		BDateTimeFormat().Format(output, fLastUpdateTime, B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT);

	return B_OK;
}


BInvoker*
OpenWeather::Invoker()
{
	return fInvoker;
}


Condition*
OpenWeather::Current()
{
	return fCurrent;
}


BObjectList<Condition>*
OpenWeather::Forecast()
{
	return fForecastList;
}


status_t
OpenWeather::ParseResult(BMessage& data)
{
	fOpenWeatherMessage->MakeEmpty();
	*fOpenWeatherMessage = data;

#if defined(DEBUG)
	BPath prefsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath) == B_OK) {
		prefsPath.Append("DeskbarWeatherSettings.OW.msg");
		BFile prefsFile;
		if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) == B_OK)
			fOpenWeatherMessage->Flatten(&prefsFile);
	}
#endif

	BMessage currentMsg;
	if (fOpenWeatherMessage->FindMessage("current", &currentMsg) == B_OK)
		fCurrent = _ParseDay(currentMsg);

	BMessage dailyMsg;
	if (fOpenWeatherMessage->FindMessage("daily", &dailyMsg) == B_OK)
		_ParseForecast(dailyMsg);

	fLastUpdateTime = time(NULL);

	return B_OK;
}


status_t
OpenWeather::_ParseCurrent(BMessage& data)
{
	fCurrent = new Condition();
	fCurrent->SetTemp(data.GetDouble("temp", -99.0));
	fCurrent->SetTemp(data.GetDouble("feels_like", -99.0), true);
	fCurrent->SetHumidity(data.GetDouble("humidity", -99.0) / 100);
	BString bufStr;
	double windspeed = data.GetDouble("wind_speed", -99.0);
	bufStr.SetToFormat("%.1f MPH", windspeed);
	fCurrent->SetWind(bufStr);

	BMessage weatherMsg;
	if (data.FindMessage("weather", &weatherMsg) != B_OK)
		return B_ERROR;

	BMessage zeroMsg;
	if (weatherMsg.FindMessage("0", &zeroMsg) != B_OK)
		return B_ERROR;

	if (zeroMsg.FindString("description", &bufStr) == B_OK)
		fCurrent->SetForecast(bufStr);

	bufStr = "";
	if (zeroMsg.FindString("icon", &bufStr) != B_OK)
		return B_ERROR;

	// strip off 'd' or 'n'
	bufStr.Truncate(bufStr.Length() - 1);
	int32 icon = atol(bufStr.String());
	switch (icon) {
		case 1:
			fCurrent->SetIcon("sunny");
			break;
		case 2:
			fCurrent->SetIcon("partlycloudy");
			break;
		case 3:
			fCurrent->SetIcon("mostlycloudy");
			break;
		case 4:
			fCurrent->SetIcon("cloudy");
			break;
		case 9:
			fCurrent->SetIcon("rain");
			break;
		case 10:
			fCurrent->SetIcon("rain");
			break;
		case 11:
			fCurrent->SetIcon("thunderstorm");
			break;
		case 13:
			fCurrent->SetIcon("snow");
			break;
		case 50:
			fCurrent->SetIcon("mist");
			break;
		default:
			fCurrent->SetIcon("unkown");
	}

	return B_OK;
}


void
OpenWeather::_ResetConditions()
{
	delete fCurrent;
	delete fForecastList;

	fCurrent = new Condition();
	fForecastList = new BObjectList<Condition>(5, true);
}


status_t
OpenWeather::_ParseForecast(BMessage& data)
{
	fForecastList = new BObjectList<Condition>(6, true);

	BMessage bufMsg;
	if (data.FindMessage("0", &bufMsg) != B_OK)
		return B_ERROR;

	double t = data.GetDouble("dt", -99999);
	fCurrent->SetDay(t);

	BMessage tempMsg;
	if (bufMsg.FindMessage("temp", &tempMsg) == B_OK) {
		fCurrent->SetLow(tempMsg.GetDouble("min", -99));
		fCurrent->SetHigh(tempMsg.GetDouble("max", -99));
	}

//	Condition* condition = _ParseDay(bufMsg);
//	if (condition != NULL)
//		fForecastList->AddItem(condition);

	BString dayStr("0");
	for (int32 idx = 0; data.HasMessage(dayStr); idx++) {
		dayStr.Truncate(0);
		dayStr << idx;

		BMessage dayMsg;
		if (data.FindMessage(dayStr, &dayMsg) == B_OK)
			fForecastList->AddItem(_ParseDay(dayMsg));
	}

	return B_OK;
}


Condition*
OpenWeather::_ParseDay(BMessage& data)
{
	Condition* condition = new Condition();
	condition->SetDay(data.GetDouble("dt", -99999));
	condition->SetTemp(data.GetDouble("temp", -999.0));
	condition->SetTemp(data.GetDouble("feels_like", -999.0), true);
	condition->SetHumidity(data.GetDouble("humidity", -99.0) / 100);
	BString bufStr;
	double windspeed = data.GetDouble("wind_speed", -99.0);
	if (fWeatherSettings->ImperialUnits())
		bufStr.SetToFormat("%.1f MPH", windspeed);
	else
		bufStr.SetToFormat("%.1f KPH", windspeed);

	condition->SetWind(bufStr);


	BMessage tempMsg;
	if (data.FindMessage("temp", &tempMsg) == B_OK) {
		condition->SetLow(tempMsg.GetDouble("min", -99));
		condition->SetHigh(tempMsg.GetDouble("max", -99));
	}

	BMessage weatherMsg;
	if (data.FindMessage("weather", &weatherMsg) != B_OK)
		return condition;

	BMessage zeroMsg;
	if (weatherMsg.FindMessage("0", &zeroMsg) != B_OK)
		return condition;

	if (zeroMsg.FindString("description", &bufStr) == B_OK)
		condition->SetForecast(bufStr);

	bufStr = "";
	if (zeroMsg.FindString("icon", &bufStr) != B_OK)
		return condition;

	// strip off 'd' or 'n'
	bufStr.Truncate(bufStr.Length() - 1);
	int32 icon = atol(bufStr.String());
	switch (icon) {
		case 1:
			condition->SetIcon("sunny");
			break;
		case 2:
			condition->SetIcon("partlycloudy");
			break;
		case 3:
			condition->SetIcon("mostlycloudy");
			break;
		case 4:
			condition->SetIcon("cloudy");
			break;
		case 9:
			condition->SetIcon("rain");
			break;
		case 10:
			condition->SetIcon("rain");
			break;
		case 11:
			condition->SetIcon("thunderstorm");
			break;
		case 13:
			condition->SetIcon("snow");
			break;
		case 50:
			condition->SetIcon("mist");
			break;
		default:
			condition->SetIcon("unkown");
	}

	return condition;
}
