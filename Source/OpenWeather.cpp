// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "OpenWeather.h"
#include "Condition.h"
#include "JsonRequest.h"

#include <DateTimeFormat.h>
#include <File.h>
#include <FindDirectory.h>
#include <Invoker.h>
#include <private/netservices/UrlProtocolRoster.h>
#include <private/netservices/UrlRequest.h>


const char* kOpenWeatherUrl = "https://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&exclude=minutely,hourly&units=%s&appid=%s";


OpenWeather::OpenWeather(const char* apikey, double latitude, double longitude, bool imperial, BInvoker* invoker)
	:
	fCurrent(NULL),
	fForecastList(NULL),
	fInvoker(invoker),
	fLastUpdateTime(-1),
	fOneUrl(NULL),
	fOpenWeatherMessage(new BMessage()),
	fUrlRequest(NULL)
{
	RebuildRequestUrl(apikey, latitude, longitude, imperial);
}


OpenWeather::~OpenWeather()
{
	if (fUrlRequest != NULL) {
		if (fUrlRequest->IsRunning())
			fUrlRequest->Stop();

		delete fUrlRequest->Output();
		delete fUrlRequest->Listener();
	}
	delete fUrlRequest;
	delete fCurrent;
	delete fForecastList;
	delete fInvoker;
	delete fOneUrl;
	delete fOpenWeatherMessage;
}


void
OpenWeather::RebuildRequestUrl(const char* apikey, double latitude, double longitude, bool imperial)
{
	if (apikey == NULL)
		return;

	//TODO check if latitude/longitude is set

	bool needRefresh = false;
	BString urlStr;
	urlStr.SetToFormat(kOpenWeatherUrl, latitude, longitude,
					   imperial ? "imperial" : "metric", apikey);

	if (fOneUrl != NULL) {
		if (fOneUrl->UrlString() == urlStr)
			return; // no changes

		delete fOneUrl;
		needRefresh = true;
	}

	fOneUrl = new BUrl(urlStr);

	if (fUrlRequest == NULL)
		fUrlRequest = BUrlProtocolRoster::MakeRequest(*fOneUrl, new BMallocIO(), new JsonRequestListener(fInvoker));
	else
		fUrlRequest->SetUrl(*fOneUrl);

	if (needRefresh)
		Refresh();
}


status_t
OpenWeather::Refresh()
{
	if (fUrlRequest->IsRunning())
		return B_ERROR; //TODO stop and restart?

	return fUrlRequest->Run() < B_OK ? B_ERROR : B_OK;
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
OpenWeather::ParseResult(BMessage& data, bool imperial)
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
		fCurrent = _ParseDay(currentMsg, imperial);

	fLastUpdateTime = fCurrent->Day();

	BMessage dailyMsg;
	if (fOpenWeatherMessage->FindMessage("daily", &dailyMsg) == B_OK)
		_ParseForecast(dailyMsg, imperial);

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
			fCurrent->SetIcon("unknown");
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
OpenWeather::_ParseForecast(BMessage& data, bool imperial)
{
	fForecastList = new BObjectList<Condition>(6, true);

	BMessage bufMsg;
	if (data.FindMessage("0", &bufMsg) != B_OK)
		return B_ERROR;

	BMessage tempMsg;
	if (bufMsg.FindMessage("temp", &tempMsg) == B_OK) {
		fCurrent->SetLow(tempMsg.GetDouble("min", -99));
		fCurrent->SetHigh(tempMsg.GetDouble("max", -99));
	}

	BString dayStr("0");
	for (int32 idx = 0; data.HasMessage(dayStr); idx++) {
		dayStr.Truncate(0);
		dayStr << idx;

		BMessage dayMsg;
		if (data.FindMessage(dayStr, &dayMsg) == B_OK)
			fForecastList->AddItem(_ParseDay(dayMsg, imperial));
	}

	return B_OK;
}


Condition*
OpenWeather::_ParseDay(BMessage& data, bool imperial)
{
	Condition* condition = new Condition();
	condition->SetDay(data.GetDouble("dt", -99999));
	condition->SetTemp(data.GetDouble("temp", -999.0));
	condition->SetTemp(data.GetDouble("feels_like", -999.0), true);
	condition->SetHumidity(data.GetDouble("humidity", -99.0) / 100);
	BString bufStr;
	double windspeed = data.GetDouble("wind_speed", -99.0);
	if (imperial)
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
			condition->SetIcon("unknown");
	}

	return condition;
}
