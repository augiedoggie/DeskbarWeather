// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _IPAPILOCATIONPROVIDER_H_
#define _IPAPILOCATIONPROVIDER_H_

#include <SupportDefs.h>

class WeatherSettings;

class BInvoker;
class BMessage;


class IpApiLocationProvider {
public:
							IpApiLocationProvider(WeatherSettings* settings, BInvoker* invoker);
							~IpApiLocationProvider();

			status_t		Run();
			status_t		ParseResult(BMessage& data, bool cacheResult = true);

private:
			status_t		_SaveCache(BMessage& message);
			status_t		_LoadCache(BMessage& message);

		BInvoker*			fInvoker;
		WeatherSettings*	fWeatherSettings;
};

#endif	// _IPAPILOCATIONPROVIDER_H_
