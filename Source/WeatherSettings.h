// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _WEATHERSETTINGS_H_
#define _WEATHERSETTINGS_H_


#include <Message.h>


class BFont;


class WeatherSettings : public BMessage {
public:
				WeatherSettings();
	virtual		~WeatherSettings();

	const char*	Location();
	void		SetLocation(const char* location);
	void		SetLocation(double latitude, double longitude);
	double		Latitude();
	double		Longitude();
	bool		ImperialUnits();
	void		SetImperialUnits(bool useImperial);
	int32		RefreshInterval();
	void		SetRefreshInterval(int32 minutes);
	void		SetApiKey(const char* key);
	const char*	ApiKey();
	void		SetUseGeoLocation(bool enabled);
	bool		UseGeoLocation();
	void		SetUseGeoNotification(bool enabled);
	bool		UseGeoNotification();
	void		SetUseNotification(bool enabled);
	bool		UseNotification();
	status_t	SetFont(const char* family, const char* style, double size);
	status_t	GetFont(BFont& font);
	void		SetCompactForecast(bool enabled);
	bool		CompactForecast();
};

#endif	// _WEATHERSETTINGS_H_
