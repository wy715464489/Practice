//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuNearbyConnectionManager
// 
//*****************************************************************************

#include "VuNearbyConnectionManager.h"
#include "VuEngine/Util/VuHash.h"


// Engine messages
const VUUINT32 VuNCMobileControllerStateMsg::smSignature = VuHash::fnv32String("VuNCMobileControllerStateMsg_v1"); // type & version
const VUUINT32 VuNCPlayVibrationMsg::smSignature = VuHash::fnv32String("VuNCPlayVibrationMsg_v1"); // type & version


//*****************************************************************************
VuNearbyConnectionManager::VuNearbyConnectionManager()
{
	// event handlers
	REG_EVENT_HANDLER(VuNearbyConnectionManager, OnNearbyConnectionEndpointFound);
	REG_EVENT_HANDLER(VuNearbyConnectionManager, OnNearbyConnectionEndpointLost);
	REG_EVENT_HANDLER(VuNearbyConnectionManager, OnNearbyConnectionConnectionResponse);
	REG_EVENT_HANDLER(VuNearbyConnectionManager, OnNearbyConnectionConnected);
	REG_EVENT_HANDLER(VuNearbyConnectionManager, OnNearbyConnectionDisconnected);
	REG_EVENT_HANDLER(VuNearbyConnectionManager, OnNearbyConnectionMessageReceived);
}

//*****************************************************************************
VuNearbyConnectionManager::~VuNearbyConnectionManager()
{
	VUASSERT(mListeners.size() == 0, "VuNearbyConnectionManager listener leak");
}

//*****************************************************************************
void VuNearbyConnectionManager::startAdvertising()
{
	mDiscoveredEndpoints.clear();
	mConnectedEndpoints.clear();
	startAdvertisingNative();
}

//*****************************************************************************
void VuNearbyConnectionManager::startDiscovery()
{
	mDiscoveredEndpoints.clear();
	mConnectedEndpoints.clear();
	startDiscoveryNative();
}

//*****************************************************************************
void VuNearbyConnectionManager::sendConnectionRequest(const char *endpointId)
{
	sendConnectionRequestNative(endpointId);
}

//*****************************************************************************
void VuNearbyConnectionManager::reset()
{
	resetNative();
}

//*****************************************************************************
void VuNearbyConnectionManager::sendMessage(const char *endpointId, void *pData, int dataSize)
{
	sendMessageNative(endpointId, pData, dataSize);
}

//*****************************************************************************
void VuNearbyConnectionManager::OnNearbyConnectionEndpointFound(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	const char *endpointId = accessor.getString();
	const char *name = accessor.getString();

	Endpoint &endpoint = mDiscoveredEndpoints[endpointId];

	endpoint.mId = endpointId;
	endpoint.mName = name;
}

//*****************************************************************************
void VuNearbyConnectionManager::OnNearbyConnectionEndpointLost(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	const char *endpointId = accessor.getString();

	Endpoints::iterator iter = mDiscoveredEndpoints.find(endpointId);
	if ( iter != mDiscoveredEndpoints.end() )
		mDiscoveredEndpoints.erase(iter);
}

//*****************************************************************************
void VuNearbyConnectionManager::OnNearbyConnectionConnectionResponse(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	const char *endpointId = accessor.getString();
	bool success = accessor.getBool();

	if ( success )
	{
		mConnectedEndpoints[endpointId] = mDiscoveredEndpoints[endpointId];
		for ( auto &listener : mListeners )
			listener->onNCConnected(endpointId);
	}
	else
	{
		for ( auto &listener : mListeners )
			listener->onNCConnectionFail(endpointId);
	}
}

//*****************************************************************************
void VuNearbyConnectionManager::OnNearbyConnectionConnected(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	const char *endpointId = accessor.getString();
	const char *name = accessor.getString();

	Endpoint &endpoint = mConnectedEndpoints[endpointId];

	endpoint.mId = endpointId;
	endpoint.mName = name;

	for ( auto &listener : mListeners )
		listener->onNCConnected(endpointId);
}

//*****************************************************************************
void VuNearbyConnectionManager::OnNearbyConnectionDisconnected(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	const char *endpointId = accessor.getString();

	for ( auto &listener : mListeners )
		listener->onNCDisconnected(endpointId);

	Endpoints::iterator iter = mConnectedEndpoints.find(endpointId);
	if ( iter != mConnectedEndpoints.end() )
		mConnectedEndpoints.erase(iter);
}

//*****************************************************************************
void VuNearbyConnectionManager::OnNearbyConnectionMessageReceived(const VuParams &params)
{
	VuParams::VuAccessor accessor(params);
	const char *endpointId = accessor.getString();
	void *payloadPtr = accessor.getPointer();
	int payloadSize = accessor.getInt();
	bool freePtr = accessor.getBool();

	for ( auto &listener : mListeners )
		listener->onNCMessageReceived(endpointId, payloadPtr, payloadSize);

	if ( freePtr )
		free(payloadPtr);
}
