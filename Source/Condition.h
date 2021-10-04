// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _CONDITION_H_
#define _CONDITION_H_


#include <SupportDefs.h>


class BString;


class Condition {

public:
						Condition();
						~Condition();

			void		SetForecast(const char* forecast);
			BString*	Forecast();

			void		SetTemp(double temp, bool feelsLike = false);
			double		Temp(bool feelsLike = false);
			int32		iTemp(bool feelsLike = false);

			void		SetLow(double temp);
			double		Low();
			int32		iLow();

			void		SetHigh(double temp);
			double		High();
			int32		iHigh();

			void		SetHumidity(const char* humidity);
			void		SetHumidity(double humidity);
			BString*	Humidity();

			void		SetWind(const char* wind);
			BString*	Wind();

			void		SetIcon(const char* icon);
			BString*	Icon();

			void		SetDay(time_t t);
			time_t		Day();

private:
			BString*	fForecast;
			BString*	fHumidity;
			BString*	fWind;
			BString*	fIcon;
			time_t		fDay;
			double		fTemp;
			double		fFeelsLike;
			double		fLowTemp;
			double		fHighTemp;
};

#endif	// _CONDITION_H_
