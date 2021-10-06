// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "DeskbarWeatherView.h"
#include "Condition.h"
#include "IpApiLocationProvider.h"
#include "ForecastWindow.h"
#include "OpenWeather.h"
#include "SettingsWindow.h"
#include "WeatherSettings.h"

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <Deskbar.h>
#include <IconUtils.h>
#include <Invoker.h>
#include <LayoutBuilder.h>
#include <MenuItem.h>
#include <MessageRunner.h>
#include <Notification.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <private/netservices/HttpRequest.h>


using namespace BPrivate::Network;


const uint32 kForecastWindowMessage = 'WnGw';
const uint32 kConfigureMessage = 'CfGw';
const uint32 kHelpMessage = 'HpGw';
const uint32 kQuitMessage = 'QuGw';
const uint32 kRefreshMessage = 'RqGw';
const uint32 kForceRefreshMessage = 'FrGw';
const uint32 kSettingsChangeMessage = 'ScGw';
const uint32 kGeoLocationMessage = 'GlGw';
const uint32 kForceGeoLocationMessage = 'GfGw';

const char* kViewName = "DeskbarWeatherView";
const char* kAppMimetype = "application/x-vnd.cpr.DeskbarWeather";


extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth, float maxHeight)
{
	//TODO init weathersetttings, calculate width for the current font, and pass settings object to the view
	BRect frame(0, 0, 39, maxHeight - 1); // 129 x 16 max?
	return new DeskbarWeatherView(frame);
}


DeskbarWeatherView::DeskbarWeatherView(BRect frame)
	:
	BView(frame, kViewName, B_FOLLOW_NONE, B_WILL_DRAW),
	fIcon(NULL),
	fLocationProvider(NULL),
	fMessageRunner(NULL),
	fWeather(NULL),
	fWeatherSettings(NULL)
{
	_Init();
}


DeskbarWeatherView::DeskbarWeatherView(BMessage* message)
	:
	BView(message),
	fIcon(NULL),
	fLocationProvider(NULL),
	fMessageRunner(NULL),
	fWeather(NULL),
	fWeatherSettings(NULL)
{
	_Init();
}


DeskbarWeatherView::~DeskbarWeatherView()
{
	//TODO close any open forecast/settings windows
	delete fIcon;
	delete fMessageRunner;
	delete fWeather;
	delete fLocationProvider;
	delete fWeatherSettings;
}


DeskbarWeatherView*
DeskbarWeatherView::Instantiate(BMessage* message)
{
	if (!validate_instantiation(message, kViewName))
		return NULL;

	return new DeskbarWeatherView(message);
}


status_t
DeskbarWeatherView::Archive(BMessage* message, bool deep) const
{
	BView::Archive(message, deep);
	message->AddString("add_on", kAppMimetype);
	message->AddString("class", kViewName);
	return B_OK;
}


void
DeskbarWeatherView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	SetLowColor(ViewColor());

	fWeather = new OpenWeather(fWeatherSettings, new BInvoker(new BMessage(kRefreshMessage), this));

	_CheckMessageRunner();

	if (fWeatherSettings->UseGeoLocation()) {
		fLocationProvider = new IpApiLocationProvider(fWeatherSettings, new BInvoker(new BMessage(kGeoLocationMessage), this));
		fLocationProvider->Run(); // will force a weather refresh when the reply message arrives
	} else
		BMessenger(this).SendMessage(kForceRefreshMessage);
}


void
DeskbarWeatherView::MouseDown(BPoint point)
{
	BMessage* message = Looper()->CurrentMessage();
	if (message == NULL || message->what != B_MOUSE_DOWN)
		return;

	int32 buttons;

	if (message->FindInt32("buttons", &buttons) != B_OK)
		return;

	if (buttons == B_PRIMARY_MOUSE_BUTTON)
		_ShowForecastWindow();
	else
		_ShowPopUpMenu(point);
}


void
DeskbarWeatherView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kForecastWindowMessage:
			_ShowForecastWindow();
			break;
		case kConfigureMessage:
			_ShowConfigureWindow();
			break;
		case kSettingsChangeMessage:
		{
			//TODO check for geolocation status change
			fWeather->RebuildRequestUrl();
			_CheckMessageRunner();
			BFont newFont, oldFont;
			fWeatherSettings->GetFont(newFont);
			GetFont(&oldFont);
			if (oldFont != newFont) {
				//TODO calculate string width and see if we need to restart the replicant to change view size
				SetFont(&newFont);
				Invalidate();
			}
			break;
		}
		case kForceRefreshMessage:
			_ForceRefresh();
			break;
		case kRefreshMessage:
			_RefreshComplete(message);
			break;
		case kForceGeoLocationMessage:
			if (fLocationProvider != NULL)
				fLocationProvider->Run(true); // will force a weather refresh when the reply message arrives
			break;
		case kGeoLocationMessage:
			_GeoLookupComplete(message);
			break;
		case kQuitMessage:
			_RemoveFromDeskbar();
			break;
		default:
			BView::MessageReceived(message);
	}
}


