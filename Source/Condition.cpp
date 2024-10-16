// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "Condition.h"

#include <NumberFormat.h>
#include <String.h>
#include <math.h>


Condition::Condition()
	:
	fForecast(new BString()),
	fHumidity(new BString()),
	fIcon(new BString("unknown")),
	fDay(-999),
	fTemp(-999),
	fLowTemp(-999),
	fHighTemp(-999),
	fWind(-999),
	fWindDirection(-999),
	fCloudCover(-999)
{}


Condition::~Condition()
{
	delete fForecast;
	delete fHumidity;
	delete fIcon;
}


void
Condition::SetForecast(const char* forecast)
{
	fForecast->SetTo(forecast);
}


BString*
Condition::Forecast()
{
	return fForecast;
}


void
Condition::SetTemp(double temp, bool feelsLike)
{
	if (feelsLike)
		fFeelsLike = temp;
	else
		fTemp = temp;
}


double
Condition::Temp(bool feelsLike)
{
	return feelsLike ? fFeelsLike : fTemp;
}


int32
Condition::iTemp(bool feelsLike)
{
	return round(feelsLike ? fFeelsLike : fTemp);
}


void
Condition::SetLow(double temp)
{
	fLowTemp = temp;
}


double
Condition::Low()
{
	return fLowTemp;
}


int32
Condition::iLow()
{
	return round(fLowTemp);
}


void
Condition::SetHigh(double temp)
{
	fHighTemp = temp;
}


double
Condition::High()
{
	return fHighTemp;
}


int32
Condition::iHigh()
{
	return round(fHighTemp);
}


void
Condition::SetHumidity(const char* humidity)
{
	fHumidity->SetTo(humidity);
}


void
Condition::SetHumidity(double humidity)
{
	BNumberFormat().FormatPercent(*fHumidity, humidity);
}


BString*
Condition::Humidity()
{
	return fHumidity;
}


void
Condition::SetWind(double wind)
{
	fWind = wind;
}


double
Condition::Wind()
{
	return fWind;
}


void
Condition::SetWindDirection(double direction)
{
	fWindDirection = direction;
}


double
Condition::WindDirection()
{
	return fWindDirection;
}


void
Condition::SetCloudCover(double cloud)
{
	fCloudCover = cloud;
}


double
Condition::CloudCover()
{
	return fCloudCover;
}


void
Condition::SetIcon(const char* icon)
{
	fIcon->SetTo(icon);
}


BString*
Condition::Icon()
{
	return fIcon;
}


void
Condition::SetDay(time_t t)
{
	fDay = t;
}


time_t
Condition::Day()
{
	return fDay;
}
