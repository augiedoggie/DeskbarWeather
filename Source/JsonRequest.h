// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _JSONREQUEST_H_
#define _JSONREQUEST_H_

#include <SupportDefs.h>
#include <private/netservices/UrlProtocolListener.h>

namespace BPrivate {
	namespace Network {
		class BUrlRequest;
	}
}
class BInvoker;
class BMallocIO;
class BMessage;
class BUrl;

using namespace BPrivate::Network;


class JsonRequestListener : public BUrlProtocolListener {
public:
						JsonRequestListener(BInvoker* invoker);
	virtual	void		RequestCompleted(BUrlRequest *caller, bool success);
private:
			BInvoker*	fInvoker;
};


class JsonRequest {
public:
	virtual				~JsonRequest();
	static	status_t	Run(BUrl& url, BMessage& reply);
private:
						JsonRequest(BUrl& url, BMessage& reply);
			status_t	_Run();

		BUrlRequest*	fUrlRequest;
		BMessage*		fReplyMessage;
		BMallocIO*		fReplyData;
};


#endif	// _JSONREQUEST_H_
