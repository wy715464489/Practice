//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ps4 HttpClient class
// 
//*****************************************************************************

#include <string>

#include <libhttp.h>
#include <libssl.h>
#include <libnetctl.h>

#include "VuEngine/Prefix/Vu.h"
#include "VuEngine/Prefix/VuPs4.h"
#include "VuEngine/HAL/Sys/Ps4/VuPs4Sys.h"
#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Libs/http-parser/http_parser.h"
#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Managers/VuTickManager.h"

#include "VuEngine/Net/Ps4/VuPs4HttpClient.h"

IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuPs4HttpClient);

#define HTTP_USER_AGENT					"RGP2/1.0"

//*****************************************************************************
bool VuPs4HttpClient::init()
{
	VuTickManager::IF()->registerHandler(this, &VuPs4HttpClient::tickNetwork, "Network");

	// TEST CODE
#if 0
	{
		std::string url = "http://requestb.in/1gww1kz1";
		VUHANDLE getRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->getAsync(getRequest, url.c_str());

		url = "http://www.hackaday.com";
		VUHANDLE anotherRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->getAsync(anotherRequest, url.c_str());

		url = "https://httpbin.org/get";
		anotherRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->getAsync(anotherRequest, url.c_str());

		url = "https://httpbin.org/gzip";
		anotherRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->getAsync(anotherRequest, url.c_str());

		// create http POST request
		url = "http://requestb.in/1gww1kz1";
		std::string strData = "Hello, this is some data";
		VUHANDLE postRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->setContentHeader(postRequest, "Content-Type", "application/json");
		VuHttpClient::IF()->setContentHeader(postRequest, "Content-Length", strData.size());
		VuHttpClient::IF()->postAsync(postRequest, url.c_str(), strData);
	}
#endif
	return true;
}

void VuPs4HttpClient::requestCompleted(VuPs4HttpRequest* pRequest)
{
	// Remove it from the list
	mRequests.remove(pRequest);

	if (pRequest->mpData)
	{
		delete[] pRequest->mpData;
		pRequest->mpData = VUNULL;
		pRequest->mDataSize = 0;
	}

	int error = sceHttpDeleteRequest(pRequest->mRequestId);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDeleteRequest() error: 0x%08X\n", error);
	}
	pRequest->mRequestId = 0;

	error = sceHttpDestroyEpoll(VuPs4Sys::getHttpContextID(), pRequest->mPollHandle);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDestroyEpoll() error: 0x%08X\n", error);
	}

	error = sceHttpDeleteConnection(pRequest->mConnectionId);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDeleteConnection() error: 0x%08X\n", error);
	}
	pRequest->mConnectionId = 0;

	error = sceHttpDeleteTemplate(pRequest->mTemplateId);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDeleteTemplate() error: 0x%08X\n", error);
	}
	pRequest->mTemplateId = 0;
}

