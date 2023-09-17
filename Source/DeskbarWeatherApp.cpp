// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "DeskbarWeatherView.h"

#include <Alert.h>
#include <Application.h>
#include <Deskbar.h>
#include <Roster.h>
#include <String.h>
#include <iostream>


class DeskbarWeatherApp : public BApplication {
public:
	DeskbarWeatherApp()
		:
		BApplication(kAppMimetype) {}

private:
	void
	_DisplayUsage(const char* name)
	{
		std::cout << "Usage: " << name << " [option]" << std::endl;
		std::cout << "\t--forecast\t\tShow forecast window" << std::endl;
		std::cout << "\t--refresh\t\tRefresh weather" << std::endl;
		std::cout << "\t--geolookup\t\tRefresh geolocation" << std::endl;
	}


	status_t
	_SendReplicantMessage(uint32 what)
	{
		BMessage query(B_GET_PROPERTY);
		query.AddSpecifier("Messenger");
		query.AddSpecifier("View");
		query.AddSpecifier("Replicant", kViewName);
		query.AddSpecifier("Shelf");
		query.AddSpecifier("View", "Status");
		query.AddSpecifier("Window", "Deskbar");

		BMessage reply;
		if (BMessenger("application/x-vnd.Be-TSKB", -1).SendMessage(&query, &reply) != B_OK) {
			std::cout << "Error: couldn't send messenger query to Deskbar" << std::endl;
			reply.PrintToStream();
			return B_ERROR;
		}

		BMessenger replicantMessenger;
		if (reply.FindMessenger("result", &replicantMessenger) != B_OK) {
			std::cout << "Error: no messenger in Deskbar query result" << std::endl;
			reply.PrintToStream();
			return B_ERROR;
		}

		BMessage message(what);
		if (replicantMessenger.SendMessage(what, &reply) != B_OK) {
			std::cout << "Error: couldn't send command to replicant messenger" << std::endl;
			reply.PrintToStream();
			return B_ERROR;
		}

		return B_OK;
	}


public:
	virtual void
	ArgvReceived(int argc, char** argv)
	{
		if (argc > 2) {
			std::cout << "Error: too many arguments" << std::endl;
			_DisplayUsage(argv[0]);
		} else if (argc == 2) {
			if (strcmp(argv[1], "--forecast") == 0) {
				_SendReplicantMessage('WnGw');
			} else if (strcmp(argv[1], "--refresh") == 0) {
				_SendReplicantMessage('FrGw');
			} else if (strcmp(argv[1], "--geolookup") == 0) {
				_SendReplicantMessage('GfGw');
			} else {
				std::cout << "Error: argument not understood" << std::endl;
				_DisplayUsage(argv[0]);
			}
		}

		Quit();
	}


	virtual void
	ReadyToRun()
	{
		BDeskbar deskbar;

		if (!deskbar.HasItem(kViewName)) {
			app_info info;
			status_t result = be_roster->GetRunningAppInfo(find_thread(NULL), &info);
			if (result != B_OK) {
				BString errorString("Roster error: ");
				errorString << strerror(result);
				(new BAlert(NULL, errorString.String(), "OK"))->Go();
			} else {
				result = deskbar.AddItem(&info.ref, NULL);
				if (result != B_OK) {
					BString errorString("Error adding weather view to deskbar: ");
					errorString << strerror(result);
					(new BAlert(NULL, errorString.String(), "OK"))->Go();
				}
			}
		}

		Quit();
	}
};


int
main(int /*argc*/, char** /*argv*/)
{
	DeskbarWeatherApp app;
	app.Run();

	return 0;
}
