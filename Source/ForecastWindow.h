// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _FORECASTWINDOW_H_
#define _FORECASTWINDOW_H_


#include <Window.h>

class BBitmap;
class BStringView;

class OpenWeather;


class ForecastWindow : public BWindow {

public:
		ForecastWindow(OpenWeather* weather, BRect frame, const char* location, bool compact);

private:
		BStringView*	_BuildStringView(const char* name, const char* label, alignment align, BFont* font = NULL);
};


#endif	// _FORECASTWINDOW_H_