void VuPs4HttpClient::tickNetwork(float fdt)
{
#ifdef TORTURE_TEST
	while (mRequests.size() < 10)
	{
		std::string url = "https://httpbin.org/get";
		VUHANDLE anotherRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->getAsync(anotherRequest, url.c_str());
	}
#endif
	int error = -1;
	VuPs4HttpRequest *pRequest = VUNULL;
	VUINT statusCode = 0;
	SceHttpNBEvent nonBlockingEvent;

	memset(&nonBlockingEvent, 0x00, sizeof(nonBlockingEvent));

	// Use a COPY of our request list so we can freely remove list elements from the real deal while we're
	// iterating
	Requests listCopy = mRequests;

	// Service each request as necessary
	for (Requests::iterator requestIterator = listCopy.begin(); requestIterator != listCopy.end(); requestIterator++)
	{
		pRequest = (*requestIterator);
		VUASSERT(pRequest, "NULL VuPs4HttpRequest found in mRequests list.");
		if (!pRequest)
		{
			continue;
		}

		// If the user doesn't have network permission, then just auto-fail any requests before we try to send them
		if (VuPs4Sys::getNetworkRestricted())
		{
			requestCompleted(pRequest);
			pRequest->mStatus = VuHttpClient::STATUS_ERROR;
			pRequest->mState = STATE_DONE;
			pRequest->removeRef();
			continue;
		}

		VUINT offline = false;
		VUINT netState;
		VUINT result = sceNetCtlGetState(&netState);
		if (result == SCE_OK)
		{
			if (netState == SCE_NET_CTL_STATE_DISCONNECTED)
			{
				offline = true;
			}
		}
		else
		{
			VUPRINTF("ERROR: sceNetCtlGetState() returned Error=%#.8x\n", result);
		}

		// Check what state each request is currently in
		switch (pRequest->mState)
		{
		case STATE_SEND:
			error = sceHttpSendRequest(pRequest->mRequestId, pRequest->mpData, pRequest->mDataSize);
			if (error < SCE_OK)
			{
				if (!offline && (error == SCE_HTTP_ERROR_EAGAIN || error == SCE_HTTP_ERROR_BUSY))
				{
					// We need to retry this next tickNework(), it's blocked and can't send right now
					break;
				}
				else
				{
					// An error was posted, return it and then skip this Request 
					// TODO: remove from list at this point?
					VUPRINTF("sceHttpSendRequest() error: 0x%08X - '%s'\n", error, pRequest->mURL.c_str());

					requestCompleted(pRequest);
					pRequest->mStatus = VuHttpClient::STATUS_ERROR;
					pRequest->mState = STATE_DONE;
					pRequest->removeRef();
					continue;
				}
			}

			// Send was successful, move to the next state, the getting of the status
			pRequest->mState = STATE_GET_STATUS;
			pRequest->mStatus = VuHttpClient::STATUS_WAITING_FOR_RESPONSE;
			// 'break' is omitted here for fall-through from STATE_SEND to STATE_GET_STATUS
		case STATE_GET_STATUS:
			error = sceHttpGetStatusCode(pRequest->mRequestId, &statusCode);
			if (error < SCE_OK)
			{
				if (error == SCE_HTTP_ERROR_EAGAIN)
				{
					// Breaks switch statement and executes just below it
					break;
				}
				else
				{
					printf("sceHttpGetStatusCode() error: 0x%08X\n", error);

					requestCompleted(pRequest);
					pRequest->mStatus = VuHttpClient::STATUS_ERROR;
					pRequest->mState = STATE_DONE;
					pRequest->removeRef();

					// Continues loop with next pRequest
					continue;
				}
			}
			VUPRINTF("HTTP Response Code=%d - '%s'\n", statusCode, pRequest->mURL.c_str());
			pRequest->mState = STATE_GET_LENGTH;
			// 'break' is omitted here for fall-through from STATE_GET_STATUS to STATE_GET_LENGTH
		case STATE_GET_LENGTH:
		{
			// Get the length of the response if we can
			VUINT		responseType = -1;
			uint64_t	responseLength = -1;
			error = sceHttpGetResponseContentLength(pRequest->mRequestId, &responseType, &responseLength);
			if (error < SCE_OK)
			{
				// not ready yet, try again next tick
				if (error == SCE_HTTP_ERROR_EAGAIN)
				{
					// Breaks out of switch statement and executes just after it
					break;
				}

				printf("sceHttpGetContentLength() error: 0x%08X\n", error);

				requestCompleted(pRequest);
				pRequest->mStatus = VuHttpClient::STATUS_ERROR;
				pRequest->mState = STATE_DONE;
				pRequest->removeRef();

				// Continue with next pRequest in the list
				continue;
			}

			// See if a length exists for us
			if (responseType == SCE_HTTP_CONTENTLEN_EXIST)
			{
				// Got a valid length, use that rather than the default if it fits
				if (responseLength < HTTP_RECEIVE_BUFFER_SIZE)
				{
					pRequest->mResponseLength = responseLength;
				}
			}

			// Time to receive data
			pRequest->mState = STATE_RECEIVE;
		}
		// 'break' is omitted here for fall-through from STATE_GET_LENGTH to STATE_RECEIVE
		case STATE_RECEIVE:
		{
			// Use this temporary buffer to receive data
			VUUINT8 receiveBuffer[HTTP_RECEIVE_BUFFER_SIZE + 1];	// +1 for the terminating zero

			// Clear it for zero termination easiness
			memset(receiveBuffer, 0, sizeof(receiveBuffer));

			// Read data into the buffer
			error = sceHttpReadData(pRequest->mRequestId, receiveBuffer, pRequest->mResponseLength);
			if (error == 0)
			{
				// We've received everything, we're done. Wahoo.
				requestCompleted(pRequest);

				// Update state
				pRequest->mStatus = VuHttpClient::STATUS_RESPONSE_RECEIVED;
				pRequest->mState = STATE_DONE;
				pRequest->removeRef();

				// This request is finished, so continue with next pRequest
				continue;
			}
			else if (error < SCE_OK)
			{
				if (error == SCE_HTTP_ERROR_EAGAIN)
				{
					// We're not ready to read data yet, try again next tick
					break;
				}
				else
				{

					VUPRINTF("sceHttpReadData() error: 0x%08X\n", error);

					requestCompleted(pRequest);
					pRequest->mStatus = VuHttpClient::STATUS_ERROR;
					pRequest->mState = STATE_DONE;
					pRequest->removeRef();
					continue;
				}
			}
			else
			{
				// we received a positive value from sceHttpReadData, that means
				// something was read. Append it to our response buffer as we go
				pRequest->mResponse += (char *)receiveBuffer;
			}

			// 
			continue;
		}
		default:
		{
			// ERROR!
			requestCompleted(pRequest);
			pRequest->mStatus = VuHttpClient::STATUS_ERROR;
			pRequest->mState = STATE_DONE;
			pRequest->removeRef();
			continue;
		}
		}

		// Now pump the http library for non-blocking events and handle the pernicious ones
		// See what our PollHandle tells us we need to do
		error = sceHttpWaitRequest(pRequest->mPollHandle, &nonBlockingEvent, 1, 1000ULL);

		if (error > 0		// returns the number of pending events to look at
			&&
			(VUINT64)nonBlockingEvent.id == (VUINT64)pRequest->mPollHandle) // it's for THIS request?
		{
			// Handle aborted non-blocking events
			if (nonBlockingEvent.events &
				(SCE_HTTP_NB_EVENT_SOCK_ERR | SCE_HTTP_NB_EVENT_HUP | SCE_HTTP_NB_EVENT_RESOLVER_ERR))
			{
				VUPRINTF("Error or terminated request %ld\n", pRequest->mRequestId);

				// Remove from list and deallocate system resources
				requestCompleted(pRequest);

				pRequest->mStatus = VuHttpClient::STATUS_ERROR;
				pRequest->mState = STATE_DONE;
				pRequest->removeRef();

				continue;
			}
		}

		if (error < SCE_OK)
		{
			VUPRINTF("sceHttpWaitRequest() error: 0x%08X\n", error);

			requestCompleted(pRequest);

			pRequest->mStatus = VuHttpClient::STATUS_ERROR;
			pRequest->mState = STATE_DONE;
			pRequest->removeRef();

			continue;
		}
	}
}

