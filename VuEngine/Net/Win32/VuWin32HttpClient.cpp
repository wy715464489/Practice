//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Win32 HttpClient
// 
//*****************************************************************************

#include <winhttp.h>
#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Libs/http-parser/http_parser.h"
#include "VuEngine/Util/VuUtf8.h"

#pragma comment(lib, "winhttp.lib")


#define HTTP_CLIENT_READ_CHUNK_SIZE (8*1024)

class VuWin32HttpRequest : public VuRefObj
{
public:
	VuWin32HttpRequest() : mStatus(VuHttpClient::STATUS_READY), mTimeoutMS(10000), mConnection(NULL), mRequest(NULL), mStatusCode(0) {}
	~VuWin32HttpRequest();

	std::string				mURL;
	std::string				mHeader;
	std::string				mData;
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;
	int						mTimeoutMS;
	HINTERNET				mConnection;
	HINTERNET				mRequest;
	DWORD					mStatusCode;
	VUBYTE					mChunk[HTTP_CLIENT_READ_CHUNK_SIZE];
};

class VuWin32HttpClient : public VuHttpClient
{
public:
	VuWin32HttpClient();

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
	static void	CALLBACK Callback(HINTERNET handle, DWORD_PTR context, DWORD code, void *info, DWORD length);

	typedef std::queue<VuWin32HttpRequest *> Requests;
	Requests	mRequests;

	HINTERNET	mSession;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuWin32HttpClient);


//*****************************************************************************
VuWin32HttpClient::VuWin32HttpClient():
	mSession(NULL)
{
}

//*****************************************************************************
bool VuWin32HttpClient::init()
{
	if ( !VuHttpClient::init() )
		return false;

	mSession = WinHttpOpen(0, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
	if ( mSession == NULL )
		return false;

	return true;
}

//*****************************************************************************
void VuWin32HttpClient::release()
{
	WinHttpCloseHandle(mSession);

	VuHttpClient::release();
}

//*****************************************************************************
VUHANDLE VuWin32HttpClient::createRequest()
{
	return new VuWin32HttpRequest;
}

//*****************************************************************************
void VuWin32HttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuWin32HttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>(hRequest);

	pRequest->mHeader += key;
	pRequest->mHeader += ": ";
	pRequest->mHeader += value;
	pRequest->mHeader += "\n";
}

//*****************************************************************************
void VuWin32HttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>(hRequest);

	pRequest->mTimeoutMS = timeoutMS;
}

//*****************************************************************************
void VuWin32HttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *strUrl, const void *data, int size)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>(hRequest);

	pRequest->mURL = strUrl;
	if ( size )
	{
		pRequest->mData.resize(size);
		VU_MEMCPY(&pRequest->mData[0], size, data, size);
	}

	bool success = false;

	struct http_parser_url url;
	if ( http_parser_parse_url(strUrl, strlen(strUrl), false, &url) == 0 )
	{
		std::string schema = pRequest->mURL.substr(url.field_data[UF_SCHEMA].off, url.field_data[UF_SCHEMA].len);
		std::string host = pRequest->mURL.substr(url.field_data[UF_HOST].off, url.field_data[UF_HOST].len);
		int port = url.port;
		std::string path = pRequest->mURL.substr(url.field_data[UF_PATH].off, url.field_data[UF_PATH].len);

		std::wstring wideHost;
		VuUtf8::convertUtf8StringToWCharString(host.c_str(), wideHost);
		pRequest->mConnection = WinHttpConnect(mSession, wideHost.c_str(), port, 0);
		if ( pRequest->mConnection )
		{
			const wchar_t *strMethod = L"GET";
			if ( method == METHOD_POST )
				strMethod = L"POST";

			DWORD flags = (schema == "https") ? WINHTTP_FLAG_SECURE  : 0;

			std::wstring widePath;
			VuUtf8::convertUtf8StringToWCharString(path.c_str(), widePath);

			pRequest->mRequest = WinHttpOpenRequest(pRequest->mConnection, strMethod, widePath.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
			if ( pRequest->mRequest )
			{
				if ( WinHttpSetStatusCallback(pRequest->mRequest, Callback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0) != WINHTTP_INVALID_STATUS_CALLBACK )
				{
					std::wstring wideHeader;
					VuUtf8::convertUtf8StringToWCharString(pRequest->mHeader.c_str(), wideHeader);
					if ( wideHeader.empty() || WinHttpAddRequestHeaders(pRequest->mRequest, wideHeader.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD ) )
					{
						if ( WinHttpSetTimeouts(pRequest->mRequest, pRequest->mTimeoutMS, pRequest->mTimeoutMS, pRequest->mTimeoutMS, pRequest->mTimeoutMS) )
						{
							if ( WinHttpSendRequest(pRequest->mRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)&pRequest->mData[0], pRequest->mData.size(), pRequest->mData.size(), (DWORD_PTR)pRequest) )
							{
								success = true;
							}
						}
					}
				}
			}
		}
	}

	if ( success )
	{
		pRequest->mStatus = STATUS_WAITING_FOR_RESPONSE;
		pRequest->addRef();
	}
	else
	{
		pRequest->mStatus = STATUS_ERROR;
	}
}

//*****************************************************************************
VuHttpClient::eStatus VuWin32HttpClient::getStatus(VUHANDLE hRequest)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuWin32HttpClient::getResponse(VUHANDLE hRequest)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>(hRequest);

	return pRequest->mResponse;
}

//*****************************************************************************
void VuWin32HttpClient::Callback(HINTERNET handle, DWORD_PTR context, DWORD code, void *info, DWORD length)
{
	VuWin32HttpRequest *pRequest = static_cast<VuWin32HttpRequest *>((void *)context);

	switch ( code )
	{
		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
		{
			if ( !WinHttpReceiveResponse(handle, 0) )
			{
				pRequest->mStatus = STATUS_ERROR;
				pRequest->removeRef();
			}

			break;
		}
		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
		{
		    DWORD statusCodeSize = sizeof(pRequest->mStatusCode);
			if ( !WinHttpQueryHeaders(handle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &pRequest->mStatusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX) )
			{
				pRequest->mStatus = STATUS_ERROR;
				pRequest->removeRef();
				break;
			}

			if ( !WinHttpReadData(handle, pRequest->mChunk, HTTP_CLIENT_READ_CHUNK_SIZE, 0) )
			{
				pRequest->mStatus = STATUS_ERROR;
				pRequest->removeRef();
			}

			break;
		}
		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		{
			if ( length > 0 )
			{
				int offset = pRequest->mResponse.size();
				pRequest->mResponse.resize(offset + length);
				VU_MEMCPY(&pRequest->mResponse[offset], length, pRequest->mChunk, length);

				if ( !WinHttpReadData(handle, pRequest->mChunk, HTTP_CLIENT_READ_CHUNK_SIZE, 0) )
				{
					pRequest->mStatus = STATUS_ERROR;
					pRequest->removeRef();
				}
			}
			else
			{
				pRequest->mStatus = STATUS_RESPONSE_RECEIVED;
				pRequest->removeRef();
			}

			break;
		}
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
		{
			pRequest->mStatus = STATUS_ERROR;
			pRequest->removeRef();
			break;
		}
	}
}

//*****************************************************************************
VuWin32HttpRequest::~VuWin32HttpRequest()
{
	if ( mRequest )
	{
		WinHttpSetStatusCallback(mRequest, NULL, NULL, NULL);
		WinHttpCloseHandle(mRequest);
	}

	if ( mConnection )
	{
		WinHttpCloseHandle(mConnection);
	}
}
