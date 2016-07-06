//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuPs4HttpClient class
// 
//*****************************************************************************

#pragma once

#include <list>
#include <libhttp.h>

#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Objects/VuRefObj.h"

enum 
{
	STATE_SEND,
	STATE_GET_STATUS,
	STATE_GET_LENGTH,
	STATE_RECEIVE,
	STATE_DONE,
};

#define HTTP_RECEIVE_BUFFER_SIZE		4096

struct VuPs4HttpRequest : public VuRefObj
{
public:
	VuPs4HttpRequest() :	mPollHandle(VUNULL), mConnectionId(-1), mRequestId(-1), 
							mStatus(VuHttpClient::STATUS_READY), mRequestTimeout(10000),
							mResponseLength(HTTP_RECEIVE_BUFFER_SIZE), mState(STATE_SEND),
							mpData(VUNULL), mDataSize(0) {}

	std::string				mURL;
	std::string				mResponse;
	std::list<std::pair<std::string, std::string>> mHeaderTuples;

	SceHttpEpollHandle		mPollHandle;
	int						mTemplateId;
	int						mConnectionId;
	int						mRequestId;
	VuHttpClient::eStatus	mStatus;
	int						mRequestTimeout;
	int						mResponseLength;
	int						mState;
	VUUINT8*				mpData;
	int						mDataSize;
};

class VuPs4HttpClient : public VuHttpClient
{
public:
		VuPs4HttpClient() : mTemplateGetId(-1), mTemplatePostId(-1) {}

protected:
	// created by engine
	friend class VuEngine;

	// Build templates and prepare the module for http work
	virtual bool		init();
	
	// Called when a request is successful or aborted to clean up system resources
	void				requestCompleted(VuPs4HttpRequest* pRequest);

	// Poll the status of various requests and service as necessary
	void				tickNetwork(float fdt);

public:
	virtual VUHANDLE	createRequest();
	virtual void		releaseRequest(VUHANDLE hRequest);
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, const char *value);
	virtual void		setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS);
	virtual eStatus		getStatus(VUHANDLE hRequest);
	virtual const std::string	&getResponse(VUHANDLE hRequest);
	
protected:
	virtual void		sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size);

private:

	// Request type for PS4
	typedef std::list<VuPs4HttpRequest *> Requests;
	
	// Queue of requests for tick-time processing
	Requests			mRequests;

	int					mTemplateGetId;		// GET command template
	int					mTemplatePostId;	// POST command template
};
