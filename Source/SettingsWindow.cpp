// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "SettingsWindow.h"
#include "WeatherSettings.h"

#include <Alert.h>
#include <Button.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <StringView.h>


enum {
	kFontMessage					= 'DwFc',
	kGeoCheckboxMessage				= 'GcGe',
	kImperialMessage				= 'DwIm',
	kMetricMessage					= 'DwMm',
	kNotificationCheckboxMessage	= 'GcNo',
	kShowForecastCheckboxMessage	= 'RnSf',
	kGeoNotificationCheckboxMessage	= 'GcGn',
	kRevertButtonMessage			= 'GcRv',
	kSaveButtonMessage				= 'GcSv',
	kCompactCheckboxMessage			= 'DwCc'
};


SettingsWindow::SettingsWindow(WeatherSettings* prefs, BInvoker* invoker, BRect frame)
	:
	BWindow(frame, "Weather Settings", B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
			B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fCompactBox(NULL),
	fGeoNotificationBox(NULL),
	fImperialButton(NULL),
	fIntervalMenuField(NULL),
	fInvoker(invoker),
	fLocationBox(NULL),
	fLocationControl(NULL),
	fMetricButton(NULL),
	fNotificationBox(NULL),
	fShowForecastBox(NULL),
	fWeatherSettings(prefs),
	fWeatherSettingsCache(new WeatherSettings(dynamic_cast<const WeatherSettings&>(*prefs)))
{
	BTextControl* apiControl = new BTextControl("ApiKeyControl", "API Key:", fWeatherSettings->ApiKey(), NULL);
	apiControl->TextView()->SetExplicitMinSize(BSize(200.0, B_SIZE_UNSET));

	BPopUpMenu* intervalMenu = new BPopUpMenu("IntervalMenu");
	BLayoutBuilder::Menu<>(intervalMenu)
		.AddItem("10 minutes", 10)
		.AddItem("30 minutes", 30)
		.AddItem("1 hour", 60)
		.AddItem("2 hours", 180)
		.AddItem("Manual refresh only", 999999);
	fIntervalMenuField = new BMenuField("IntervalMenuField", "Refresh Interval:", intervalMenu);

	fLocationControl = new BTextControl("LocationControl", "Location:", fWeatherSettings->Location(), NULL);
	fLocationControl->TextView()->SetExplicitMinSize(BSize(200.0, B_SIZE_UNSET));

	fLocationBox = new BCheckBox("GeoLocationCheckBox", "Use GeoLocation lookup", new BMessage(kGeoCheckboxMessage));
	fLocationBox->SetEnabled(false); // disable until we have working geocoding

	BStringView* unitsView = new BStringView("UnitStringView", "Units:");
//	unitsView->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_VERTICAL_UNSET));

	fImperialButton = new BRadioButton("ImperialRadio", "Imperial", new BMessage(kImperialMessage));
	fMetricButton = new BRadioButton("MetricRadio", "Metric", new BMessage(kMetricMessage));

	BMenuField* fontMenuField = new BMenuField("FontMenuField", "Font:", _BuildFontMenu());

	fNotificationBox = new BCheckBox("NotificationCheckBox", "Show notification after refresh", new BMessage(kNotificationCheckboxMessage));

	fGeoNotificationBox = new BCheckBox("GeoNotificationCheckBox", "Show notification after lookup", new BMessage(kGeoNotificationCheckboxMessage));

	fShowForecastBox = new BCheckBox("ShowForecastCheckBox", "Clicking the notification opens the forecast", new BMessage(kShowForecastCheckboxMessage));

	fCompactBox = new BCheckBox("CompactForecastBox", "Use compact forecast window", new BMessage(kCompactCheckboxMessage));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.AddGrid(0.0, B_USE_HALF_ITEM_INSETS)
			.AddTextControl(apiControl, 0, 0, B_ALIGN_RIGHT)
			.AddMenuField(fIntervalMenuField, 0, 1, B_ALIGN_RIGHT)
			.Add(fNotificationBox, 1, 2)
			.Add(fShowForecastBox, 1, 3)
			.AddTextControl(fLocationControl, 0, 4, B_ALIGN_RIGHT)
			.Add(fLocationBox, 1, 5)
			.Add(fGeoNotificationBox, 1, 6)
//			.Add(BSpaceLayoutItem::CreateGlue(), 0, 4, 1, 1)
			.AddGroup(B_HORIZONTAL, 0.0, 0, 7, 1, 1)
				.SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE))
				.Add(unitsView)
				.AddStrut(be_control_look->DefaultLabelSpacing())
			.End()
			.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING, 1, 7, 1, 1)
				.SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP))
				.Add(fImperialButton)
				.AddStrut(be_control_look->DefaultItemSpacing())
				.Add(fMetricButton)
			.End()
			.AddMenuField(fontMenuField, 0, 8, B_ALIGN_RIGHT)
			.Add(fCompactBox, 1, 9)
