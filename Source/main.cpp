// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include <Application.h>
#include <Alert.h>
#include <Deskbar.h>
#include <Roster.h>
#include <String.h>

#include "DeskbarWeatherView.h"


int
main(int /*argc*/, char** /*argv*/)
{
	BApplication app("application/x-vnd.cpr.DeskbarWeather");

	BDeskbar deskbar;

	if (deskbar.HasItem("DeskbarWeatherView"))
		return 0;

	app_info info;
	status_t result = be_roster->GetRunningAppInfo(find_thread(NULL), &info);
	if (result != B_OK) {
		BString errorString("Roster error: ");
		errorString << strerror(result);
		(new BAlert(NULL, errorString.String(), "OK"))->Go();
		return 1;
	}

//	result = deskbar.AddItem(new DeskbarWeatherView(), NULL);
	result = deskbar.AddItem(&info.ref, NULL);
	if (result != B_OK) {
		BString errorString("Error adding weather view to deskbar: ");
		errorString << strerror(result);
		(new BAlert(NULL, errorString.String(), "OK"))->Go();
		return 1;
	}

	return 0;
}
