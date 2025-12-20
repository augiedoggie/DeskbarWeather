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
#include <private/shared/AutoLocker.h>


enum {
	kFontMessage					= 'DwFc',
	kGeoCheckboxMessage				= 'GcGe',
	kImperialMessage				= 'DwIm',
	kMetricMessage					= 'DwMm',
	kNotificationCheckboxMessage	= 'GcNo',
	kShowForecastCheckboxMessage	= 'RnSf',
	kGeoNotificationCheckboxMessage	= 'GcGn',
	kResetFontMessage				= 'GcRf',
	kRevertButtonMessage			= 'GcRv',
	kShowFeelsLikeCheckboxMessage	= 'DwFl',
	kCompactCheckboxMessage			= 'DwCc'
};


SettingsWindow::SettingsWindow(WeatherSettings* settings, BInvoker* invoker, BRect frame)
	:
	BWindow(frame, "DeskbarWeather Preferences", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
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
	fSettings(settings),
	fSettingsCache(new WeatherSettings(dynamic_cast<const WeatherSettings&>(*settings))),
	fShowFeelsLikeBox(NULL),
	fShowForecastBox(NULL)
{
	AutoLocker<WeatherSettings> slocker(fSettings);

	BPopUpMenu* intervalMenu = new BPopUpMenu("IntervalMenu");
	BLayoutBuilder::Menu<>(intervalMenu)
		.AddItem("15 minutes", 15)
		.AddItem("30 minutes", 30)
		.AddItem("1 hour", 60)
		.AddItem("2 hours", 180)
		.AddItem("Manual refresh only", 999999);
	fIntervalMenuField = new BMenuField("IntervalMenuField", "Refresh Interval:", intervalMenu);

	fLocationControl = new BTextControl("LocationControl", "Location:", fSettings->Location(), NULL);
	fLocationControl->TextView()->SetExplicitMinSize(BSize(200.0, B_SIZE_UNSET));

	fLocationBox = new BCheckBox("GeoLocationCheckBox", "Use GeoLocation lookup", new BMessage(kGeoCheckboxMessage));
	fLocationBox->SetEnabled(false); // disable until we have working geocoding

	BStringView* unitsView = new BStringView("UnitStringView", "Units:");

	fImperialButton = new BRadioButton("ImperialRadio", "Imperial", new BMessage(kImperialMessage));
	fMetricButton = new BRadioButton("MetricRadio", "Metric", new BMessage(kMetricMessage));

	BMenuField* fontMenuField = new BMenuField("FontMenuField", "Font:", _BuildFontMenu());

	fNotificationBox = new BCheckBox("NotificationCheckBox", "Show notification after refresh", new BMessage(kNotificationCheckboxMessage));

	fGeoNotificationBox = new BCheckBox("GeoNotificationCheckBox", "Show notification after lookup", new BMessage(kGeoNotificationCheckboxMessage));

	fShowForecastBox = new BCheckBox("ShowForecastCheckBox", "Clicking the notification opens the forecast", new BMessage(kShowForecastCheckboxMessage));

	fCompactBox = new BCheckBox("CompactForecastBox", "Use compact forecast window", new BMessage(kCompactCheckboxMessage));

	fShowFeelsLikeBox = new BCheckBox("ShowFeelsLikeBox", "Show \"Feels Like\" temperature in the Deskbar", new BMessage(kShowFeelsLikeCheckboxMessage));

	BButton* closeButton = new BButton("CloseButton", "Close", new BMessage(B_QUIT_REQUESTED));
	closeButton->MakeDefault(true);

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.AddGrid(0.0, B_USE_HALF_ITEM_INSETS)
			.AddMenuField(fIntervalMenuField, 0, 0, B_ALIGN_RIGHT)
			.Add(fNotificationBox, 1, 1)
			.Add(fShowForecastBox, 1, 2)
			.AddTextControl(fLocationControl, 0, 3, B_ALIGN_RIGHT)
			.Add(fLocationBox, 1, 4)
			.Add(fGeoNotificationBox, 1, 5)
			.AddGroup(B_HORIZONTAL, 0.0, 0, 6, 1, 1)
				.SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE))
				.Add(unitsView)
				.AddStrut(be_control_look->DefaultLabelSpacing())
			.End()
			.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING, 1, 6, 1, 1)
				.SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP))
				.Add(fImperialButton)
				.AddStrut(be_control_look->DefaultItemSpacing())
				.Add(fMetricButton)
			.End()
			.AddMenuField(fontMenuField, 0, 7, B_ALIGN_RIGHT)
			.Add(new BButton("ResetFontButton", "Reset font to default", new BMessage(kResetFontMessage)), 1, 8)
			.Add(fCompactBox, 1, 9)
			.Add(fShowFeelsLikeBox, 1, 10)
		.End()
		.Add(new BStringView("InfoStringView", "Changing font or units may require the app to be restarted to display properly"))
		.AddGlue()
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.Add(new BButton("RevertButton", "Revert", new BMessage(kRevertButtonMessage)))
			.Add(closeButton)
		.End()
	.End();
	// clang-format on

	_InitControls();

	Lock();
	CenterOnScreen();
	Show();
	Unlock();
}


