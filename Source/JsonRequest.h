// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2021 Chris Roberts

#ifndef _JSONREQUEST_H_
#define _JSONREQUEST_H_

#include <private/netservices/UrlProtocolListener.h>

namespace BPrivate
{
namespace Network
{
	class BUrlRequest;
}
} // namespace BPrivate

class BInvoker;
class BMallocIO;

using namespace BPrivate::Network;


class JsonRequestListener : public BUrlProtocolListener {
public:
						JsonRequestListener(BInvoker* invoker);
	virtual				~JsonRequestListener();
	virtual	void		RequestCompleted(BUrlRequest *caller, bool success);
private:
			BInvoker*	fInvoker;
};

#endif // _JSONREQUEST_H_
