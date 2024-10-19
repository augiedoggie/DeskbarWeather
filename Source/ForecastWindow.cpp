// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "ForecastWindow.h"
#include "BitmapView.h"
#include "Condition.h"
#include "DeskbarWeatherView.h"
#include "OpenMeteo.h"

#include <Bitmap.h>
#include <Box.h>
#include <DateTimeFormat.h>
#include <File.h>
#include <IconUtils.h>
#include <LayoutBuilder.h>
#include <Resources.h>
#include <StringView.h>


ForecastWindow::ForecastWindow(OpenMeteo* weather, BRect frame, const char* location, bool compact)
	:
	BWindow(frame, location, B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
	BFont bigFont(be_bold_font);
	bigFont.SetSize(bigFont.Size() + (compact ? 2 : 4));

	BFont bigPlainFont(be_plain_font);
	bigPlainFont.SetSize(bigPlainFont.Size() + (compact ? 2 : 4));

	BString windowTitle("Weather Conditions & Forecast for ");
	windowTitle << location;
	SetTitle(windowTitle);

	BString currentString;
	currentString.SetToFormat("%.1f°", weather->Current()->Temp());

	BString currentFeelString;
	currentFeelString.SetToFormat("%.1f°", weather->Current()->Temp(true));

	BString currentLowString;
	currentLowString << weather->Current()->iLow() << "°";

	BString currentHighString;
	currentHighString << weather->Current()->iHigh() << "°";

	BString currentWindString;
	currentWindString.SetToFormat("%.1f %s", weather->Current()->Wind(), (weather->IsImperial() ? " mph" : " kmh"));

	BString currentDirectionString;
	BString directionSymbolString;
	double direction = weather->Current()->WindDirection();
	if (direction > 337.5 || direction <= 22.5) {
		// north
		directionSymbolString = "\u2191";
	} else if (direction > 22.5 && direction <= 67.5) {
		// north-east
		directionSymbolString = "\u2197";
	} else if (direction > 67.5 && direction <= 112.5) {
		// east
		directionSymbolString = "\u2192";
	} else if (direction > 112.5 && direction <= 157.5) {
		// south-east
		directionSymbolString = "\u2198";
	} else if (direction > 157.5 && direction <= 202.5) {
		// south
		directionSymbolString = "\u2193";
	} else if (direction > 202.5 && direction <= 247.5) {
		// south-west
		directionSymbolString = "\u2199";
	} else if (direction > 247.5 && direction <= 292.5) {
		// west
		directionSymbolString = "\u2190";
	} else if (direction > 292.5 && direction <= 337.5) {
		// north-west
		directionSymbolString = "\u2196";
	}
	currentDirectionString.SetToFormat("%s %.0f°", directionSymbolString.String(), direction);

	BString currentCloudString;
	currentCloudString.SetToFormat("%.0f%%", weather->Current()->CloudCover());

	BGridLayout* tempGrid;
	BGridLayout* otherGrid;

	BGroupView* currentView = new BGroupView(B_HORIZONTAL, compact ? 0 : B_USE_BIG_SPACING);
	// clang-format off
	BLayoutBuilder::Group<>(currentView, B_HORIZONTAL, compact ? 0 : B_USE_BIG_SPACING)
		.SetInsets(compact ? 0 : B_USE_BIG_INSETS)
		.AddGlue()
		.AddGroup(B_VERTICAL, compact ? 0 : B_USE_DEFAULT_SPACING)
			.AddGlue()
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(_BuildStringView("LocationString", location, B_ALIGN_CENTER, &bigFont))
				.AddGlue()
			.End()
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(new BitmapView("ConditionBitmap", DeskbarWeatherView::LoadResourceBitmap(weather->Current()->Icon()->String(), compact ? 48 : 64)), 0)
				.AddGlue()
			.End()
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(_BuildStringView("CurrentConditionString", weather->Current()->Forecast()->String(), B_ALIGN_CENTER, &bigFont))
				.AddGlue()
			.End()
			.AddGlue()
		.End()
		.AddGlue()
		.Add(tempGrid = BLayoutBuilder::Grid<>(B_USE_HALF_ITEM_SPACING, compact ? 0 : B_USE_BIG_SPACING)
			.Add(_BuildStringView("CurrentLabel", "Current:", B_ALIGN_RIGHT, &bigFont), 0, 0)
			.Add(_BuildStringView("CurrentString", currentString.String(), B_ALIGN_LEFT, &bigFont), 1, 0)
			.Add(_BuildStringView("CurrentFeelLabel", "Feels Like:", B_ALIGN_RIGHT, &bigPlainFont), 0, 1)
			.Add(_BuildStringView("CurrentFeelString", currentFeelString.String(), B_ALIGN_LEFT, &bigPlainFont), 1, 1)
			.Add(_BuildStringView("HighLabel", "High:", B_ALIGN_RIGHT, &bigPlainFont), 0, 2)
			.Add(_BuildStringView("HighString", currentHighString.String(), B_ALIGN_LEFT, &bigPlainFont), 1, 2)
			.Add(_BuildStringView("LowLabel", "Low:", B_ALIGN_RIGHT, &bigPlainFont), 0, 3)
			.Add(_BuildStringView("LowLabel", currentLowString.String(), B_ALIGN_LEFT, &bigPlainFont), 1, 3)
			.SetColumnWeight(0, 0)
		)
		.AddGlue()
		.Add(otherGrid = BLayoutBuilder::Grid<>(B_USE_HALF_ITEM_SPACING, compact ? 0 : B_USE_BIG_SPACING)
			.Add(_BuildStringView("HumidityLabel", "Humidity:", B_ALIGN_RIGHT, &bigPlainFont), 0, 0)
			.Add(_BuildStringView("HumidityString", weather->Current()->Humidity()->String(), B_ALIGN_LEFT, &bigPlainFont), 1, 0)
			.Add(_BuildStringView("WindLabel", "Wind Speed:", B_ALIGN_RIGHT, &bigPlainFont), 0, 1)
			.Add(_BuildStringView("WindString", currentWindString, B_ALIGN_LEFT, &bigPlainFont), 1, 1)
			.Add(_BuildStringView("DirectionLabel", "Wind Direction:", B_ALIGN_RIGHT, &bigPlainFont), 0, 2)
			.Add(_BuildStringView("DirectionString", currentDirectionString, B_ALIGN_LEFT, &bigPlainFont), 1, 2)
			.Add(_BuildStringView("CloudLabel", "Cloud Cover:", B_ALIGN_RIGHT, &bigPlainFont), 0, 3)
			.Add(_BuildStringView("CloudString", currentCloudString, B_ALIGN_LEFT, &bigPlainFont), 1, 3)
			.SetColumnWeight(0, 0)
		)
		.AddGlue();
	// clang-format on

	otherGrid->AlignLayoutWith(tempGrid, B_VERTICAL);

	BGroupView* forecastView = new BGroupView(B_HORIZONTAL, compact ? 0 : B_USE_DEFAULT_SPACING);
	BLayoutBuilder::Group<> forecastBuilder = BLayoutBuilder::Group<>(forecastView);

	BDateTimeFormat format;
	format.SetDateTimeFormat(B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT, B_DATE_ELEMENT_WEEKDAY | B_DATE_ELEMENT_MONTH | B_DATE_ELEMENT_DAY);

	BObjectList<Condition>* forecastList = weather->Forecast();
	for (int32 i = 0; i < forecastList->CountItems(); i++) {
		Condition* condition = forecastList->ItemAt(i);

		BString dayString;
		format.Format(dayString, condition->Day(), B_SHORT_DATE_FORMAT, B_SHORT_TIME_FORMAT);

		BString lowString;
		lowString << condition->iLow() << "°";

		BString highString;
		highString << condition->iHigh() << "°";

		// clang-format off
		forecastBuilder
			.AddGroup(B_VERTICAL, compact ? B_USE_SMALL_SPACING : B_USE_DEFAULT_SPACING)
				.AddGroup(B_HORIZONTAL, compact ? B_USE_SMALL_SPACING : B_USE_DEFAULT_SPACING)
					.AddGlue()
					.Add(new BStringView("DateString", dayString))
					.AddGlue()
				.End()
				.AddGroup(B_HORIZONTAL, compact ? B_USE_SMALL_SPACING : B_USE_DEFAULT_SPACING)
					.AddGlue()
					.Add(new BitmapView("ConditionBitmap", DeskbarWeatherView::LoadResourceBitmap(condition->Icon()->String(), compact ? 36 : 48)), 0)
					.AddGlue()
				.End()
				.AddGroup(B_HORIZONTAL, compact ? B_USE_SMALL_SPACING : B_USE_DEFAULT_SPACING)
					.AddGlue()
					.Add(new BStringView("ConditionString", condition->Forecast()->String()))
					.AddGlue()
				.End()
				.AddGroup(B_HORIZONTAL, compact ? B_USE_SMALL_SPACING : B_USE_DEFAULT_SPACING)
					.AddGlue()
					.AddGrid(B_USE_HALF_ITEM_SPACING, 0.0)
						.Add(_BuildStringView("HighLabel", "High:", B_ALIGN_RIGHT), 0, 0)
						.Add(_BuildStringView("HighString", highString.String(), B_ALIGN_LEFT), 1, 0)
						.Add(_BuildStringView("LowLabel", "Low:", B_ALIGN_RIGHT), 0, 1)
						.Add(_BuildStringView("LowString", lowString.String(), B_ALIGN_LEFT), 1, 1)
						.SetColumnWeight(0, 0)
					.End()
					.AddGlue()
				.End()
			.End();
		// clang-format on

		if (forecastList->ItemAt(i + 1) != NULL) {
			BView* separatorView = new BView("SeparatorView", B_WILL_DRAW);
			separatorView->SetExplicitSize(BSize(0, B_SIZE_UNSET));
			separatorView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR, B_DARKEN_1_TINT);
			forecastBuilder.Add(separatorView);
		}
	}
	forecastBuilder.SetInsets(compact ? 1 : B_USE_ITEM_INSETS);

	BBox* currentBox = new BBox("CurrentBBox");
	BString boxLabel("Current conditions updated: ");
	BString dateStr;
	weather->LastUpdate(dateStr, true);
	boxLabel << dateStr;
	currentBox->SetLabel(boxLabel.String());
	currentBox->AddChild(currentView);

	BBox* forecastBox = new BBox("ForecastBBox");
	forecastBox->SetLabel("Forecast");
	forecastBox->AddChild(forecastView);

	BLayoutBuilder::Group<>(this, B_VERTICAL, compact ? 0 : B_USE_SMALL_SPACING)
		.Add(currentBox)
		.Add(forecastBox)
		.SetInsets(compact ? 1 : B_USE_HALF_ITEM_INSETS);

	AddShortcut('W', B_COMMAND_KEY, new BMessage(B_QUIT_REQUESTED));

	Lock();
	CenterOnScreen();
	Show();
	Unlock();
}


BStringView*
ForecastWindow::_BuildStringView(const char* name, const char* label, alignment align, BFont* font)
{
	BStringView* stringView = new BStringView(name, label);
	stringView->SetAlignment(align);
	stringView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	if (font != NULL)
		stringView->SetFont(font);

	return stringView;
}
