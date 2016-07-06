//*****************************************************************************
//
//  Copyright (c) 2015-2015 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuNearbyConnectionManager
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Events/VuEventMap.h"


class VuNearbyConnectionManager : public VuSystemComponent
{
	DECLARE_SYSTEM_COMPONENT(VuNearbyConnectionManager)
	DECLARE_EVENT_MAP

public:
	VuNearbyConnectionManager();
	~VuNearbyConnectionManager();

	virtual bool	init() { return true; }

	void			startAdvertising();
	void			startDiscovery();
	void			sendConnectionRequest(const char *endpointId);
	void			reset();
	void			sendMessage(const char *endpointId, void *pData, int dataSize);

	struct Endpoint
	{
		std::string	mId;
		std::string	mName;
	};
	typedef std::map<std::string, Endpoint> Endpoints;
	const Endpoints	&getDiscoveredEndpoints() { return mDiscoveredEndpoints; }
	const Endpoints	&getConnectedEndpoints() { return mConnectedEndpoints; }

	class Listener
	{
	public:
		virtual void	onNCConnectionFail(const char *endpointId) {}
		virtual void	onNCConnected(const char *endpointId) {}
		virtual void	onNCDisconnected(const char *endpointId) {}
		virtual void	onNCMessageReceived(const char *endpointId, const void *pData, int dataSize) {}
	};
	void			addListener(Listener *pListener) { mListeners.push_back(pListener); }
	void			removeListener(Listener *pListener) { mListeners.remove(pListener); }

protected:
	virtual void	startAdvertisingNative() {}
	virtual void	startDiscoveryNative() {}
	virtual void	sendConnectionRequestNative(const char *endpointId) {}
	virtual void	resetNative() {}
	virtual void	sendMessageNative(const char *endpointId, void *pData, int dataSize) {}

	// event handlers
	void			OnNearbyConnectionEndpointFound(const VuParams &params);
	void			OnNearbyConnectionEndpointLost(const VuParams &params);
	void			OnNearbyConnectionConnectionResponse(const VuParams &params);
	void			OnNearbyConnectionConnected(const VuParams &params);
	void			OnNearbyConnectionDisconnected(const VuParams &params);
	void			OnNearbyConnectionMessageReceived(const VuParams &params);

	Endpoints		mDiscoveredEndpoints;
	Endpoints		mConnectedEndpoints;

	typedef std::list<Listener *> Listeners;
	Listeners		mListeners;
};


// Engine messages

struct VuNCMobileControllerStateMsg
{
	VuNCMobileControllerStateMsg() : mSignature(smSignature), mButtons(0), mDeviceTilt(0.0f) {}

	static bool	validate(const void *pData, int dataSize) { return dataSize >= sizeof(VuNCMobileControllerStateMsg) && memcmp(pData, &smSignature, 4) == 0; }

	static const VUUINT32 smSignature;

	VUUINT32	mSignature;

	VUUINT32	mButtons;
	float		mDeviceTilt;
};

struct VuNCPlayVibrationMsg
{
	VuNCPlayVibrationMsg() : mSignature(smSignature) {}

	static bool	validate(const void *pData, int dataSize) { return dataSize >= sizeof(VuNCPlayVibrationMsg) && memcmp(pData, &smSignature, 4) == 0; }

	static const VUUINT32 smSignature;

	VUUINT32	mSignature;

	int			mEffect;
};