//*****************************************************************************
VUHANDLE VuPs4HttpClient::createRequest()
{
	VuPs4HttpRequest* pRequest = new VuPs4HttpRequest();

	return pRequest;
}

//*****************************************************************************
void VuPs4HttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuPs4HttpRequest* pRequest = static_cast<VuPs4HttpRequest*>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuPs4HttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuPs4HttpRequest* pRequest = static_cast<VuPs4HttpRequest*>(hRequest);
	VUASSERT(pRequest, "Invalid Request Handle");
	if (!pRequest)
	{
		return;
	}

	std::pair<std::string, std::string> pair;

	pair.first = key;
	pair.second = value;

	pRequest->mHeaderTuples.push_back(pair);
}

//*****************************************************************************
void VuPs4HttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuPs4HttpRequest* pRequest = static_cast<VuPs4HttpRequest*>(hRequest);
	VUASSERT(pRequest, "Invalid Request Handle");
	if (!pRequest)
	{
		return;
	}

	pRequest->mRequestTimeout = timeoutMS;
}

//*****************************************************************************
VuHttpClient::eStatus VuPs4HttpClient::getStatus(VUHANDLE hRequest)
{
	VuPs4HttpRequest* pRequest = static_cast<VuPs4HttpRequest*>(hRequest);
	VUASSERT(pRequest, "Invalid Request Handle");
	if (!pRequest)
	{
		return STATUS_ERROR;
	}

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string& VuPs4HttpClient::getResponse(VUHANDLE hRequest)
{
	VuPs4HttpRequest* pRequest = static_cast<VuPs4HttpRequest*>(hRequest);
	VUASSERT(pRequest, "Invalid Request Handle");

	return pRequest->mResponse;
}

//*****************************************************************************
void VuPs4HttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuPs4HttpRequest* pRequest = static_cast<VuPs4HttpRequest*>(hRequest);
	VUASSERT(pRequest, "Invalid Request Handle");
	if (!pRequest)
	{
		return;
	}

	// POST or GET?
	//
	int sceMethod = -1;
	switch (method)
	{
	case METHOD_GET:	sceMethod = SCE_HTTP_METHOD_GET; break;
	case METHOD_POST:	sceMethod = SCE_HTTP_METHOD_POST; break;
	default:
		VUPRINTF("Error: Illegal Http Request method specified: %d\n", method);
	}

	// Declare variables here because we're using goto's below
	int i = 0;
	int numTuples = 0;
	int error = -1;
	int templateId = 0;
	int connectionId = 0;
	int requestId = 0;
	SceHttpEpollHandle handle = VUNULL;

	// SCE http library requires you create a "template" with some connection settings. We do it every time
	templateId = sceHttpCreateTemplate(VuPs4Sys::getHttpContextID(), HTTP_USER_AGENT, SCE_HTTP_VERSION_1_1, true);
	if (templateId < 0)
	{
		VUPRINTF("Error: Unable to create Http template: %0x\n", templateId);
		return;
	}

	// We want asynchronous operation
	error = sceHttpSetNonblock(templateId, true);
	if (error < SCE_OK)
	{
		VUPRINTF("Error: Unable to set GET Http template to NONBLOCK: %0x\n", mTemplateGetId);
		goto errorTemplate;
	}

	// Epoll handle associates a template/connection/request when using nonblocking
	error = sceHttpCreateEpoll(VuPs4Sys::getHttpContextID(), &handle);
	if (error < SCE_OK)
	{
		VUPRINTF("Error: %0x Unable to create Epoll for %s\n", error, url);
		goto errorTemplate;
	}

	// Creating the Connection for that template first
	connectionId = sceHttpCreateConnectionWithURL(templateId, url, true);
	if (connectionId < 0)
	{
		VUPRINTF("Error: Unable to create Connection for %s\n", url);
		goto errorEpoll;
	}

	// Then create the Request for that given connection
	requestId = sceHttpCreateRequestWithURL(connectionId, sceMethod, url, 0ULL);
	if (requestId < 0)
	{
		VUPRINTF("Error: Unable to create Request for %s\n", url);
		goto errorConnection;
	}

	// Assign the "Epoll" object to the new Request we just made
	error = sceHttpSetEpoll(requestId, handle, VUNULL);
	if (error < SCE_OK)
	{
		VUPRINTF("Error: %0x Unable to set Epoll for Request for %s\n", error, url);
		goto errorRequest;
	}

	// Fill out the request structure
	pRequest->mURL = url;
	pRequest->mPollHandle = handle;
	pRequest->mTemplateId = templateId;
	pRequest->mRequestId = requestId;
	pRequest->mConnectionId = connectionId;
	pRequest->mStatus = VuHttpClient::STATUS_WAITING_FOR_RESPONSE;
	pRequest->mState = STATE_SEND;

	// Copy the data block
	if (data != NULL && size > 0)
	{
		pRequest->mDataSize = size;
		pRequest->mpData = new VUUINT8[size];
		VU_MEMCPY(pRequest->mpData, size, data, size);
	}

	// Now add all the key value pairs into the header if any were added via the API prior to calling sendRequest
	//
	numTuples = pRequest->mHeaderTuples.size();
	for (i = 0; i<numTuples; i++)
	{
		std::pair<std::string, std::string> pair = pRequest->mHeaderTuples.front();
		pRequest->mHeaderTuples.pop_front();

		error = sceHttpAddRequestHeader(pRequest->mRequestId, pair.first.c_str(), pair.second.c_str(), SCE_HTTP_HEADER_OVERWRITE);
		if (error < SCE_OK)
		{
			VUPRINTF("Error: Unable to add request header for request %llx (%s, %s)\n", hRequest, pair.first.c_str(), pair.second.c_str());

			if (pRequest->mpData)
			{
				delete[] pRequest->mpData;
				pRequest->mpData = VUNULL;
			}

			goto errorRequest;
		}
	}

	error = sceHttpSetRequestContentLength(requestId, size);
	if (error < SCE_OK)
	{
		VUPRINTF("Error: Unable to sceHttpSetRequestContentLength of %d for request %llx.\n", (int)size, hRequest);
	}

	if (method == METHOD_GET)
	{
		sceHttpsDisableOption(pRequest->mRequestId, SCE_HTTPS_FLAG_CN_CHECK);
	}

	pRequest->addRef();

	// Add to request list, for tickNetwork() time processing and dispatch
	mRequests.push_back(pRequest);

	return;

errorRequest:
	error = sceHttpDeleteRequest(requestId);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDeleteRequest() error: 0x%08X\n", error);
	}
errorConnection:
	error = sceHttpDeleteConnection(connectionId);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDeleteConnection() error: 0x%08X\n", error);
	}
	pRequest->mConnectionId = 0;

errorEpoll:
	error = sceHttpDestroyEpoll(VuPs4Sys::getHttpContextID(), handle);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDestroyEpoll() error: 0x%08X\n", error);
	}

errorTemplate:
	error = sceHttpDeleteTemplate(templateId);
	if (error < SCE_OK)
	{
		VUPRINTF("sceHttpDeleteTemplate() error: 0x%08X\n", error);
	}
	pRequest->mTemplateId = 0;

	releaseRequest(pRequest);
}

