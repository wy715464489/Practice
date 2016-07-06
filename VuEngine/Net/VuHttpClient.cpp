//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  HttpClient class
// 
//*****************************************************************************

#include "VuHttpClient.h"


//*****************************************************************************
void VuHttpClient::setContentHeader(VUHANDLE hRequest, const char *key, int value)
{
	char strValue[32];
	VU_SPRINTF(strValue, sizeof(strValue), "%d", value);

	setContentHeader(hRequest, key, strValue);
}

//*****************************************************************************
void VuHttpClient::getAsync(VUHANDLE hRequest, const char *url)
{
	sendRequest(hRequest, METHOD_GET, url, VUNULL, 0);
}

//*****************************************************************************
void VuHttpClient::postAsync(VUHANDLE hRequest, const char *url)
{
	sendRequest(hRequest, METHOD_POST, url, VUNULL, 0);
}

//*****************************************************************************
void VuHttpClient::postAsync(VUHANDLE hRequest, const char *url, const char *data)
{
	sendRequest(hRequest, METHOD_POST, url, data, (int)strlen(data));
}

//*****************************************************************************
void VuHttpClient::postAsync(VUHANDLE hRequest, const char *url, const std::string &data)
{
	sendRequest(hRequest, METHOD_POST, url, &data[0], (int)data.length());
}

//*****************************************************************************
void VuHttpClient::postAsync(VUHANDLE hRequest, const char *url, const void *data, int size)
{
	sendRequest(hRequest, METHOD_POST, url, data, size);
}
