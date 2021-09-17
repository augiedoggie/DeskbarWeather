// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "BitmapView.h"

#include <Bitmap.h>


BitmapView::BitmapView(const char* name, BBitmap* bitmap)
	:	BView(name, B_WILL_DRAW),
		fBitmap(bitmap)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetDrawingMode(B_OP_ALPHA);
}


void
BitmapView::Draw(BRect updateRect)
{
	BView::Draw(updateRect);

	if (fBitmap != NULL)
		DrawBitmap(fBitmap);
}


void
BitmapView::GetPreferredSize(float* width, float* height)
{
	if (fBitmap == NULL) {
		if (width != NULL)
			*width = 0;
		if (height != NULL)
			*height = 0;
		return;
	}

	if (width != NULL)
		*width = fBitmap->Bounds().Width();
	if (height != NULL)
		*height = fBitmap->Bounds().Height();
}
