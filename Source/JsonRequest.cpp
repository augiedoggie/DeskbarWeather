// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#include "JsonRequest.h"

#include <Invoker.h>
#include <private/netservices/HttpRequest.h>
#include <private/netservices/UrlProtocolRoster.h>
#include <private/shared/Json.h>


JsonRequestListener::JsonRequestListener(BInvoker* invoker)
	:
	fInvoker(invoker)
{}


JsonRequestListener::~JsonRequestListener() {}


void
JsonRequestListener::RequestCompleted(BUrlRequest* caller, bool /*success*/)
{
	if (fInvoker == NULL)
		return;

	BMessage replyCopy(*fInvoker->Message());

	const BHttpResult& result = dynamic_cast<const BHttpResult&>(caller->Result());
	replyCopy.AddInt32("re:code", result.StatusCode());
	replyCopy.AddString("re:message", result.StatusText());

	if (BHttpRequest::IsSuccessStatusCode(result.StatusCode())) {
		BMallocIO* data = dynamic_cast<BMallocIO*>(caller->Output());
		if (data == NULL)
			return; // TODO reset re:code and re:message ?

		BPrivate::BJson::Parse(static_cast<const char*>(data->Buffer()), replyCopy);
		data->Seek(0, SEEK_SET);
	}

	replyCopy.what = fInvoker->Message()->what; // reset ->what after BJson::Parse() messed with it
	fInvoker->Invoke(&replyCopy);
}
