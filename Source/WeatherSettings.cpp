// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "WeatherSettings.h"

#include <Path.h>
#include <File.h>
#include <FindDirectory.h>
#include <Font.h>


const char* kPrefsFileName = "DeskbarWeatherSettings";

const char* kLocationKey = "dw:Location";
const char* kIntervalKey = "dw:RefreshInterval";
const char* kUseImperialKey = "dw:UseImperial";
const char* kApiKey = "dw:ApiKey";
const char* kUseGeoLocationKey = "dw:UseGeoLocation";
const char* kUseGeoNotificationKey = "dw:UseGeoNotification";
const char* kUseNotificationKey = "dw:UseNotification";
const char* kLatitudeKey = "dw:Latitude";
const char* kLongitudeKey = "dw:Longitude";
const char* kFontFamilyKey = "dw:FontFamily";
const char* kFontStyleKey = "dw:FontStyle";
const char* kFontSizeKey = "dw:FontSize";
const char* kCompactForecastKey = "dw:CompactForecast";

const char* kDefaultLocation = "Rapa Nui";
const double kDefaultLatitude = -27.116667;
const double kDefaultLongitude = -109.366667;

const int32 kDefaultInterval = 10;
const bool kImperialDefaultUnit = true;
const bool kUseGeoLocationDefault = true;
const bool kUseNotificationDefault = true;
const bool kUseGeoNotificationDefault = true;
const bool kCompactForecastDefault = false;


WeatherSettings::WeatherSettings()
{
	BPath prefsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath) != B_OK)
		return;

	prefsPath.Append(kPrefsFileName);
	BFile prefsFile;

	if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE | B_CREATE_FILE) != B_OK)
		return;

	if (Unflatten(&prefsFile) != B_OK)
		return;
}


WeatherSettings::~WeatherSettings()
{
	BPath prefsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath) != B_OK)
		return;

	prefsPath.Append(kPrefsFileName);
	BFile prefsFile;

	if (prefsFile.SetTo(prefsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) != B_OK)
		return;

	Flatten(&prefsFile);
}


bool
WeatherSettings::CompactForecast()
{
	return GetBool(kCompactForecastKey, kCompactForecastDefault);
}


void
WeatherSettings::SetCompactForecast(bool enabled)
{
	SetBool(kCompactForecastKey, enabled);
}


const char*
WeatherSettings::Location()
{
	const char* location;
	if (FindString(kLocationKey, (const char**)&location) != B_OK)
		return kDefaultLocation;

	return location;
}


void
WeatherSettings::SetLocation(const char* location)
{
	SetString(kLocationKey, location);
}


void
WeatherSettings::SetLocation(double latitude, double longitude)
{
	SetDouble(kLatitudeKey, latitude);
	SetDouble(kLongitudeKey, longitude);
}


double
WeatherSettings::Latitude()
{
	return GetDouble(kLatitudeKey, kDefaultLatitude);
}


double
WeatherSettings::Longitude()
{
	return GetDouble(kLongitudeKey, kDefaultLongitude);
}


const char*
WeatherSettings::ApiKey()
{
	const char* key = NULL;
	FindString(kApiKey, (const char**)&key);

	return key;
}


void
WeatherSettings::SetApiKey(const char* key)
{
	SetString(kApiKey, key);
}


bool
WeatherSettings::ImperialUnits()
{
	return GetBool(kUseImperialKey, kImperialDefaultUnit);
}


void
WeatherSettings::SetImperialUnits(bool useImperial)
{
	SetBool(kUseImperialKey, useImperial);
}


bool
WeatherSettings::UseGeoLocation()
{
	return GetBool(kUseGeoLocationKey, kUseGeoLocationDefault);
}


void
WeatherSettings::SetUseGeoLocation(bool useGeo)
{
	SetBool(kUseGeoLocationKey, useGeo);
}


bool
WeatherSettings::UseGeoNotification()
{
	return GetBool(kUseGeoNotificationKey, kUseGeoNotificationDefault);
}


void
WeatherSettings::SetUseGeoNotification(bool useNotificaton)
{
	SetBool(kUseGeoNotificationKey, useNotificaton);
}


bool
WeatherSettings::UseNotification()
{
	return GetBool(kUseNotificationKey, kUseNotificationDefault);
}


void
WeatherSettings::SetUseNotification(bool useNotificaton)
{
	SetBool(kUseNotificationKey, useNotificaton);
}


int32
WeatherSettings::RefreshInterval()
{
	return GetInt32(kIntervalKey, kDefaultInterval);
}


void
WeatherSettings::SetRefreshInterval(int32 minutes)
{
	if (minutes <= 0)
		return;

	SetInt32(kIntervalKey, minutes);
}


status_t
WeatherSettings::GetFont(BFont& font)
{
	BFont defaultFont(be_plain_font);
	font_family family;
	font_style style;
	defaultFont.GetFamilyAndStyle(&family, &style);
	font.SetFamilyAndStyle(GetString(kFontFamilyKey, family), GetString(kFontStyleKey, style));
	font.SetSize(GetDouble(kFontSizeKey, defaultFont.Size())); // TODO ensure size is within limits
	return B_OK;
}


status_t
WeatherSettings::SetFont(const char* family, const char* style, double size)
{
	if (family == NULL || style == NULL || size < 8.0 || size > 16.0)
		return B_ERROR;

	SetString(kFontFamilyKey, family);
	SetString(kFontStyleKey, style);
	SetDouble(kFontSizeKey, size);

	return B_OK;
}
