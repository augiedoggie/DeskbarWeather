// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "IpApiLocationProvider.h"
#include "JsonRequest.h"
#include "WeatherSettings.h"

#include <AutoDeleter.h>
#include <File.h>
#include <FindDirectory.h>
#include <Invoker.h>
#include <Message.h>
#include <private/netservices/HttpRequest.h>
#include <private/netservices/HttpResult.h>
#include <private/netservices/UrlProtocolRoster.h>
#include <private/netservices/UrlRequest.h>
#include <private/shared/Json.h>


using namespace BPrivate;
using namespace BPrivate::Network;


const char* kGeoLookupCacheKey = "dw:GeoLookupCache";


IpApiLocationProvider::IpApiLocationProvider(WeatherSettings* settings, BInvoker* invoker)
	:
	fInvoker(invoker),
	fWeatherSettings(settings)
{}


IpApiLocationProvider::~IpApiLocationProvider()
{
	delete fInvoker;
}


status_t
IpApiLocationProvider::Run()
{
	BMessage geoMsg;
	if (_LoadCache(geoMsg) == B_OK) {
		if (ParseResult(geoMsg, false) != B_OK)
			return B_ERROR;

		//TODO add bool to mark the result as cached?
		return fInvoker->Invoke(&geoMsg);
	}

	//TODO fix leak
	BUrl url("http://ip-api.com/json/?fields=status,message,lat,lon,country,regionName,city");
	BUrlRequest* request = BUrlProtocolRoster::MakeRequest(url, new BMallocIO(), new JsonRequestListener(fInvoker));
	return request->Run() < B_OK ? B_ERROR : B_OK;
}


status_t
IpApiLocationProvider::ParseResult(BMessage& data, bool cacheResult)
{
	BString bufStr;
	if (data.FindString("city", &bufStr) == B_OK) {
		BString locationStr(bufStr);
		if (data.FindString("regionName", &bufStr) == B_OK)
			locationStr << ", " << bufStr;
		fWeatherSettings->SetLocation(locationStr);
	}

	double latitude = data.GetDouble("lat", -999.0);
	double longitude = data.GetDouble("lon", -999.0);

	if (latitude != -999.0 && longitude != -999.0)
		fWeatherSettings->SetLocation(latitude, longitude);

	type_code type;
	if (fWeatherSettings->GetInfo(kGeoLookupCacheKey, &type) == B_OK)
		fWeatherSettings->ReplaceMessage(kGeoLookupCacheKey, &data);
	else
		fWeatherSettings->AddMessage(kGeoLookupCacheKey, &data);

	if (cacheResult)
		_SaveCache(data);

	return B_OK;
}


status_t
IpApiLocationProvider::_SaveCache(BMessage& message)
{
	BPath prefsPath;
	if (find_directory(B_USER_CACHE_DIRECTORY, &prefsPath) != B_OK)
		return B_ERROR;

	//TODO mkdir DeskbarWeather
	prefsPath.Append("DeskbarWeather.GL.msg");
	BFile prefsFile;
	if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) == B_OK)
		return message.Flatten(&prefsFile);

	return B_ERROR;
}


status_t
IpApiLocationProvider::_LoadCache(BMessage& message)
{
	BPath prefsPath;
	if (find_directory(B_USER_CACHE_DIRECTORY, &prefsPath) != B_OK)
		return B_ERROR;

	//TODO mkdir DeskbarWeather
	prefsPath.Append("DeskbarWeather.GL.msg");
	BFile prefsFile;
	if (prefsFile.SetTo(prefsPath.Path(), B_READ_ONLY) == B_OK)
		return message.Unflatten(&prefsFile);

	return B_ERROR;
}
