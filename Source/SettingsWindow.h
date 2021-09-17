// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _SETTINGSWINDOW_H_
#define _SETTINGSWINDOW_H_


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
						SettingsWindow(WeatherSettings* prefs, BInvoker* invoker, BRect frame);
	virtual				~SettingsWindow();

	virtual	void		MessageReceived(BMessage* message);

private:
			void		_InitControls(bool revert = false);
			void		_UpdatePrefs();
			BMenu*		_BuildFontMenu();
			status_t	_UpdateFontMenu(BMessage* message);

	WeatherSettings*	fWeatherSettings;
	WeatherSettings*	fWeatherSettingsCache;
	BInvoker*			fInvoker;
	BRadioButton*		fMetricButton;
	BRadioButton*		fImperialButton;
	BTextControl*		fLocationControl;
	BCheckBox*			fLocationBox;
	BCheckBox*			fNotificationBox;
	BMenuField*			fIntervalMenuField;

};

#endif	// _SETTINGSWINDOW_H_
