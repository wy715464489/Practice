//*****************************************************************************
//
//  Copyright (c) 2013-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Ios HttpClient
// 
//*****************************************************************************

#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"

class VuIosHttpRequest;

@interface VuIosHttpRequestDelegate : NSObject
{
@public
	VuIosHttpRequest	*mpRequest;
}
@end


class VuIosHttpRequest : public VuRefObj
{
public:
	VuIosHttpRequest();
	~VuIosHttpRequest();

	NSMutableURLRequest			*mUrlRequest;
	NSURLConnection				*mUrlConnection;
	VuIosHttpRequestDelegate	*mDelegate;
	
	std::string				mResponse;
	VuHttpClient::eStatus	mStatus;
};


class VuIosHttpClient : public VuHttpClient
{
public:
	virtual VUHANDLE	createRequest();
	virtual void		releaseRequest(VUHANDLE hRequest);
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, const char *value);
	virtual void		setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS);

	virtual void		sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size);

	virtual eStatus		getStatus(VUHANDLE hRequest);

	virtual const std::string	&getResponse(VUHANDLE hRequest);
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuHttpClient, VuIosHttpClient);


//*****************************************************************************
VuIosHttpRequest::VuIosHttpRequest():
	mStatus(VuHttpClient::STATUS_READY)
{
	mUrlRequest = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:@""]];
	[mUrlRequest setCachePolicy:NSURLRequestReloadIgnoringLocalCacheData];
	
	mUrlConnection = [NSURLConnection alloc];
	
	mDelegate = [VuIosHttpRequestDelegate alloc];
	mDelegate->mpRequest = this;
}

//*****************************************************************************
VuIosHttpRequest::~VuIosHttpRequest()
{
}

//*****************************************************************************
VUHANDLE VuIosHttpClient::createRequest()
{
	return new VuIosHttpRequest;
}

//*****************************************************************************
void VuIosHttpClient::releaseRequest(VUHANDLE hRequest)
{
	VuIosHttpRequest *pRequest = static_cast<VuIosHttpRequest *>(hRequest);

	pRequest->removeRef();
}

//*****************************************************************************
void VuIosHttpClient::setContentHeader(VUHANDLE hRequest, const char *key, const char *value)
{
	VuIosHttpRequest *pRequest = static_cast<VuIosHttpRequest *>(hRequest);

	[pRequest->mUrlRequest setValue:[NSString stringWithUTF8String:value] forHTTPHeaderField:[NSString stringWithUTF8String:key]];
}

//*****************************************************************************
void VuIosHttpClient::setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS)
{
	VuIosHttpRequest *pRequest = static_cast<VuIosHttpRequest *>(hRequest);
	
	[pRequest->mUrlRequest setTimeoutInterval:(timeoutMS/1000.0)];
}

//*****************************************************************************
void VuIosHttpClient::sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size)
{
	VuIosHttpRequest *pRequest = static_cast<VuIosHttpRequest *>(hRequest);

	if ( pRequest->mStatus == STATUS_READY )
	{
		[pRequest->mUrlRequest setURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
		
		if ( method == METHOD_POST )
			[pRequest->mUrlRequest setHTTPMethod:@"POST"];

		if ( size )
			[pRequest->mUrlRequest setHTTPBody:[NSData dataWithBytes: data length: size]];
		
		[pRequest->mUrlConnection initWithRequest:pRequest->mUrlRequest delegate:pRequest->mDelegate];
		
		pRequest->mStatus = STATUS_WAITING_FOR_RESPONSE;
		pRequest->addRef();
	}
	else
	{
		VUPRINTF("VuHttpClient::sendRequest() already sent\n");
	}
}

//*****************************************************************************
VuHttpClient::eStatus VuIosHttpClient::getStatus(VUHANDLE hRequest)
{
	VuIosHttpRequest *pRequest = static_cast<VuIosHttpRequest *>(hRequest);

	return pRequest->mStatus;
}

//*****************************************************************************
const std::string &VuIosHttpClient::getResponse(VUHANDLE hRequest)
{
	VuIosHttpRequest *pRequest = static_cast<VuIosHttpRequest *>(hRequest);

	return pRequest->mResponse;
}


@implementation VuIosHttpRequestDelegate

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	mpRequest->mStatus = VuHttpClient::STATUS_ERROR;
	mpRequest->removeRef();
}

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	mpRequest->mResponse.clear();
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
	mpRequest->mResponse.append((const char *)[data bytes], [data length]);
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	mpRequest->mStatus = VuHttpClient::STATUS_RESPONSE_RECEIVED;
	mpRequest->removeRef();
}

@end

