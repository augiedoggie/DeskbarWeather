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
						SettingsWindow(WeatherSettings* settings, BInvoker* invoker, BRect frame);
	virtual				~SettingsWindow();

	virtual	void		MessageReceived(BMessage* message);
	virtual	bool		QuitRequested();

private:
			void		_InitControls(bool revert = false);
			void		_SaveSettings();
			BMenu*		_BuildFontMenu();
			status_t	_ResetFontMenu();
			status_t	_HandleFontChange(BMessage* message);
			status_t	_UpdateFontMenu(const char* family, const char* style, double size);

	WeatherSettings*	fSettings;
	WeatherSettings*	fSettingsCache;
	BInvoker*			fInvoker;
	BRadioButton*		fMetricButton;
	BRadioButton*		fImperialButton;
	BTextControl*		fApiKeyControl;
	BTextControl*		fLocationControl;
	BCheckBox*			fLocationBox;
	BCheckBox*			fGeoNotificationBox;
	BCheckBox*			fNotificationBox;
	BCheckBox*			fCompactBox;
	BCheckBox*			fShowForecastBox;
	BMenuField*			fIntervalMenuField;

};

#endif	// _SETTINGSWINDOW_H_
