// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Chris Roberts

#ifndef _WEATHERTOOLTIP_H_
#define _WEATHERTOOLTIP_H_


#include <ToolTip.h>

class OpenMeteo;
class WeatherSettings;

class BFont;
class BStringView;


class WeatherToolTip : public BToolTip {

public:
	WeatherToolTip(WeatherSettings* settings, OpenMeteo* weather);
	virtual BView*	View() const;

private:
	BStringView*	_BuildStringView(const char* name, const char* label, alignment align, BFont* font);

	BView*			fView;
};


#endif // _WEATHERTOOLTIP_H_
