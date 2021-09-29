// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _IPAPILOCATIONPROVIDER_H_
#define _IPAPILOCATIONPROVIDER_H_

#include <SupportDefs.h>

class WeatherSettings;

class BInvoker;
class BMessage;
namespace BPrivate {
	namespace Network {
		class BUrlRequest;
	}
}

static const char* kGeoLookupCacheKey = "dw:GeoLookupCache";


using namespace BPrivate::Network;


class IpApiLocationProvider {
public:
							IpApiLocationProvider(WeatherSettings* settings, BInvoker* invoker);
							~IpApiLocationProvider();

			status_t		Run(bool force = false);
			status_t		ParseResult(BMessage& data, bool cacheResult = true);

private:
			status_t		_SaveCache(BMessage& message);
			status_t		_LoadCache(BMessage& message);

		BInvoker*			fInvoker;
		WeatherSettings*	fWeatherSettings;
		BUrlRequest*		fUrlRequest;
};

#endif	// _IPAPILOCATIONPROVIDER_H_
