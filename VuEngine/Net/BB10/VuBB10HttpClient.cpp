//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic HttpClient
// 
//*****************************************************************************

#include <curl/curl.h>

#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Math/VuMath.h"


class VuBB10HttpRequest : public VuRefObj
{
public:
	VuBB10HttpRequest() : mMethod(VuHttpCLient::METHOD_GET), mpHeaderList(VUNULL), mStatus(VuHttpClient::STATUS_READY), mTimeoutMS(10000), mReadOffset(0) {}
	~VuBB10HttpRequest() { curl_slist_free_all(mpHeaderList); }

	VuHttpClient::eMethod	mMethod;
	std::string				mURL;
	curl_slist				*mpHeaderList;
	std::string				mData;
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;
	int						mTimeoutMS;
	int						mReadOffset;
};


class VuBB10HttpClient : public VuHttpClient
{
public:
	VuBB10HttpClient();

	virtual bool		init();
	virtual void		release();

	virtual VUHANDLE	createRequest();
	virtual void		releaseRequest(VUHANDLE hRequest);
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, const char *value);
	virtual void		setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS);

	virtual void		sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size);

	virtual eStatus		getStatus(VUHANDLE hRequest);

	virtual const std::string	&getResponse(VUHANDLE hRequest);

protected:
	static void		threadProc(void *pParam) { static_cast<VuBB10HttpClient *>(pParam)->threadProc(); }
	void			threadProc();

	typedef std::queue<VuBB10HttpRequest *> Requests;
	Requests		mRequests;

	// synchronization
	VUHANDLE		mCriticalSection;
	VUHANDLE		mhThread;
	bool			mTerminateThread;
	VUHANDLE		mWorkAvailableEvent;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuBB10HttpClient);

// callbacks
size_t WriteHttpDataCB(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(userdata);

	pRequest->mResponse.append(ptr, size*nmemb);

	return size*nmemb;
}

size_t ReadHttpDataCB(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(userdata);

	int remaining = pRequest->mData.size() - pRequest->mReadOffset;
	int chunkSize = VuMin((int)(size*nmemb), remaining);

	if ( chunkSize > 0 )
	{
		memcpy(ptr, &pRequest->mData[pRequest->mReadOffset], chunkSize);
		pRequest->mReadOffset += chunkSize;
	}

	return chunkSize;
}

//*****************************************************************************
VuBB10HttpClient::VuBB10HttpClient():
	mTerminateThread(false)
{
}

//*****************************************************************************
bool VuBB10HttpClient::init()
{
	if ( !VuHttpClient::init() )
		return false;

	// curl initialization
	curl_global_init(CURL_GLOBAL_DEFAULT);

	mCriticalSection = VuThread::IF()->createCriticalSection();

	// threading
	mWorkAvailableEvent = VuThread::IF()->createEvent();
	mhThread = VuThread::IF()->createThread(threadProc, this);

	return true;
}

//*****************************************************************************
void VuBB10HttpClient::release()
{
	VUPRINTF("Terminating VuHttpClient thread...\n");

	mTerminateThread = true;
	VuThread::IF()->setEvent(mWorkAvailableEvent);
	VuThread::IF()->joinThread(mhThread);

	VuThread::IF()->destroyEvent(mWorkAvailableEvent);

	VuThread::IF()->deleteCriticalSection(mCriticalSection);

	while ( mRequests.size() )
	{
		mRequests.front()->removeRef();
		mRequests.pop();
	}

	// curl cleanup
	curl_global_cleanup();

	VuHttpClient::release();
}

//*****************************************************************************
VUHANDLE VuBB10HttpClient::createRequest()
{
	return new VuBB10HttpRequest;
}

//*****************************************************************************
void VuBB10HttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuBB10HttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(hRequest);

	char headerLine[256];
	VU_SPRINTF(headerLine, sizeof(headerLine), "%s: %s", key, value);

	pRequest->mpHeaderList = curl_slist_append(pRequest->mpHeaderList, headerLine);
}

//*****************************************************************************
void VuBB10HttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(hRequest);
	
	pRequest->mTimeoutMS = timeoutMS;
}

//*****************************************************************************
void VuBB10HttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(hRequest);

	if ( pRequest->mStatus == STATUS_READY )
	{
		pRequest->mMethod = method;
		pRequest->mURL = url;
		if ( size )
		{
			pRequest->mData.resize(size);
			VU_MEMCPY(&pRequest->mData[0], size, data, size);
		}

		pRequest->mStatus = STATUS_WAITING_FOR_RESPONSE;
		pRequest->addRef();

		VuThread::IF()->enterCriticalSection(mCriticalSection);
		mRequests.push(pRequest);
		VuThread::IF()->leaveCriticalSection(mCriticalSection);

		VuThread::IF()->setEvent(mWorkAvailableEvent);
	}
	else
	{
		VUPRINTF("VuHttpClient::sendRequest() already sent\n");
	}
}

//*****************************************************************************
VuHttpClient::eStatus VuBB10HttpClient::getStatus(VUHANDLE hRequest)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuBB10HttpClient::getResponse(VUHANDLE hRequest)
{
	VuBB10HttpRequest *pRequest = static_cast<VuBB10HttpRequest *>(hRequest);

	return pRequest->mResponse;
}

//*****************************************************************************
void VuBB10HttpClient::threadProc()
{
	VUPRINTF("VuHttpClient thread starting...\n");

	for (;;)
	{
		if ( !VuThread::IF()->waitForSingleObject(mWorkAvailableEvent) )
		{
			VUPRINTF("VuHttpClient::threadProc() wait error!\n");
			break;
		}
		if ( mTerminateThread )
		{
			VUPRINTF("VuHttpClient thread exiting...\n");
			break;
		}

		// do work
		{
			VuThread::IF()->enterCriticalSection(mCriticalSection);
			VuBB10HttpRequest *pRequest = mRequests.front();
			mRequests.pop();
			VuThread::IF()->leaveCriticalSection(mCriticalSection);
			
			bool success = false;

			if ( CURL *curl = curl_easy_init() )
			{
				curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, pRequest->mTimeoutMS);
				curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, pRequest->mTimeoutMS);
				curl_easy_setopt(curl, CURLOPT_URL, pRequest->mURL.c_str());
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pRequest->mpHeaderList);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteHttpDataCB);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, pRequest);

				if ( pRequest->mMethod == METHOD_POST )
					curl_easy_setopt(curl, CURLOPT_POST, 1);

				if ( pRequest->mData.size() )
				{
					curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadHttpDataCB);
					curl_easy_setopt(curl, CURLOPT_READDATA, pRequest);
					curl_easy_setopt(curl, CURLOPT_INFILESIZE, pRequest->mData.size());
				}

				if ( curl_easy_perform(curl) == CURLE_OK )
					success = true;
				else
					pRequest->mResponse.clear();

				curl_easy_cleanup(curl);
			}

			if ( success )
				pRequest->mStatus = STATUS_RESPONSE_RECEIVED;
			else
				pRequest->mStatus = STATUS_ERROR;

			pRequest->removeRef();
		}
	}

	VuThread::IF()->endThread();
}
