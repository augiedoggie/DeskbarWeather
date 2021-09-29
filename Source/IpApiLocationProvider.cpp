// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "IpApiLocationProvider.h"
#include "JsonRequest.h"
#include "WeatherSettings.h"

#include <File.h>
#include <FindDirectory.h>
#include <Invoker.h>
#include <private/netservices/UrlProtocolRoster.h>
#include <private/netservices/UrlRequest.h>


const char* kGeoLookupCacheKey = "dw:GeoLookupCache";

const char* kIpApiUrl = "http://ip-api.com/json/?fields=status,message,lat,lon,country,regionName,city";


IpApiLocationProvider::IpApiLocationProvider(WeatherSettings* settings, BInvoker* invoker)
	:
	fInvoker(invoker),
	fUrlRequest(NULL),
	fWeatherSettings(settings)
{}


IpApiLocationProvider::~IpApiLocationProvider()
{
	if (fUrlRequest != NULL) {
		if (fUrlRequest->IsRunning())
			fUrlRequest->Stop();

		delete fUrlRequest->Output();
		delete fUrlRequest->Listener();
	}
	delete fUrlRequest;
	delete fInvoker;
}


status_t
IpApiLocationProvider::Run()
{
	BMessage geoMsg;
	//TODO check if cache needs to be invalidated and a new lookup performed
	if (_LoadCache(geoMsg) == B_OK) {
		if (ParseResult(geoMsg, false) != B_OK)
			return B_ERROR;

		//TODO add bool to mark the result as cached?
		return fInvoker->Invoke(&geoMsg);
	}

	BUrl url(kIpApiUrl);
	if (fUrlRequest == NULL)
		fUrlRequest = BUrlProtocolRoster::MakeRequest(url, new BMallocIO(), new JsonRequestListener(fInvoker));
	else if (fUrlRequest->IsRunning())
		return B_ERROR; //TODO stop and restart?

	return fUrlRequest->Run() < B_OK ? B_ERROR : B_OK;
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