void
DeskbarWeatherView::Draw(BRect updateRect)
{
	float maxHeight = Bounds().Height();

	if (fIcon != NULL) {
		SetDrawingMode(B_OP_ALPHA);
		DrawBitmap(fIcon);
		SetDrawingMode(B_OP_OVER);
	} else {
		BRect iconRect(0, 0, maxHeight - 1, maxHeight - 1);
		rgb_color origColor = HighColor();
		SetHighColor({ 0, 100, 255, 255 });
		FillRect(iconRect);
		SetHighColor(origColor);
	}

	font_height fontHeight;
	GetFontHeight(&fontHeight);

	float textY = (maxHeight / 2) + ((fontHeight.ascent - fontHeight.descent) / 2);

	// maxHeight + 1 moves to 17 in the X to give an extra pixel padding
	MovePenTo(maxHeight + 1, textY + 1);

	BString tempString;
	//TODO fix text being cut off during decimal display in celsius mode
	if (fWeather != NULL && fWeather->Current() != NULL)
		if (fWeatherSettings->ImperialUnits())
			tempString << fWeather->Current()->iTemp() << "°";
		else
			tempString.SetToFormat("%.1f°", fWeather->Current()->Temp());
	else
		tempString << "??°";

	DrawString(tempString.String());

	BView::Draw(updateRect);
}


void
DeskbarWeatherView::_Init()
{
	fWeatherSettings = new WeatherSettings();

	float iconHeight = Bounds().Height() - 1;
	fIcon = new BBitmap(BRect(0, 0, iconHeight, iconHeight), B_RGBA32);
	if (fIcon->InitCheck() != B_OK) {
		delete fIcon;
		fIcon = NULL;
	} else
		_LoadIcon("unknown");

	BFont font;
	if (fWeatherSettings->GetFont(font) == B_OK)
		SetFont(&font);
}