SettingsWindow::~SettingsWindow()
{
	delete fInvoker;
	delete fSettingsCache;
}


void
SettingsWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case 15:
		case 30:
		case 60:
		case 180:
		case 999999:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			fSettings->SetRefreshInterval(message->what);
			// wait to Invoke() until we close the window
			break;
		}
		case kCompactCheckboxMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fSettings->CompactForecast() != value)
				fSettings->SetCompactForecast(value);

			break;
		}
		case kShowFeelsLikeCheckboxMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fSettings->ShowFeelsLike() != value)
				fSettings->SetShowFeelsLike(value);

			BMessage copy(*fInvoker->Message());
			copy.AddBool("skiprefresh", true); // don't refresh the weather, just the UI
			fInvoker->Invoke(&copy);

			break;
		}
		case kGeoCheckboxMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			BCheckBox* useGeoCheckbox = dynamic_cast<BCheckBox*>(FindView("GeoLocationCheckBox"));
			if (useGeoCheckbox == NULL)
				return;

			BTextControl* locationControl = dynamic_cast<BTextControl*>(FindView("LocationControl"));
			if (locationControl == NULL)
				return;

			locationControl->SetEnabled(!useGeoCheckbox->Value());
			fGeoNotificationBox->SetEnabled(useGeoCheckbox->Value());

			if (fSettings->UseGeoLocation() != useGeoCheckbox->Value()) {
				fSettings->SetUseGeoLocation(useGeoCheckbox->Value());
				fInvoker->Invoke();
			}
			break;
		}
		case kGeoNotificationCheckboxMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fSettings->UseGeoNotification() != value)
				fSettings->SetUseGeoNotification(value);

			break;
		}
		case kNotificationCheckboxMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fSettings->UseNotification() != value)
				fSettings->SetUseNotification(value);

			fShowForecastBox->SetEnabled(fSettings->UseNotification());

			break;
		}
		case kShowForecastCheckboxMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			int32 value = message->GetInt32("be:value", -1);
			if (value == -1)
				break;

			if (fSettings->NotificationClick() != value)
				fSettings->SetNotificationClick(value);

			break;
		}
		case kImperialMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			if (!fSettings->ImperialUnits()) {
				fSettings->SetImperialUnits(true);
				fInvoker->Invoke();
			}
			break;
		}
		case kMetricMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			if (fSettings->ImperialUnits()) {
				fSettings->SetImperialUnits(false);
				fInvoker->Invoke();
			}
			break;
		}
		case kFontMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			if (_HandleFontChange(message) != B_OK)
				(new BAlert("Error", "Font Error!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			break;
		}
		case kResetFontMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			fSettings->ResetFont();
			if (_ResetFontMenu() != B_OK)
				(new BAlert("Error", "Font Error!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			break;
		}
		case kRevertButtonMessage:
		{
			AutoLocker<WeatherSettings> slocker(fSettings);
			_RevertSettings();
			_InitControls();
			break;
		}
		default:
			BWindow::MessageReceived(message);
	}
}


bool
SettingsWindow::QuitRequested()
{
	AutoLocker<WeatherSettings> slocker(fSettings);
	_SaveSettings();
	slocker.Unlock();

	return BWindow::QuitRequested();
}


