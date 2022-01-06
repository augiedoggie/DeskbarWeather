// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _DESKBARWEATHERVIEW_H_
#define _DESKBARWEATHERVIEW_H_


#include <Locker.h>
#include <View.h>

enum {
	kForecastWindowMessage = 'WnGw',
	kSettingsMessage = 'CfGw',
	kHelpMessage = 'HpGw',
	kGithubMessage = 'GhGw',
	kQuitMessage = 'QuGw',
	kRefreshMessage = 'RqGw',
	kForceRefreshMessage = 'FrGw',
	kSettingsChangeMessage = 'ScGw',
	kGeoLocationMessage = 'GlGw',
	kForceGeoLocationMessage = 'GfGw'
};

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

static const char* kViewName = "DeskbarWeatherView";
static const char* kAppMimetype = "application/x-vnd.cpr.DeskbarWeather";

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

class BBitmap;
class BMessageRunner;

class IpApiLocationProvider;
class OpenWeather;
class WeatherSettings;


class DeskbarWeatherView : public BView {

public:
						DeskbarWeatherView(BRect frame);
						DeskbarWeatherView(BMessage* message);
						~DeskbarWeatherView();

	static	DeskbarWeatherView*	Instantiate(BMessage* message);
	static	status_t	GetAppImage(image_info& image);

	virtual	status_t	Archive(BMessage* message, bool deep = true) const;
	virtual	void		AttachedToWindow();
	virtual	void		Draw(BRect updateRect);
	virtual	void		MouseDown(BPoint point);
	virtual	void		MessageReceived(BMessage* message);

	static	BBitmap*	LoadResourceBitmap(const char* name, int32 size);

private:
			void		_AboutRequested();
			void		_Init();
			status_t	_CheckMessageRunner();
			void		_Configure();
			void		_RefreshComplete(BMessage* message);
			void		_GeoLookupComplete(BMessage* message);
			void		_RemoveFromDeskbar();
			void		_ShowPopUpMenu(BPoint point);
			void		_OpenWebSite();
			void		_OpenFolder();
			void		_ShowErrorNotification(const char* title, const char* content);
			void		_ShowForecastWindow(bool toggle = false);
			void		_ShowSettingsWindow();
			void		_ForceRefresh();

	BBitmap*				fIcon;
	IpApiLocationProvider*	fLocationProvider;
	BLocker					fLock;
	BMessageRunner*			fMessageRunner;
	WeatherSettings*		fSettings;
	OpenWeather*			fWeather;
};


#endif	// _DESKBARWEATHERVIEW_H_
