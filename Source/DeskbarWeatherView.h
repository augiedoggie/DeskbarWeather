// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _DESKBARWEATHERVIEW_H_
#define _DESKBARWEATHERVIEW_H_


#include <Locker.h>
#include <View.h>

enum {
	kForecastWindowMessage = 'WnGw',
	kConfigureMessage = 'CfGw',
	kHelpMessage = 'HpGw',
	kGithubMessage = 'GhGw',
	kQuitMessage = 'QuGw',
	kRefreshMessage = 'RqGw',
	kForceRefreshMessage = 'FrGw',
	kSettingsChangeMessage = 'ScGw',
	kGeoLocationMessage = 'GlGw',
	kForceGeoLocationMessage = 'GfGw'
};

static const char* kViewName = "DeskbarWeatherView";
static const char* kAppMimetype = "application/x-vnd.cpr.DeskbarWeather";


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
			void		_ShowForecastWindow();
			void		_ShowConfigureWindow();
			void		_ForceRefresh();

	IpApiLocationProvider*	fLocationProvider;
	OpenWeather*			fWeather;
	BBitmap*				fIcon;
	BLocker					fLock;
	BMessageRunner*			fMessageRunner;
	WeatherSettings*		fWeatherSettings;
};


#endif	// _DESKBARWEATHERVIEW_H_
