//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Xb1 HttpClient
// 
//*****************************************************************************

#include <wrl.h>
#include <ixmlhttprequest2.h>

#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/Objects/VuRefObj.h"

using namespace Microsoft::WRL; 
using namespace Microsoft::WRL::Details; 

class VuXb1HttpRequest;


class VuXb1HttpRequest2Callback : public Microsoft::WRL::RuntimeClass<RuntimeClassFlags<ClassicCom>, IXMLHTTPRequest2Callback>
{ 
private:
	VuXb1HttpRequest2Callback() : mpRequest(VUNULL), mStatus(0), mErrorCode(S_OK) {}

	friend HRESULT Microsoft::WRL::Details::MakeAndInitialize<VuXb1HttpRequest2Callback,VuXb1HttpRequest2Callback>(VuXb1HttpRequest2Callback **); 

public:
	STDMETHODIMP	OnRedirect(__RPC__in_opt IXMLHTTPRequest2 *pXHR, __RPC__in_string const WCHAR *pwszRedirectUrl);
	STDMETHODIMP	OnHeadersAvailable(__RPC__in_opt IXMLHTTPRequest2 *pXHR, DWORD dwStatus, __RPC__in_string const WCHAR *pwszStatus);
	STDMETHODIMP	OnDataAvailable(__RPC__in_opt IXMLHTTPRequest2 *pXHR, __RPC__in_opt ISequentialStream *pResponseStream);
	STDMETHODIMP	OnResponseReceived(__RPC__in_opt IXMLHTTPRequest2 *pXHR, __RPC__in_opt ISequentialStream *pResponseStream);
	STDMETHODIMP	OnError(__RPC__in_opt IXMLHTTPRequest2 *pXHR, HRESULT hrError);

	VuXb1HttpRequest	*mpRequest;
	DWORD				mStatus;
	HRESULT				mErrorCode;
};

class VuXb1HttpStream : public Microsoft::WRL::RuntimeClass<RuntimeClassFlags<ClassicCom>, ISequentialStream> 
{ 
private: 
	VuXb1HttpStream() : mpRequest(VUNULL), mReadOffset(0) {}

    friend Microsoft::WRL::ComPtr<VuXb1HttpStream> Microsoft::WRL::Details::Make<VuXb1HttpStream>(); 
 
public:
	STDMETHODIMP	Read(_Out_writes_bytes_to_(cb, *pcbRead) void *pv, ULONG cb, _Out_opt_  ULONG *pcbRead); 
	STDMETHODIMP	Write(_In_reads_bytes_(cb) const void *pv, ULONG cb, _Out_opt_  ULONG *pcbWritten); 

	VuXb1HttpRequest	*mpRequest;
	int					mReadOffset;
};


class VuXb1HttpRequest : public VuRefObj
{
public:
	VuXb1HttpRequest() : mStatus(VuHttpClient::STATUS_READY) {}

	std::string				mData;
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;

	ComPtr<IXMLHTTPRequest2>			mXmlHttpRequest;
	ComPtr<VuXb1HttpRequest2Callback>	mMyXmlHttpRequestCallback;
	ComPtr<IXMLHTTPRequest2Callback>	mXmlHttpRequestCallback;
	ComPtr<VuXb1HttpStream>				mMyXmlStream;
	ComPtr<ISequentialStream>			mXmlStream;
};

class VuXb1HttpClient : public VuHttpClient
{
public:
	VuXb1HttpClient();

	virtual bool		init();
	virtual void		release();

	virtual VUHANDLE	createRequest();
	virtual void		releaseRequest(VUHANDLE hRequest);
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, const char *value);
	virtual void		setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS);

	virtual void		sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size);

	virtual eStatus		getStatus(VUHANDLE hRequest);

	virtual const std::string	&getResponse(VUHANDLE hRequest);
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuXb1HttpClient);


//*****************************************************************************
VuXb1HttpClient::VuXb1HttpClient()
{
}

//*****************************************************************************
bool VuXb1HttpClient::init()
{
	if ( !VuHttpClient::init() )
		return false;

	return true;
}

//*****************************************************************************
void VuXb1HttpClient::release()
{
	VuHttpClient::release();
}

//*****************************************************************************
VUHANDLE VuXb1HttpClient::createRequest()
{
	VuXb1HttpRequest *pRequest = new VuXb1HttpRequest;

	if ( CoCreateInstance(CLSID_FreeThreadedXMLHTTP60, nullptr, CLSCTX_SERVER, IID_PPV_ARGS(&pRequest->mXmlHttpRequest)) == S_OK )
	{
		if ( MakeAndInitialize<VuXb1HttpRequest2Callback>(&pRequest->mMyXmlHttpRequestCallback) == S_OK )
		{
			pRequest->mMyXmlHttpRequestCallback->mpRequest = pRequest;
			if ( pRequest->mMyXmlHttpRequestCallback.As(&pRequest->mXmlHttpRequestCallback) == S_OK )
			{
				if ( pRequest->mMyXmlStream = Make<VuXb1HttpStream>() )
				{
					pRequest->mMyXmlStream->mpRequest = pRequest;
					if ( pRequest->mMyXmlStream.As(&pRequest->mXmlStream) == S_OK )
					{
						return pRequest;
					}
				}
			}
		}
	}

	delete pRequest;
	return VUNULL;
}

