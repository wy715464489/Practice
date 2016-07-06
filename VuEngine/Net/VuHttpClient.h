//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  HttpClient class
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"


class VuHttpClient : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuHttpClient)

protected:
	// created by engine
	friend class VuEngine;
	virtual bool init() { return true; }

public:
	virtual VUHANDLE	createRequest() = 0;
	virtual void		releaseRequest(VUHANDLE hRequest) = 0;
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, const char *value) = 0;
	virtual void		setContentHeader(VUHANDLE hRequest, const char *key, int value);
	virtual void		setRequestTimeoutMS(VUHANDLE hRequest, int timeoutMS) = 0;

	virtual void		getAsync(VUHANDLE hRequest, const char *url);
	virtual void		postAsync(VUHANDLE hRequest, const char *url);
	virtual void		postAsync(VUHANDLE hRequest, const char *url, const char *data);
	virtual void		postAsync(VUHANDLE hRequest, const char *url, const std::string &data);
	virtual void		postAsync(VUHANDLE hRequest, const char *url, const void *data, int size);

	enum eStatus { STATUS_READY, STATUS_WAITING_FOR_RESPONSE, STATUS_RESPONSE_RECEIVED, STATUS_ERROR };
	virtual eStatus		getStatus(VUHANDLE hRequest) = 0;

	virtual const std::string	&getResponse(VUHANDLE hRequest) = 0;

protected:
	enum eMethod { METHOD_GET, METHOD_POST };
	virtual void		sendRequest(VUHANDLE hRequest, eMethod method, const char *url, const void *data, int size) = 0;
};
