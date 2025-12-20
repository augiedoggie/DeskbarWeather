// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "OpenMeteo.h"
#include "Condition.h"
#include "JsonRequest.h"

#include <DateTimeFormat.h>
#include <File.h>
#include <FindDirectory.h>
#include <Invoker.h>
#include <private/netservices/UrlProtocolRoster.h>
#include <private/netservices/UrlRequest.h>


const char* kOpenMeteoUrl = 
	"https://api.open-meteo.com/v1/forecast"
	"?latitude=%f&longitude=%f"
	"&timezone=auto"
	"&temperature_unit=%s"
	"&wind_speed_unit=%s"
	"&precipitation_unit=%s"
	"&timeformat=unixtime"
	"&forecast_days=%i"
	"&current=temperature_2m,apparent_temperature,relative_humidity_2m,wind_speed_10m,wind_direction_10m,cloud_cover,weathercode"
	"&daily=temperature_2m_min,temperature_2m_max,weathercode";


OpenMeteo::OpenMeteo(double latitude, double longitude, bool imperial, int32 forecastDays, BInvoker* invoker)
	:
	fCurrent(NULL),
	fForecastList(NULL),
	fInvoker(invoker),
	fLastUpdateTime(-1),
	fApiUrl(NULL),
	fOpenMeteoMessage(new BMessage()),
	fUrlRequest(NULL)
{
	RebuildRequestUrl(latitude, longitude, imperial, forecastDays);
}


OpenMeteo::~OpenMeteo()
{
	if (fUrlRequest != NULL) {
		if (fUrlRequest->IsRunning())
			fUrlRequest->Stop();

		delete fUrlRequest->Output();
		delete dynamic_cast<JsonRequestListener*>(fUrlRequest->Listener());
	}
	delete fUrlRequest;
	delete fCurrent;
	delete fForecastList;
	delete fInvoker;
	delete fApiUrl;
	delete fOpenMeteoMessage;
}


void
OpenMeteo::RebuildRequestUrl(double latitude, double longitude, bool imperial, int32 forecastDays)
{
	fImperial = imperial;
	fForecastDays = forecastDays;

	//TODO check if latitude/longitude is set

	bool needRefresh = false;
	BString urlStr;
	// always request at least one forecast day so we can get the high/low temperature for the current day
	urlStr.SetToFormat(kOpenMeteoUrl, latitude, longitude,
					   imperial ? "fahrenheit" : "celsius", imperial ? "mph" : "kmh", imperial ? "inch" : "mm", fForecastDays > 0 ? fForecastDays : 1);

	if (fApiUrl != NULL) {
		if (fApiUrl->UrlString() == urlStr)
			return; // no changes

		delete fApiUrl;
		needRefresh = true;
	}

	fApiUrl =
#if B_HAIKU_VERSION > B_HAIKU_VERSION_1_BETA_5
		new BUrl(urlStr, true);
#else
		new BUrl(urlStr);
#endif

	if (fUrlRequest == NULL)
		fUrlRequest = BUrlProtocolRoster::MakeRequest(*fApiUrl, new BMallocIO(), new JsonRequestListener(fInvoker));
	else
		fUrlRequest->SetUrl(*fApiUrl);

	if (needRefresh)
		Refresh();
}


status_t
OpenMeteo::Refresh()
{
	if (fUrlRequest->IsRunning())
		return B_ERROR; //TODO stop and restart?

	return fUrlRequest->Run() < B_OK ? B_ERROR : B_OK;
}


status_t
OpenMeteo::LastUpdate(BString& output, bool longFormat)
{
	if (longFormat)
		BDateTimeFormat().Format(output, fLastUpdateTime, B_FULL_DATE_FORMAT, B_SHORT_TIME_FORMAT);
	else
		BDateTimeFormat().Format(output, fLastUpdateTime, B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT);

	return B_OK;
}


BInvoker*
OpenMeteo::Invoker()
{
	return fInvoker;
}


Condition*
OpenMeteo::Current()
{
	return fCurrent;
}


BObjectList<Condition>*
OpenMeteo::Forecast()
{
	return fForecastList;
}


bool
OpenMeteo::IsImperial()
{
	return fImperial;
}


status_t
OpenMeteo::ParseResult(BMessage& data)
{
	fOpenMeteoMessage->MakeEmpty();
	*fOpenMeteoMessage = data;

#if defined(DEBUG)
	BPath prefsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath) == B_OK) {
		prefsPath.Append("DeskbarWeatherSettings.OW.msg");
		BFile prefsFile;
		if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) == B_OK)
			fOpenMeteoMessage->Flatten(&prefsFile);
	}
#endif

	BMessage message, unitsMessage;

	if (fOpenMeteoMessage->FindMessage("current_units", &unitsMessage) != B_OK)
		return B_ERROR;

	if (fOpenMeteoMessage->FindMessage("current", &message) != B_OK)
		return B_ERROR;

	if (_ParseCurrent(message) != B_OK)
		return B_ERROR;

	message.MakeEmpty();
	if (fOpenMeteoMessage->FindMessage("daily", &message) != B_OK)
		return B_ERROR;

	if (_ParseForecast(message) != B_OK)
		return B_ERROR;

	fLastUpdateTime = fCurrent->Day();

	return B_OK;
}


void
OpenMeteo::_ResetConditions()
{
	delete fCurrent;
	delete fForecastList;

	fCurrent = new Condition();
	fForecastList =
#if B_HAIKU_VERSION > B_HAIKU_VERSION_1_BETA_5
		new BObjectList<Condition>(6);
#else
		new BObjectList<Condition>(6, true);
#endif
}


