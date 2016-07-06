//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Generic HttpClient
// 
//*****************************************************************************

#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/HAL/Net/VuNet.h"
#include "VuEngine/HAL/Thread/VuThread.h"
#include "VuEngine/Libs/http-parser/http_parser.h"


class VuGenericHttpRequest : public VuRefObj
{
public:
	VuGenericHttpRequest() : mStatus(VuHttpClient::STATUS_READY), mTimeoutMS(10000) {}

	std::string				mURL;
	std::string				mHeader;
	std::string				mData;
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;
	int						mTimeoutMS;
	int						mMethod;
};


class VuGenericHttpClient : public VuHttpClient
{
public:
	VuGenericHttpClient();

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
	static void		threadProc(void *pParam) { static_cast<VuGenericHttpClient *>(pParam)->threadProc(); }
	void			threadProc();

	typedef std::queue<VuGenericHttpRequest *> Requests;
	Requests		mRequests;

	// synchronization
	VUHANDLE		mCriticalSection;
	VUHANDLE		mhThread;
	bool			mTerminateThread;
	VUHANDLE		mWorkAvailableEvent;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuGenericHttpClient);

// constants
#define HTTP_CLIENT_READ_CHUNK_SIZE (16*1024)

//*****************************************************************************
static int HttpDataCB(http_parser *parser, const char *at, size_t length)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(parser->data);

	pRequest->mResponse.append(at, length);

	return 0;
}

//*****************************************************************************
VuGenericHttpClient::VuGenericHttpClient():
	mTerminateThread(false)
{
}

//*****************************************************************************
bool VuGenericHttpClient::init()
{
	if ( !VuHttpClient::init() )
		return false;

	mCriticalSection = VuThread::IF()->createCriticalSection();

	// threading
	mWorkAvailableEvent = VuThread::IF()->createEvent();
	mhThread = VuThread::IF()->createThread(threadProc, this);

	return true;
}

//*****************************************************************************
void VuGenericHttpClient::release()
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

	VuHttpClient::release();
}

//*****************************************************************************
VUHANDLE VuGenericHttpClient::createRequest()
{
	return new VuGenericHttpRequest;
}

//*****************************************************************************
void VuGenericHttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuGenericHttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(hRequest);

	pRequest->mHeader += key;
	pRequest->mHeader += ": ";
	pRequest->mHeader += value;
	pRequest->mHeader += "\n";
}

//*****************************************************************************
void VuGenericHttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(hRequest);
	
	pRequest->mTimeoutMS = timeoutMS;
}

//*****************************************************************************
void VuGenericHttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(hRequest);

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
VuHttpClient::eStatus VuGenericHttpClient::getStatus(VUHANDLE hRequest)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuGenericHttpClient::getResponse(VUHANDLE hRequest)
{
	VuGenericHttpRequest *pRequest = static_cast<VuGenericHttpRequest *>(hRequest);

	return pRequest->mResponse;
}

//*****************************************************************************
void VuGenericHttpClient::threadProc()
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
			VuGenericHttpRequest *pRequest = mRequests.front();
			mRequests.pop();
			VuThread::IF()->leaveCriticalSection(mCriticalSection);

			struct http_parser_url url;
			if ( http_parser_parse_url(pRequest->mURL.c_str(), pRequest->mURL.length(), false, &url) == 0 )
			{
				std::string host = pRequest->mURL.substr(url.field_data[UF_HOST].off, url.field_data[UF_HOST].len);

				VuTcpSocket *pTcpSocket = VuTcpSocket::create(0);
				if ( pTcpSocket )
				{
					if ( pTcpSocket->setTimeOut(pRequest->mTimeoutMS, pRequest->mTimeoutMS) )
					{
						if ( pTcpSocket->connect(host.c_str(), url.port, pRequest->mTimeoutMS) )
						{
							// send request
							std::string request;
							if ( pRequest->mData.size() )
							{
								request += std::string("POST ") + pRequest->mURL + " HTTP/1.0\n";
								request += pRequest->mHeader;
								request += "\n";
								request += pRequest->mData;
							}
							else
							{
								request += std::string("GET ") + pRequest->mURL + " HTTP/1.0\n";
								request += pRequest->mHeader;
								request += "\n";
							}

							int bytesToSend = (int)request.length() + 1;
							if ( pTcpSocket->send(request.c_str(), bytesToSend) == bytesToSend )
							{
								// read response
								http_parser_settings settings;
								memset(&settings, 0, sizeof(settings));
								settings.on_body = HttpDataCB;

								http_parser parser;
								http_parser_init(&parser, HTTP_RESPONSE);
								parser.data = pRequest;

								char buffer[HTTP_CLIENT_READ_CHUNK_SIZE];
								while ( int bytesRead = pTcpSocket->recv(buffer, HTTP_CLIENT_READ_CHUNK_SIZE) )
								{
									if ( bytesRead == -1 )
									{
										// timeout
										pRequest->mResponse.clear();
										break;
									}
									int bytesParsed = (int)http_parser_execute(&parser, &settings, buffer, bytesRead);
									if ( bytesParsed != bytesRead )
									{
										// error
										pRequest->mResponse.clear();
										break;
									}
								}
							}
						}
					}

					delete pTcpSocket;
				}
			}

			if ( pRequest->mResponse.size() )
				pRequest->mStatus = STATUS_RESPONSE_RECEIVED;
			else
				pRequest->mStatus = STATUS_ERROR;

			pRequest->removeRef();
		}
	}

	VuThread::IF()->endThread();
}
