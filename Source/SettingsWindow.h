// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _SETTINGSWINDOW_H_
#define _SETTINGSWINDOW_H_


#include <Locker.h>
#include <Window.h>


class BCheckBox;
class BInvoker;
class BMenu;
class BMenuField;
class BRadioButton;
class BTextControl;

class WeatherSettings;


class SettingsWindow : public BWindow {

public:
						SettingsWindow(WeatherSettings* settings, BLocker& lock, BInvoker* invoker, BRect frame);
	virtual				~SettingsWindow();

	virtual	void		MessageReceived(BMessage* message);
	virtual	bool		QuitRequested();

private:
			void		_InitControls(bool revert = false);
			void		_SaveSettings();
			BMenu*		_BuildFontMenu();
			status_t	_UpdateFontMenu(BMessage* message);

	WeatherSettings*	fSettings;
	WeatherSettings*	fSettingsCache;
	BInvoker*			fInvoker;
	BRadioButton*		fMetricButton;
	BRadioButton*		fImperialButton;
	BTextControl*		fLocationControl;
	BCheckBox*			fLocationBox;
	BCheckBox*			fGeoNotificationBox;
	BCheckBox*			fNotificationBox;
	BCheckBox*			fCompactBox;
	BCheckBox*			fShowForecastBox;
	BMenuField*			fIntervalMenuField;
	BLocker*			fLock;

};

#endif	// _SETTINGSWINDOW_H_
