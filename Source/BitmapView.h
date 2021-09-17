// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _BITMAPVIEW_H_
#define _BITMAPVIEW_H_

#include <View.h>


class BBitmap;


class BitmapView : public BView {
public:
					BitmapView(const char* name, BBitmap* bitmap);
					~BitmapView();

	virtual void	Draw(BRect updateRect);
	virtual void	GetPreferredSize(float* width, float* height);

private:
	BBitmap*		fBitmap;
};

#endif	// _BITMAPVIEW_H_
