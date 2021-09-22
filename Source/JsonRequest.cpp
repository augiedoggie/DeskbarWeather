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


void
JsonRequestListener::RequestCompleted(BUrlRequest *caller, bool /*success*/)
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
		delete data;
	}

	replyCopy.what = fInvoker->Message()->what; // reset ->what after BJson::Parse() messed with it
	fInvoker->Invoke(&replyCopy);
}


JsonRequest::JsonRequest(BUrl& url, BMessage& message)
	:
	fUrlRequest(NULL),
	fReplyMessage(&message),
	fReplyData(new BMallocIO())
{
	fUrlRequest = BUrlProtocolRoster::MakeRequest(url, fReplyData);
}


JsonRequest::~JsonRequest()
{
	delete fUrlRequest;
	delete fReplyData;
}


status_t
JsonRequest::_Run()
{
	if (fUrlRequest == NULL) return B_ERROR;

	thread_id rThread = fUrlRequest->Run();
	//TODO change style to invoker to avoid waiting
	wait_for_thread(rThread, NULL);

	const BHttpResult& result = dynamic_cast<const BHttpResult&>(fUrlRequest->Result());

	if (!BHttpRequest::IsSuccessStatusCode(result.StatusCode()))
		return B_ERROR;

	return BPrivate::BJson::Parse(static_cast<const char*>(fReplyData->Buffer()), *fReplyMessage);
}


status_t
JsonRequest::Run(BUrl& url, BMessage& reply)
{
	JsonRequest request(url, reply);
	return request._Run();
}
