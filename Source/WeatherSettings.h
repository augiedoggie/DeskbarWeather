// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _WEATHERSETTINGS_H_
#define _WEATHERSETTINGS_H_


#include <Locker.h>
#include <Message.h>


class BFont;


class WeatherSettings : public BMessage, public BLocker {
public:
				WeatherSettings();
				WeatherSettings(const WeatherSettings& settings);
	virtual		~WeatherSettings();

	status_t	Load();
	status_t	Save();

	const char*	Location();
	void		SetLocation(const char* location);
	void		SetLocation(double latitude, double longitude);
	double		Latitude();
	double		Longitude();
	bool		ImperialUnits();
	void		SetImperialUnits(bool useImperial);
	int32		RefreshInterval();
	void		SetRefreshInterval(int32 minutes);
	void		SetUseGeoLocation(bool enabled);
	bool		UseGeoLocation();
	void		SetUseGeoNotification(bool enabled);
	bool		UseGeoNotification();
	void		SetUseNotification(bool enabled);
	bool		UseNotification();
	void		SetNotificationClick(bool enabled);
	bool		NotificationClick();
	status_t	SetFont(const char* family, const char* style, double size);
	status_t	SetFont(BFont& font);
	status_t	GetFont(BFont& font);
	void		ResetFont();
	void		SetCompactForecast(bool enabled);
	bool		CompactForecast();
	void		SetShowFeelsLike(bool enabled);
	bool		ShowFeelsLike();
	void		SetForecastDays(int32 days);
	int32		ForecastDays();
};

#endif // _WEATHERSETTINGS_H_