status_t
DeskbarWeatherView::_CheckMessageRunner()
{
	if (fWeatherSettings->ApiKey() == NULL) {
		BMessenger(this).SendMessage(kConfigureMessage);
		SetToolTip("ERROR: API Key not set");
		return B_ERROR;
	}

	//TODO check if we have a valid location(latitude/longitude)

	if (fWeatherSettings->RefreshInterval() < 0) {
		// automatic updates are disabled, remove any existing runner and return
		if (fMessageRunner != NULL) {
			delete fMessageRunner;
			fMessageRunner = NULL;
		}
		return B_OK;
	}

	if (fMessageRunner == NULL) {
		BMessage bufMsg(kForceRefreshMessage);
		fMessageRunner = new BMessageRunner(BMessenger(this), &bufMsg, (bigtime_t)fWeatherSettings->RefreshInterval() * 60000000, -1);
	} else
		fMessageRunner->SetInterval((bigtime_t)fWeatherSettings->RefreshInterval() * 60000000);

	if (fMessageRunner->InitCheck() == B_OK)
		return B_OK;

	delete fMessageRunner;
	fMessageRunner = NULL;
	//TODO use notification instead?
	(new BAlert("Error", "MessageRunner Error!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();

	return B_ERROR;
}


void
DeskbarWeatherView::_ShowForecastWindow()
{
	//TODO check for existing window and activate it
	if (fWeather->Current() != NULL)
		new ForecastWindow(fWeather, BRect(100, 100, 500, 300), fWeatherSettings->Location(), fWeatherSettings->CompactForecast());
}


void
DeskbarWeatherView::_ShowConfigureWindow()
{
	//TODO check for existing window and activate it
	new SettingsWindow(fWeatherSettings, new BInvoker(new BMessage(kSettingsChangeMessage), this), BRect(100, 100, 500, 300));
}


void
DeskbarWeatherView::_ForceRefresh()
{
	if (fWeatherSettings->ApiKey() == NULL)
		return;

	//TODO check if we have a valid location(latitude/longitude)

	if (fMessageRunner == NULL)  // may not have been started if no api key was set
		_CheckMessageRunner();

	fWeather->Refresh();
}


status_t
DeskbarWeatherView::GetAppImage(image_info& image)
{
	int32 cookie = 0;
	while (get_next_image_info(B_CURRENT_TEAM, &cookie, &image) == B_OK) {
		if ((char*)DeskbarWeatherView::GetAppImage >= (char*)image.text
			&& (char*)DeskbarWeatherView::GetAppImage <= (char*)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}


void
DeskbarWeatherView::_RefreshComplete(BMessage* message)
{
	int32 status = message->GetInt32("re:code", -1);
	BString response(message->GetString("re:message", "BMessage Error"));

	if (BHttpRequest::IsSuccessStatusCode(status)) {
		if (fWeather->ParseResult(*message) != B_OK) {
			//always send bad notification
			BNotification notification(B_ERROR_NOTIFICATION);
			notification.SetGroup("DeskbarWeather");
			notification.SetTitle("Json Parse Error");
			//TODO add a more descriptive error message
			notification.SetContent("There was an error parsing the returned weather data!");
			notification.Send();
		} else if (fWeatherSettings->UseNotification()) {
			//send good notification if they're enabled
			BNotification notification(B_INFORMATION_NOTIFICATION);
			notification.SetGroup("DeskbarWeather");
			notification.SetTitle("Weather Refresh Complete");
			BString content(fWeatherSettings->Location());
			//TODO configurable notification information
			content << "\n\n" << fWeather->Current()->Forecast()->String() << "\n\n" << fWeather->Current()->Temp() << "°";
			notification.SetContent(content);
			notification.Send();
		}
	} else {
		//always send bad notification
		BNotification notification(B_ERROR_NOTIFICATION);
		notification.SetGroup("DeskbarWeather");
		notification.SetTitle("Weather Refresh Error");
		BString content("Error: ");
		content << response;
		notification.SetContent(content);
		notification.Send();
	}
	_LoadIcon(fWeather->Current()->Icon()->String());

	BString updateStr;
	fWeather->LastUpdate(updateStr);
	//TODO configurable tooltip information
	BString tooltip;
	tooltip << fWeatherSettings->Location() << "\n";
	tooltip << fWeather->Current()->Forecast()->String() << "\n";
	tooltip << "High: " << fWeather->Current()->iHigh() << "°\n";
	tooltip << "Low: " << fWeather->Current()->iLow() << "°\n";
	tooltip << "Updated: " << updateStr;
	SetToolTip(tooltip);

	Invalidate();
}


void
DeskbarWeatherView::_GeoLookupComplete(BMessage* message)
{
	int32 status = message->GetInt32("re:code", -1);
	BString response(message->GetString("re:message", "BMessage Error"));

	if (!BHttpRequest::IsSuccessStatusCode(status)) {
		BNotification notification(B_ERROR_NOTIFICATION);
		notification.SetGroup("DeskbarWeather");
		notification.SetTitle("GeoLocation Lookup Error");
		BString content("GeoLocation Error: ");
		content << response;
		notification.SetContent(content);
		notification.Send();
		return;
	}

	if (fLocationProvider->ParseResult(*message) != B_OK) {
		BNotification notification(B_ERROR_NOTIFICATION);
		notification.SetGroup("DeskbarWeather");
		notification.SetTitle("GeoLocation Json Parse Error");
		notification.SetContent("There was an error parsing the returned location data!");
		notification.Send();
		return;
	}

	if (fWeatherSettings->UseGeoNotification()) {
		BNotification notification(B_INFORMATION_NOTIFICATION);
		notification.SetGroup("DeskbarWeather");
		notification.SetTitle("GeoLocation Refresh Complete");
		BString content;
		content.SetToFormat("%s\n\nLatitude: %.4f\n\nLongitude: %.4f", fWeatherSettings->Location(), fWeatherSettings->Latitude(), fWeatherSettings->Longitude());
		if (message->HasBool(kGeoLookupCacheKey))
			content << "\n\n(using cached location)";
		notification.SetContent(content);
		notification.Send();
	}

	_ForceRefresh();
}


void
DeskbarWeatherView::_RemoveFromDeskbar()
{
	BDeskbar deskbar;
	status_t status = deskbar.RemoveItem(kViewName);

	if (status != B_OK) {
		BString error("Error removing DeskbarWeather: ");
		error << strerror(status);
		(new BAlert(NULL, error.String(), "OK"))->Go();
	}
}


void
DeskbarWeatherView::_ShowPopUpMenu(BPoint point)
{
	BMenuItem* forecastItem = NULL;
	BMenuItem* refreshItem = NULL;
	BPopUpMenu* popupmenu = new BPopUpMenu("Menu");
	BLayoutBuilder::Menu<> builder = BLayoutBuilder::Menu<>(popupmenu)
		.AddItem("Open Forecast Window", kForecastWindowMessage)
		.GetItem(forecastItem)
		.AddItem("Refresh Weather", kForceRefreshMessage)
		.GetItem(refreshItem);

	if (fWeatherSettings->UseGeoLocation())
		builder.AddItem("Refresh GeoLocation", kForceGeoLocationMessage);

	builder
		.AddSeparator()
		.AddItem("Settings" B_UTF8_ELLIPSIS, kConfigureMessage)
		.AddItem("Help", kHelpMessage).SetEnabled(false)
		.AddItem("Quit", kQuitMessage);

	// disable items if we have no data to show
	if (fWeather == NULL && refreshItem != NULL)
		refreshItem->SetEnabled(false);

	if ((fWeather == NULL || fWeather->Current() == NULL) && forecastItem != NULL)
		forecastItem->SetEnabled(false);

	popupmenu->SetTargetForItems(this);

	popupmenu->Go(ConvertToScreen(point), true, true);

	delete popupmenu;
}


void
DeskbarWeatherView::_LoadIcon(const char* name)
{
	if (fIcon == NULL)
		return;

	image_info info;
	if (DeskbarWeatherView::GetAppImage(info) != B_OK)
		return;

	BFile file(info.name, B_READ_ONLY);
	if (file.InitCheck() < B_OK)
		return;

	BResources resources(&file);

	size_t size;
	const uint8* data = static_cast<const uint8*>(resources.LoadResource(B_VECTOR_ICON_TYPE, name, &size));
	if (data != NULL)
		BIconUtils::GetVectorIcon(data, size, fIcon);
}