void
SettingsWindow::_RevertSettings()
{
	bool needRefresh = false;

	if (fSettings->ImperialUnits() != fSettingsCache->ImperialUnits()) {
		fSettings->SetImperialUnits(fSettingsCache->ImperialUnits());
		needRefresh = true;
	}

	if (fSettings->UseGeoLocation() != fSettingsCache->UseGeoLocation()) {
		fSettings->SetUseGeoLocation(fSettingsCache->UseGeoLocation());
		needRefresh = true;
	}

	if (strcmp(fSettings->Location(), fSettingsCache->Location()) != 0) {
		fSettings->SetLocation(fSettingsCache->Location());
		needRefresh = true;
	}

	if (fSettings->RefreshInterval() != fSettingsCache->RefreshInterval()) {
		fSettings->SetRefreshInterval(fSettingsCache->RefreshInterval());
		needRefresh = true;
	}

	if (fSettings->ShowFeelsLike() != fSettingsCache->ShowFeelsLike()) {
		fSettings->SetShowFeelsLike(fSettingsCache->ShowFeelsLike());
		needRefresh = true;
	}

	// no need to refresh immediately for these
	fSettings->SetUseNotification(fSettingsCache->UseNotification());
	fSettings->SetNotificationClick(fSettingsCache->NotificationClick());
	fSettings->SetCompactForecast(fSettingsCache->CompactForecast());

	BFont font;
	if (fSettingsCache->GetFont(font) == B_OK)
		fSettings->SetFont(font);
	// _InitControls() will be called after this and will handle the font refresh message

	if (needRefresh)
		fInvoker->Invoke();
}


void
SettingsWindow::_InitControls()
{
	if (fSettings->ImperialUnits())
		fImperialButton->SetValue(1);
	else
		fMetricButton->SetValue(1);

	fGeoNotificationBox->SetEnabled(fSettings->UseGeoLocation());
	fGeoNotificationBox->SetValue(fSettings->UseGeoNotification());

	fLocationBox->SetValue(fSettings->UseGeoLocation());

	fLocationControl->SetEnabled(!fSettings->UseGeoLocation());
	fLocationControl->SetText(fSettings->Location());

	fNotificationBox->SetValue(fSettings->UseNotification());

	fShowForecastBox->SetValue(fSettings->NotificationClick());

	fShowForecastBox->SetEnabled(fSettings->UseNotification());
	fShowForecastBox->SetValue(fSettings->NotificationClick());

	fCompactBox->SetValue(fSettings->CompactForecast());

	fShowFeelsLikeBox->SetValue(fSettings->ShowFeelsLike());

	BMenu* intervalMenu = fIntervalMenuField->Menu();
	BMenuItem* selectedItem = intervalMenu->FindItem(fSettings->RefreshInterval());
	if (selectedItem != NULL)
		selectedItem->SetMarked(true);

	_ResetFontMenu();
}


void
SettingsWindow::_SaveSettings()
{
	bool needRefresh = false;

	//TODO a geolocation request might have changed our location name while the window was open (on first run)
	//this should be enabled/fixed whenever manual location selection is implemented
//	if (strcmp(fSettings->Location(), fLocationControl->Text()) != 0) {
//		fSettings->SetLocation(fLocationControl->Text());
//		needRefresh = true;
//	}

	if (fSettings->RefreshInterval() != fSettingsCache->RefreshInterval())
		needRefresh = true;

	if (needRefresh)
		fInvoker->Invoke();
}


BMenu*
SettingsWindow::_BuildFontMenu()
{
	BFont origFont;
	fSettings->GetFont(origFont);
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

			float baseSize = be_plain_font->Size();
			for (double k = baseSize - 4; k < baseSize + 4.5; k += 0.5) {
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
SettingsWindow::_ResetFontMenu()
{
	BFont font;
	fSettings->GetFont(font);

	font_family family;
	font_style style;
	font.GetFamilyAndStyle(&family, &style);

	return _UpdateFontMenu(family, style, font.Size());
}


status_t
SettingsWindow::_HandleFontChange(BMessage* message)
{
	const char* family = NULL;
	if (message->FindString("FontFamily", &family) != B_OK)
		return B_ERROR;

	const char* style = NULL;
	if (message->FindString("FontStyle", &style) != B_OK)
		return B_ERROR;

	double size = -1.0;
	if (message->FindDouble("FontSize", &size) != B_OK)
		return B_ERROR;

	fSettings->SetFont(family, style, size);

	return _UpdateFontMenu(family, style, size);
}


status_t
SettingsWindow::_UpdateFontMenu(const char* family, const char* style, double size)
{
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

	BMessage copy(*fInvoker->Message());
	copy.AddBool("skiprefresh", true); // don't refresh the weather, just the UI
	fInvoker->Invoke(&copy);

	return B_OK;
}