//*****************************************************************************
void VuXb1HttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuXb1HttpRequest *pRequest = static_cast<VuXb1HttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuXb1HttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuXb1HttpRequest *pRequest = static_cast<VuXb1HttpRequest *>(hRequest);

	std::wstring wideKey, wideValue;
	VuUtf8::convertUtf8StringToWCharString(key, wideKey);
	VuUtf8::convertUtf8StringToWCharString(value, wideValue);

	pRequest->mXmlHttpRequest->SetRequestHeader(wideKey.c_str(), wideValue.c_str());
}

//*****************************************************************************
void VuXb1HttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuXb1HttpRequest *pRequest = static_cast<VuXb1HttpRequest *>(hRequest);

	pRequest->mXmlHttpRequest->SetProperty(XHR_PROP_TIMEOUT, timeoutMS);
}

//*****************************************************************************
void VuXb1HttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuXb1HttpRequest *pRequest = static_cast<VuXb1HttpRequest *>(hRequest);

	if ( pRequest->mStatus == STATUS_READY )
	{
		if ( size )
		{
			pRequest->mData.resize(size);
			VU_MEMCPY(&pRequest->mData[0], size, data, size);
		}

		pRequest->mStatus = STATUS_WAITING_FOR_RESPONSE;
		pRequest->addRef();

		std::wstring wideUriPath;
		VuUtf8::convertUtf8StringToWCharString(url, wideUriPath);
		
		const wchar_t *strMethod = (method == METHOD_GET) ? L"GET" : L"POST";

		bool success = false;
		if ( pRequest->mXmlHttpRequest->Open(strMethod, wideUriPath.c_str(), pRequest->mXmlHttpRequestCallback.Get(), NULL, NULL, NULL, NULL) == S_OK )
			if ( pRequest->mXmlHttpRequest->Send(pRequest->mXmlStream.Get(), pRequest->mData.size()) == S_OK )
				success = true;

		if ( !success )
		{
				pRequest->mStatus = STATUS_ERROR;
				pRequest->removeRef();
		}

	}
	else
	{
		VUPRINTF("VuHttpClient::sendRequest() already sent\n");
	}
}

//*****************************************************************************
VuHttpClient::eStatus VuXb1HttpClient::getStatus(VUHANDLE hRequest)
{
	VuXb1HttpRequest *pRequest = static_cast<VuXb1HttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuXb1HttpClient::getResponse(VUHANDLE hRequest)
{
	VuXb1HttpRequest *pRequest = static_cast<VuXb1HttpRequest *>(hRequest);

	return pRequest->mResponse;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpRequest2Callback::OnRedirect(__RPC__in_opt IXMLHTTPRequest2 *pXHR, __RPC__in_string const WCHAR *pwszRedirectUrl)
{
	return S_OK;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpRequest2Callback::OnHeadersAvailable(__RPC__in_opt IXMLHTTPRequest2 *pXHR, DWORD dwStatus, __RPC__in_string const WCHAR *pwszStatus)
{
	mStatus = dwStatus;
	return S_OK;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpRequest2Callback::OnDataAvailable(__RPC__in_opt IXMLHTTPRequest2 *pXHR, __RPC__in_opt ISequentialStream *pResponseStream)
{
	VUBYTE buffer[4096];

	HRESULT hr = S_OK;
	while ( hr == S_OK )
	{
		ULONG bytesRead = 0;
		hr = pResponseStream->Read(buffer, sizeof(buffer), &bytesRead);
		if ( bytesRead )
		{
			int offset = (int)mpRequest->mResponse.size();
			mpRequest->mResponse.resize(mpRequest->mResponse.size() + bytesRead);
			VU_MEMCPY(&mpRequest->mResponse[offset], bytesRead, buffer, bytesRead);
		}
	}

	return hr;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpRequest2Callback::OnResponseReceived(__RPC__in_opt IXMLHTTPRequest2 *pXHR, __RPC__in_opt ISequentialStream *pResponseStream)
{
	mpRequest->mStatus = VuHttpClient::STATUS_RESPONSE_RECEIVED;
	mpRequest->removeRef();

	return S_OK;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpRequest2Callback::OnError(__RPC__in_opt IXMLHTTPRequest2 *pXHR, HRESULT hrError)
{
	mErrorCode = hrError;
	mpRequest->mStatus = VuHttpClient::STATUS_ERROR;
	mpRequest->removeRef();

	return S_OK;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpStream::Read(_Out_writes_bytes_to_(cb, *pcbRead) void *pv, ULONG cb, _Out_opt_  ULONG *pcbRead)
{
	if ( pv && cb )
	{
		int bytesRemaining = (int)mpRequest->mData.size() - mReadOffset;
		int bytesRead = VuMin(cb, bytesRemaining);

		if ( bytesRead )
		{
			VU_MEMCPY(pv, cb, &mpRequest->mData[mReadOffset], bytesRead);
			mReadOffset += bytesRead;
		}

		*pcbRead = bytesRead;

		return (cb == (ULONG)bytesRemaining) ? S_OK : S_FALSE;
	}

	return S_FALSE;
}

//*****************************************************************************
STDMETHODIMP VuXb1HttpStream::Write(_In_reads_bytes_(cb) const void *pv, ULONG cb, _Out_opt_  ULONG *pcbWritten)
{
	VUASSERT(0, "Not expecting to write to stream!");
	*pcbWritten = cb;
	return S_OK;
}