//			.Add(BSpaceLayoutItem::CreateGlue(), 2, 0, 1, 5)
		.End()
		.AddGlue()
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
//			.AddGlue()
			.Add(new BButton("RevertButton", "Revert", new BMessage(kRevertButtonMessage)))
			.Add(new BButton("CloseButton", "Close", new BMessage(kSaveButtonMessage)))
		.End()
		.End();

	_InitControls();

	Lock();
	CenterOnScreen();
	Show();
	Unlock();
}


SettingsWindow::~SettingsWindow()
{
	delete fInvoker;
	delete fWeatherSettingsCache;
}


void
SettingsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case 10:
		case 30:
		case 60:
		case 180:
		case 999999:
			if (fWeatherSettings->RefreshInterval() != (int32)message->what) {
				fWeatherSettings->SetRefreshInterval(message->what);
				fInvoker->Invoke();
			}
			break;
		case kCompactCheckboxMessage:
		{
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fWeatherSettings->CompactForecast() != value)
				fWeatherSettings->SetCompactForecast(value);

			break;
		}
		case kGeoCheckboxMessage:
		{
			BCheckBox* useGeoCheckbox = dynamic_cast<BCheckBox*>(FindView("GeoLocationCheckBox"));
			if (useGeoCheckbox == NULL)
				return;

			BTextControl* locationControl = dynamic_cast<BTextControl*>(FindView("LocationControl"));
			if (locationControl == NULL)
				return;

			locationControl->SetEnabled(!useGeoCheckbox->Value());
			fGeoNotificationBox->SetEnabled(useGeoCheckbox->Value());

			if (fWeatherSettings->UseGeoLocation() != useGeoCheckbox->Value()) {
				fWeatherSettings->SetUseGeoLocation(useGeoCheckbox->Value());
				fInvoker->Invoke();
			}
			break;
		}
		case kGeoNotificationCheckboxMessage:
		{
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fWeatherSettings->UseGeoNotification() != value)
				fWeatherSettings->SetUseGeoNotification(value);

			break;
		}
		case kNotificationCheckboxMessage:
		{
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fWeatherSettings->UseNotification() != value)
				fWeatherSettings->SetUseNotification(value);

			fShowForecastBox->SetEnabled(fWeatherSettings->UseNotification());

			break;
		}
		case kShowForecastCheckboxMessage:
		{
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fWeatherSettings->NotificationClick() != value)
				fWeatherSettings->SetNotificationClick(value);

			break;
		}
		case kImperialMessage:
			if (!fWeatherSettings->ImperialUnits()) {
				fWeatherSettings->SetImperialUnits(true);
				fInvoker->Invoke();
			}
			break;
		case kMetricMessage:
			if (fWeatherSettings->ImperialUnits()) {
				fWeatherSettings->SetImperialUnits(false);
				fInvoker->Invoke();
			}
			break;
		case kFontMessage:
			if (_UpdateFontMenu(message) != B_OK)
				(new BAlert("Error", "Font Error!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			break;
		case kSaveButtonMessage:
			_SaveSettings();
			break;
		case kRevertButtonMessage:
			_InitControls(true);
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


void
SettingsWindow::_InitControls(bool revert)
{
	if (revert) {
		fWeatherSettings->SetImperialUnits(fWeatherSettingsCache->ImperialUnits());
		fWeatherSettings->SetUseGeoLocation(fWeatherSettingsCache->UseGeoLocation());
		fWeatherSettings->SetLocation(fWeatherSettingsCache->Location());
		fWeatherSettings->SetRefreshInterval(fWeatherSettingsCache->RefreshInterval());
		fWeatherSettings->SetUseNotification(fWeatherSettingsCache->UseNotification());
		fWeatherSettings->SetNotificationClick(fWeatherSettingsCache->NotificationClick());
		//TODO revert font
	}

	if (fWeatherSettings->ImperialUnits())
		fImperialButton->SetValue(1);
	else
		fMetricButton->SetValue(1);

	if (fWeatherSettings->UseGeoLocation()) {
		fLocationBox->SetValue(1);
		fLocationControl->SetEnabled(false);
	} else
		fGeoNotificationBox->SetEnabled(false);

	if (fWeatherSettings->UseGeoNotification())
		fGeoNotificationBox->SetValue(1);

	if (fWeatherSettings->NotificationClick())
		fShowForecastBox->SetValue(1);

	if (fWeatherSettings->UseNotification())
		fNotificationBox->SetValue(1);
	else
		fShowForecastBox->SetEnabled(false);

	BMenu* intervalMenu = fIntervalMenuField->Menu();
	BMenuItem* selectedItem = intervalMenu->FindItem(fWeatherSettings->RefreshInterval());
	if (selectedItem != NULL)
		selectedItem->SetMarked(true);
	//TODO show error?

	if (fWeatherSettings->CompactForecast())
		fCompactBox->SetValue(1);

//	if (revert)
	//TODO send settings change message
}


void
SettingsWindow::_SaveSettings()
{
	bool needRefresh = false;

	BTextControl* control = dynamic_cast<BTextControl*>(FindView("ApiKeyControl"));
	if (control != NULL) {
		const char* text = control->Text();
		if (fWeatherSettings->ApiKey() == NULL) {
			if (strlen(text) > 0) {
				fWeatherSettings->SetApiKey(text);
				needRefresh = true;
			}
		} else if (strcmp(fWeatherSettings->ApiKey(), text) != 0) {
			if (strlen(text) == 0)
				fWeatherSettings->SetApiKey(NULL);
			else
				fWeatherSettings->SetApiKey(text);
			needRefresh = true;
		}
	}

	control = dynamic_cast<BTextControl*>(FindView("LocationControl"));
	if (control != NULL) {
		if (strcmp(fWeatherSettings->Location(), control->Text()) != 0) {
			fWeatherSettings->SetLocation(control->Text());
			needRefresh = true;
		}
	}

	if (needRefresh)
		fInvoker->Invoke();

	Quit();
}


BMenu*
SettingsWindow::_BuildFontMenu()
{
	BFont origFont;
	fWeatherSettings->GetFont(origFont);
	font_family origFamily;
	font_style origStyle;
	origFont.GetFamilyAndStyle(&origFamily, &origStyle);
	BString menuLabelStr;
	menuLabelStr.SetToFormat("%s - %s - %g", origFamily, origStyle, origFont.Size());
	BPopUpMenu* menu = new BPopUpMenu(menuLabelStr, false);
	menu->SetLabelFromMarked(false);

	int32 numFamilies = count_font_families();
	font_family family;
	for (int32 i = 0; i < numFamilies; i++) {
		bool inFamily = false;
		if (get_font_family(i, &family, NULL) != B_OK)
			continue;

		BMenu* familyMenu = new BMenu(family);
		menu->AddItem(familyMenu);
		familyMenu->SetRadioMode(false);
		if (strcmp(family, origFamily) == 0) {
			BMenuItem* item = menu->FindItem(family);
			item->SetMarked(true);
			inFamily = true;
		}

		font_style style;
		int32 numStyles = count_font_styles(family);
		for (int32 j = 0; j < numStyles; j++) {
			bool inStyle = false;
			if (get_font_style(family, j, &style) != B_OK)
				continue;

			BMenu* styleMenu = new BMenu(style);
			familyMenu->AddItem(styleMenu);
			styleMenu->SetRadioMode(false);
			if (inFamily && strcmp(style, origStyle) == 0) {
				BMenuItem* item = familyMenu->FindItem(style);
				item->SetMarked(true);
				inStyle = true;
			}

			for (double k = 8.0; k < 16.5; k += 0.5) {
				BString itemStr;
				itemStr.SetToFormat("%g", k);
				BMessage* message = new BMessage(kFontMessage);
				message->AddString("FontFamily", family);
				message->AddString("FontStyle", style);
				message->AddDouble("FontSize", k);
				BMenuItem* sizeItem = new BMenuItem(itemStr, message);
				styleMenu->AddItem(sizeItem);
				if (inStyle && k == origFont.Size())
					sizeItem->SetMarked(true);
			}
		}
	}

	//TODO ensure we found at least one item to mark
//	if (menu->FindMarked() == NULL)
//		menu->ItemAt(0)->SetMarked(true);

	menu->SetTargetForItems(this);

	return menu;
}


status_t
SettingsWindow::_UpdateFontMenu(BMessage* message)
{
	const char* family = NULL;
	if (message->FindString("FontFamily", &family) != B_OK)
		return B_ERROR;

	const char* style;
	if (message->FindString("FontStyle", &style) != B_OK)
		return B_ERROR;

	double size = -1.0;
	if (message->FindDouble("FontSize", &size) != B_OK)
		return B_ERROR;

	BMenuField* menuField = dynamic_cast<BMenuField*>(FindView("FontMenuField"));
	if (menuField == NULL)
		return B_ERROR;

	BMenu* menu = menuField->Menu();
	for (int32 i = 0; i < menu->CountItems(); i++) {
		bool inFamily = false;
		BMenuItem* familyItem = menu->ItemAt(i);
		if (strcmp(family, familyItem->Label()) == 0)
			inFamily = true;

		familyItem->SetMarked(inFamily);

		BMenu* familyMenu = menu->SubmenuAt(i);
		for (int32 j = 0; j < familyMenu->CountItems(); j++) {
			bool inStyle = false;
			BMenuItem* styleItem = familyMenu->ItemAt(j);
			if (inFamily && strcmp(style, styleItem->Label()) == 0)
				inStyle = true;

			styleItem->SetMarked(inStyle);

			BMenu* styleMenu = familyMenu->SubmenuAt(j);
			for (int32 k = 0; k < styleMenu->CountItems(); k++) {
				BMenuItem* sizeItem = styleMenu->ItemAt(k);
				sizeItem->SetMarked(inStyle && atof(sizeItem->Label()) == size);
				//TODO ensure we found at least one item to mark
			}
		}
	}

	BString menuLabelStr;
	menuLabelStr.SetToFormat("%s - %s - %g", family, style, size);
	menuField->MenuItem()->SetLabel(menuLabelStr);

	fWeatherSettings->SetFont(family, style, size);
	fInvoker->Invoke();

	return B_OK;
}
