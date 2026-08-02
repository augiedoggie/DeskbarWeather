// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Chris Roberts

#include "BitmapView.h"
#include "Condition.h"
#include "DeskbarWeatherView.h"
#include "OpenMeteo.h"
#include "WeatherSettings.h"
#include "WeatherToolTip.h"

#include <Font.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <StringView.h>


WeatherToolTip::WeatherToolTip(WeatherSettings* settings, OpenMeteo* weather)
	: BToolTip()
{
	fView = new BView(BRect(0, 0, 600, 300), "WeatherToolTipView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fView->SetViewUIColor(B_TOOL_TIP_BACKGROUND_COLOR);

	BFont bigFont(be_bold_font);
	bigFont.SetSize(bigFont.Size() * 1.6);

	BFont smallFont(be_plain_font);
	smallFont.SetSize(smallFont.Size() * 0.8);

	BitmapView* conditionView = new BitmapView("ConditionBitmap", DeskbarWeatherView::LoadResourceBitmap(weather->Current()->Icon()->String(), 48));
	conditionView->SetViewUIColor(B_TOOL_TIP_BACKGROUND_COLOR);

	BString tempString;
	tempString.SetToFormat("%.1f°", weather->Current()->Temp());

	BString feelsLikeString;
	feelsLikeString.SetToFormat("%.1f°", weather->Current()->Temp(true));

	BString highLowString;
	highLowString << weather->Current()->iHigh() << "° / " << weather->Current()->iLow() << "°";

	BString currentWindString;
	currentWindString.SetToFormat("%.1f %s", weather->Current()->Wind(), (weather->IsImperial() ? " mph" : " kmh"));

	BString updateTimeString;
	weather->LastUpdate(updateTimeString);

	BString updateString("Updated: ");
	updateString << updateTimeString;

	BView* separatorView = new BView("SeparatorView", B_WILL_DRAW);
	separatorView->SetExplicitSize(BSize(0, B_SIZE_UNSET));
	separatorView->SetViewUIColor(B_TOOL_TIP_BACKGROUND_COLOR, B_DARKEN_1_TINT);

	// clang-format off
	BLayoutBuilder::Group<>(fView, B_VERTICAL, 0)
		.SetInsets(0)
		.Add(_BuildStringView("LocationString", settings->Location(), B_ALIGN_CENTER, const_cast<BFont*>(be_bold_font)))
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.AddGlue()
			.AddGroup(B_VERTICAL, 0)
				.AddGlue()
				.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
					.AddGlue()
					.Add(conditionView)
					.AddGlue()
				.End()
				.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
					.AddGlue()
					.Add(_BuildStringView("CurrentConditionString", weather->Current()->Forecast()->String(), B_ALIGN_CENTER, const_cast<BFont*>(be_bold_font)))
					.AddGlue()
				.End()
				.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
					.AddGlue()
					.Add(_BuildStringView("HighLowString", highLowString, B_ALIGN_CENTER, const_cast<BFont*>(be_bold_font)))
					.AddGlue()
				.End()
				.AddGlue()
			.End()
			.AddGlue()
			.AddGroup(B_VERTICAL, 0)
				.Add(_BuildStringView("CurrentLabelString", "Current", B_ALIGN_CENTER, const_cast<BFont*>(be_plain_font)))
				.Add(_BuildStringView("CurrentString", tempString, B_ALIGN_CENTER, &bigFont))
				.AddStrut(2.0f * be_plain_font->Size() / 12.0f)
				.AddGlue(2.0f)
				.Add(_BuildStringView("FeelsLikeLabelString", "Feels Like", B_ALIGN_CENTER, const_cast<BFont*>(be_plain_font)))
				.Add(_BuildStringView("FeelsLikeString", feelsLikeString, B_ALIGN_CENTER, &bigFont))
			.End()
			.AddGlue()
			.Add(separatorView)
			.AddGlue()
			.AddGroup(B_VERTICAL, 0)
				.Add(_BuildStringView("HumidityLabelString", "Humidity", B_ALIGN_CENTER, const_cast<BFont*>(be_plain_font)))
				.Add(_BuildStringView("HumidityString", weather->Current()->Humidity()->String(), B_ALIGN_CENTER, const_cast<BFont*>(be_bold_font)))
				.AddGlue()
				.Add(_BuildStringView("WindLabelString", "Wind Speed", B_ALIGN_CENTER, const_cast<BFont*>(be_plain_font)))
				.Add(_BuildStringView("WindString", currentWindString, B_ALIGN_CENTER, const_cast<BFont*>(be_bold_font)))
			.End()
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.AddGlue()
			.Add(_BuildStringView("LastUpdateString", updateString, B_ALIGN_CENTER, &smallFont))
			.AddGlue()
		.End()
		.AddGlue();
	// clang-format on

}


BView*
WeatherToolTip::View() const
{
	return fView;
}


BStringView*
WeatherToolTip::_BuildStringView(const char* name, const char* label, alignment align, BFont* font)
{
	BStringView* stringView = new BStringView(name, label);
	stringView->SetAlignment(align);
	stringView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
	if (font != NULL)
		stringView->SetFont(font);

	return stringView;
}