status_t
OpenMeteo::_ParseCurrent(BMessage& data)
{
	fCurrent = new Condition();
	fCurrent->SetTemp(data.GetDouble("temperature_2m", -99.0));
	fCurrent->SetTemp(data.GetDouble("apparent_temperature", -99.0), true);
	fCurrent->SetHumidity(data.GetDouble("relative_humidity_2m", -99.0) / 100);
	fCurrent->SetWind(data.GetDouble("wind_speed_10m", -99.0));
	fCurrent->SetWindDirection(data.GetDouble("wind_direction_10m", -99.0));
	fCurrent->SetCloudCover(data.GetDouble("cloud_cover", -99.0));
	fCurrent->SetDay(data.GetDouble("time", -9999));

	int32 weathercode = static_cast<int32>(data.GetDouble("weathercode", -99.0));
	return _ParseWeatherCode(*fCurrent, weathercode);
}


status_t
OpenMeteo::_ParseForecast(BMessage& data)
{
	fForecastList =
#if B_HAIKU_VERSION > B_HAIKU_VERSION_1_BETA_5
		new BObjectList<Condition>(6);
#else
		new BObjectList<Condition>(6, true);
#endif

	BMessage timeMessage, tempMinMessage, tempMaxMessage, codeMessage;
	if (data.FindMessage("time", &timeMessage) != B_OK
		|| data.FindMessage("temperature_2m_min", &tempMinMessage)
		|| data.FindMessage("temperature_2m_max", &tempMaxMessage)
		|| data.FindMessage("weathercode", &codeMessage))
		return B_ERROR;

	fCurrent->SetLow(tempMinMessage.GetDouble("0", -99));
	fCurrent->SetHigh(tempMaxMessage.GetDouble("0", -99));

	// return now if we only needed the daily high/low
	if (fForecastDays == 0)
		return B_OK;

	BString dayStr("0");
	for (int32 idx = 0; timeMessage.HasDouble(dayStr); idx++) {
		Condition* condition = new Condition();
		condition->SetDay(timeMessage.GetDouble(dayStr, -99999));
		condition->SetLow(tempMinMessage.GetDouble(dayStr, -999.0));
		condition->SetHigh(tempMaxMessage.GetDouble(dayStr, -999.0));
		int32 weathercode = static_cast<int32>(codeMessage.GetDouble(dayStr, -99.0));
		_ParseWeatherCode(*condition, weathercode);
		fForecastList->AddItem(condition);

		dayStr.Truncate(0);
		dayStr << idx + 1;
	}

	return B_OK;
}


status_t
OpenMeteo::_ParseWeatherCode(Condition& condition, int32 weathercode)
{
	switch (weathercode) {
		case 0:
			condition.SetForecast("Clear sky");
			condition.SetIcon("sunny");
			break;
		case 1:
			condition.SetForecast("Mainly clear");
			condition.SetIcon("partlycloudy");
			break;
		case 2:
			condition.SetForecast("Partly cloudy");
			condition.SetIcon("partlycloudy");
			break;
		case 3:
			condition.SetForecast("Overcast");
			condition.SetIcon("cloudy");
			break;
		case 45:
			condition.SetForecast("Fog");
			condition.SetIcon("cloudy");
			break;
		case 48:
			condition.SetForecast("Rime fog");
			condition.SetIcon("cloudy");
			break;
		case 51:
			condition.SetForecast("Light drizzle");
			condition.SetIcon("rain");
			break;
		case 53:
			condition.SetForecast("Drizzle");
			condition.SetIcon("rain");
			break;
		case 55:
			condition.SetForecast("Dense drizzle");
			condition.SetIcon("rain");
			break;
		case 56:
			condition.SetForecast("Light Freezing drizzle");
			condition.SetIcon("rain");
			break;
		case 57:
			condition.SetForecast("Dense Freezing drizzle");
			condition.SetIcon("rain");
			break;
		case 61:
			condition.SetForecast("Slight rain");
			condition.SetIcon("rain");
			break;
		case 63:
			condition.SetForecast("Rain");
			condition.SetIcon("rain");
			break;
		case 65:
			condition.SetForecast("Heavy rain");
			condition.SetIcon("rain");
			break;
		case 66:
			condition.SetForecast("Light freezing rain");
			condition.SetIcon("rain");
			break;
		case 67:
			condition.SetForecast("Heavy freezing rain");
			condition.SetIcon("rain");
			break;
		case 71:
			condition.SetForecast("Slight snow");
			condition.SetIcon("snow");
			break;
		case 73:
			condition.SetForecast("Snow");
			condition.SetIcon("snow");
			break;
		case 75:
			condition.SetForecast("Heavy snow");
			condition.SetIcon("snow");
			break;
		case 77:
			condition.SetForecast("Snow grains");
			condition.SetIcon("snow");
			break;
		case 80:
			condition.SetForecast("Slight rain showers");
			condition.SetIcon("rain");
			break;
		case 81:
			condition.SetForecast("Rain showers");
			condition.SetIcon("rain");
			break;
		case 82:
			condition.SetForecast("Violent rain showers");
			condition.SetIcon("rain");
			break;
		case 85:
			condition.SetForecast("Slight snow showers");
			condition.SetIcon("snow");
			break;
		case 86:
			condition.SetForecast("Heavy snow showers");
			condition.SetIcon("snow");
			break;
		case 95:
			condition.SetForecast("Thunderstorm");
			condition.SetIcon("thunderstorm");
			break;
		case 96:
			condition.SetForecast("Thunderstorm with slight hail");
			condition.SetIcon("thunderstorm");
			break;
		case 99:
			condition.SetForecast("Thunderstorm with heavy hail");
			condition.SetIcon("thunderstorm");
			break;
	}

	return B_OK;
}
