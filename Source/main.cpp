// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include <iostream>

#include <Application.h>
#include <Alert.h>
#include <Deskbar.h>
#include <Roster.h>
#include <String.h>

#include "DeskbarWeatherView.h"


void
display_usage(const char* name)
{
	std::cout << "Usage: " << name << " [option]" << std::endl;
	std::cout << "\t--forecast\t\tShow forecast window" << std::endl;
	std::cout << "\t--refresh\t\tRefresh weather" << std::endl;
	std::cout << "\t--geolookup\t\tRefresh geolocation" << std::endl;
	exit(1);
}


status_t
send_replicant_message(uint32 what)
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


int
main(int argc, char** argv)
{
	if (argc > 2) {
		std::cout << "Error: Too many arguments" << std::endl;
		display_usage(argv[0]);
	} else if (argc == 2) {
		if (strcmp(argv[1], "--forecast") == 0) {
			send_replicant_message('WnGw');
			return 0;
		} else if (strcmp(argv[1], "--refresh") == 0) {
			send_replicant_message('FrGw');
			return 0;
		} else if (strcmp(argv[1], "--geolookup") == 0) {
			send_replicant_message('GfGw');
			return 0;
		}
		std::cout << "Error: argument not understood" << std::endl;
		display_usage(argv[0]);
	}

	BApplication app(kAppMimetype);

	BDeskbar deskbar;

	if (deskbar.HasItem(kViewName))
		return 0;

	app_info info;
	status_t result = be_roster->GetRunningAppInfo(find_thread(NULL), &info);
	if (result != B_OK) {
		BString errorString("Roster error: ");
		errorString << strerror(result);
		(new BAlert(NULL, errorString.String(), "OK"))->Go();
		return 1;
	}

	result = deskbar.AddItem(&info.ref, NULL);
	if (result != B_OK) {
		BString errorString("Error adding weather view to deskbar: ");
		errorString << strerror(result);
		(new BAlert(NULL, errorString.String(), "OK"))->Go();
		return 1;
	}

	return 0;
}
