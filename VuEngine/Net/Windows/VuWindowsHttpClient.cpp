//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Windows HttpClient
// 
//*****************************************************************************

#include <ppltasks.h>

#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"
#include "VuEngine/Math/VuMath.h"
#include "VuEngine/Util/VuUtf8.h"

using namespace Concurrency;
using namespace Windows::Foundation;
using namespace Windows::Web::Http;
using namespace Windows::Storage::Streams;

class VuWindowsHttpClient : public VuHttpClient
{
public:
	VuWindowsHttpClient();

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
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuWindowsHttpClient);


class VuWindowsHttpRequest : public VuRefObj
{
private:
	~VuWindowsHttpRequest() {}
public:
	VuWindowsHttpRequest() : mStatus(VuHttpClient::STATUS_READY) {}

	typedef std::map<std::string, std::string> Header;

	Header					mContentHeader;
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;
};


//*****************************************************************************
VuWindowsHttpClient::VuWindowsHttpClient()
{
}

//*****************************************************************************
bool VuWindowsHttpClient::init()
{
	if ( !VuHttpClient::init() )
		return false;

	return true;
}

//*****************************************************************************
void VuWindowsHttpClient::release()
{
	VuHttpClient::release();
}

//*****************************************************************************
VUHANDLE VuWindowsHttpClient::createRequest()
{
	return new VuWindowsHttpRequest;
}

//*****************************************************************************
void VuWindowsHttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuWindowsHttpRequest *pRequest = static_cast<VuWindowsHttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuWindowsHttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuWindowsHttpRequest *pRequest = static_cast<VuWindowsHttpRequest *>(hRequest);

	pRequest->mContentHeader[key] = value;
}

//*****************************************************************************
void VuWindowsHttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuWindowsHttpRequest *pRequest = static_cast<VuWindowsHttpRequest *>(hRequest);

	// don't know how to do this with Windows::Web::Http::HttpClient - not in docs
}

//*****************************************************************************
void VuWindowsHttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuWindowsHttpRequest *pRequest = static_cast<VuWindowsHttpRequest *>(hRequest);

	if (pRequest->mStatus == STATUS_READY)
	{
		pRequest->mStatus = STATUS_WAITING_FOR_RESPONSE;
		pRequest->addRef();

		std::wstring wideUriPath;
		VuUtf8::convertUtf8StringToWCharString(url, wideUriPath);

		auto uriPath = ref new Platform::String(wideUriPath.c_str());

		HttpClient^ httpClient = ref new HttpClient();

		IAsyncOperationWithProgress<HttpResponseMessage^, HttpProgress>^ task;

		if (method == METHOD_POST)
		{
			InMemoryRandomAccessStream^ memoryStream = ref new InMemoryRandomAccessStream();
			DataWriter^ dataWriter = ref new DataWriter(memoryStream);
			if (size)
				dataWriter->WriteBytes(ref new Platform::Array<unsigned char>((unsigned char *)data, size));
			IBuffer^ buffer = dataWriter->DetachBuffer();

			auto httpContent = ref new HttpBufferContent(buffer);
			for (VuWindowsHttpRequest::Header::const_iterator iter = pRequest->mContentHeader.begin(); iter != pRequest->mContentHeader.end(); iter++)
			{
				std::wstring wideKey, wideValue;
				VuUtf8::convertUtf8StringToWCharString(iter->first.c_str(), wideKey);
				VuUtf8::convertUtf8StringToWCharString(iter->second.c_str(), wideValue);
				httpContent->Headers->TryAppendWithoutValidation(ref new Platform::String(wideKey.c_str()), ref new Platform::String(wideValue.c_str()));
			}

			task = httpClient->PostAsync(ref new Uri(uriPath), httpContent);
		}
		else
		{
			task = httpClient->GetAsync(ref new Uri(uriPath));
		}

		create_task(task).then([this, pRequest](HttpResponseMessage^ responseMessage)
		{
			create_task(responseMessage->Content->ReadAsBufferAsync()).then([this, pRequest](IBuffer^ response)
			{
				int length = response->Length;

				DataReader ^reader = DataReader::FromBuffer(response);
				pRequest->mResponse.resize(length);

				if (length > 0)
				{
					std::vector<unsigned char> data(length);
					reader->ReadBytes(Platform::ArrayReference<unsigned char>(&data[0], length));
					VU_MEMCPY(&pRequest->mResponse[0], length, &data[0], length);
				}

				pRequest->mStatus = VuHttpClient::STATUS_RESPONSE_RECEIVED;
				pRequest->removeRef();
			});
		})
		.then([this, pRequest](concurrency::task<void>t)
		{
			try
			{
				t.get();
			}
			catch (Platform::Exception^ exception)
			{
				pRequest->mStatus = VuHttpClient::STATUS_ERROR;
				pRequest->removeRef();
			}
		});
	}
	else
	{
		VUPRINTF("VuHttpClient::sendRequest() already sent\n");
	}
}

//*****************************************************************************
VuHttpClient::eStatus VuWindowsHttpClient::getStatus(VUHANDLE hRequest)
{
	VuWindowsHttpRequest *pRequest = static_cast<VuWindowsHttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuWindowsHttpClient::getResponse(VUHANDLE hRequest)
{
	VuWindowsHttpRequest *pRequest = static_cast<VuWindowsHttpRequest *>(hRequest);

	return pRequest->mResponse;
}
