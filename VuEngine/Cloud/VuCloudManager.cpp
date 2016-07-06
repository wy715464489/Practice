//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Cloud Manager
// 
//*****************************************************************************

#include "VuCloudManager.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Net/VuHttpClient.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/Managers/VuProfileManager.h"
#include "VuEngine/Events/VuEventManager.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonWriter.h"
#include "VuEngine/Util/VuDataUtil.h"
#include "VuEngine/Util/VuBase64.h"
#include "VuEngine/Util/VuZLibUtil.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuCloudManager, VuCloudManager);

#define CLOUD_VERSION 1


//*****************************************************************************
VuCloudManager::VuCloudManager():
	mHttpRequest(VUNULL),
	mIdentitySaved(false),
	mNewsUnread(false)
{
	// event handlers
	REG_EVENT_HANDLER(VuCloudManager, OnSaveProfile);
}

//*****************************************************************************
bool VuCloudManager::init()
{
	return true;
}

//*****************************************************************************
void VuCloudManager::postInit()
{
	loadFromProfile();

	// create FSM
	{
		VuFSM::VuState *pState = mFSM.addState("Identity");
		pState->setEnterMethod(this, &VuCloudManager::onIdentityEnter);
		pState->setExitMethod(this, &VuCloudManager::onIdentityExit);
		pState->setTickMethod(this, &VuCloudManager::onIdentityTick);

		pState = mFSM.addState("News");
		pState->setEnterMethod(this, &VuCloudManager::onNewsEnter);
		pState->setExitMethod(this, &VuCloudManager::onNewsExit);
		pState->setTickMethod(this, &VuCloudManager::onNewsTick);

		pState = mFSM.addState("NewsData");
		pState->setEnterMethod(this, &VuCloudManager::onNewsDataEnter);
		pState->setExitMethod(this, &VuCloudManager::onNewsDataExit);
		pState->setTickMethod(this, &VuCloudManager::onNewsDataTick);

		pState = mFSM.addState("NewsTrack");
		pState->setEnterMethod(this, &VuCloudManager::onNewsTrackEnter);
		pState->setExitMethod(this, &VuCloudManager::onNewsTrackExit);
		pState->setTickMethod(this, &VuCloudManager::onNewsTrackTick);

		pState = mFSM.addState("Idle");

		// transitions
		mFSM.addTransition("Identity", "News", "IdentityFinished");

		mFSM.addTransition("News", "NewsData", "NewsReceived");
		mFSM.addTransition("News", "Idle", "NewsFinished");

		mFSM.addTransition("NewsData", "Idle", "NewsFinished");

		mFSM.addTransition("NewsTrack", "Idle", "NewsTrackFinished");

		mFSM.addTransition("Idle", "NewsTrack", "NewsLaunched");
	}

	mFSM.begin();

	VuTickManager::IF()->registerHandler(this, &VuCloudManager::tick, "Final");
}

//*****************************************************************************
void VuCloudManager::release()
{
	VuTickManager::IF()->unregisterHandlers(this);

	mFSM.end();
}

//*****************************************************************************
bool VuCloudManager::isNewsAvailable()
{
	return VuProfileManager::IF()->dataRead()["Cloud"]["News"]["Data"].hasMember("Base64");
}

//*****************************************************************************
bool VuCloudManager::isNewsInterstitial()
{
	return VuProfileManager::IF()->dataRead()["Cloud"]["News"]["Interstitial"].asBool();
}

//*****************************************************************************
void VuCloudManager::getNewsData(VuArray<VUBYTE> &data)
{
	VuDataUtil::getValue(VuProfileManager::IF()->dataRead()["Cloud"]["News"]["Data"], data);
	mNewsUnread = false;
}

//*****************************************************************************
void VuCloudManager::onNewsLaunched()
{
	mFSM.setCondition("NewsLaunched", true);
}

//*****************************************************************************
void VuCloudManager::loadFromProfile()
{
	const VuJsonContainer &data = VuProfileManager::IF()->dataRead()["Cloud"];

	mIdentitySaved = data["IdentitySaved"].asBool();
	mNewsUnread = data["News"]["Unread"].asBool();
}

//*****************************************************************************
void VuCloudManager::saveToProfile()
{
	VuJsonContainer &data = VuProfileManager::IF()->dataWrite()["Cloud"];

	data["IdentitySaved"].putValue(mIdentitySaved);
	data["News"]["Unread"].putValue(mNewsUnread);
}

//*****************************************************************************
void VuCloudManager::tick(float fdt)
{
	mFSM.evaluate();
	mFSM.tick(fdt);
}

//*****************************************************************************
void VuCloudManager::onIdentityEnter()
{
	const char *userIdentifier = VuSys::IF()->getUserIdentifier();

	if ( !mIdentitySaved && (strlen(userIdentifier) > 0) )
	{
		// create http request
		mHttpRequest = VuHttpClient::IF()->createRequest();

		// URL
		char url[256] = "https://vectorunit-cloud.appspot.com/identity";

		// data
		VuJsonContainer data;
		data["Game"].putValue(VuEngine::IF()->options().mGameName);
		data["Platform"].putValue(VUPLATFORM);
		data["Sku"].putValue(VuAssetFactory::IF()->getSku());
		data["UserID"].putValue(userIdentifier);

		std::string strData;
		VuJsonWriter writer;
		writer.configCompact(true);
		writer.saveToString(data, strData);

		// header
		VuHttpClient::IF()->setContentHeader(mHttpRequest, "Content-Type", "application/json");
		VuHttpClient::IF()->setContentHeader(mHttpRequest, "Content-Length", (int)strData.size());

		// send request
		VuHttpClient::IF()->postAsync(mHttpRequest, url, strData);
	}
	else
	{
		mFSM.setCondition("IdentityFinished", true);
	}
}

//*****************************************************************************
void VuCloudManager::onIdentityExit()
{
	if ( mHttpRequest )
	{
		VuHttpClient::IF()->releaseRequest(mHttpRequest);
		mHttpRequest = VUNULL;
	}
}

//*****************************************************************************
void VuCloudManager::onIdentityTick(float fdt)
{
	if ( mHttpRequest )
	{
		VuHttpClient::eStatus status = VuHttpClient::IF()->getStatus(mHttpRequest);
		if ( status != VuHttpClient::STATUS_WAITING_FOR_RESPONSE )
		{
//			bool success = false;

			if ( status == VuHttpClient::STATUS_RESPONSE_RECEIVED )
			{
				VuJsonContainer response;
				VuJsonReader jsonReader;
				if ( jsonReader.loadFromString(response, VuHttpClient::IF()->getResponse(mHttpRequest)) )
				{
					response.getValue(mIdentitySaved);
				}
			}

			mFSM.setCondition("IdentityFinished", true);
		}
	}
}

//*****************************************************************************
void VuCloudManager::onNewsEnter()
{
	// create http request
	mHttpRequest = VuHttpClient::IF()->createRequest();

	// URL
	char url[256] = "http://vectorunit-cloud.appspot.com/news";

	// data
	VuJsonContainer data;
	data["Game"].putValue(VuEngine::IF()->options().mGameName);
	data["Platform"].putValue(VUPLATFORM);
	data["Sku"].putValue(VuAssetFactory::IF()->getSku());

	std::string strData;
	VuJsonWriter writer;
	writer.configCompact(true);
	writer.saveToString(data, strData);

	// header
	VuHttpClient::IF()->setContentHeader(mHttpRequest, "Content-Type", "application/json");
	VuHttpClient::IF()->setContentHeader(mHttpRequest, "Content-Length", (int)strData.size());

	// send request
	VuHttpClient::IF()->postAsync(mHttpRequest, url, strData);
}

//*****************************************************************************
void VuCloudManager::onNewsExit()
{
	if ( mHttpRequest )
	{
		VuHttpClient::IF()->releaseRequest(mHttpRequest);
		mHttpRequest = VUNULL;
	}
}

//*****************************************************************************
void VuCloudManager::onNewsTick(float fdt)
{
	if ( mHttpRequest )
	{
		VuHttpClient::eStatus status = VuHttpClient::IF()->getStatus(mHttpRequest);
		if ( status != VuHttpClient::STATUS_WAITING_FOR_RESPONSE )
		{
			bool success = false;

			if ( status == VuHttpClient::STATUS_RESPONSE_RECEIVED )
			{
				VuJsonReader jsonReader;
				if ( jsonReader.loadFromString(mNewsResponse, VuHttpClient::IF()->getResponse(mHttpRequest)) )
				{
					if ( VuProfileManager::IF()->dataRead()["Cloud"]["News"]["Campaign"].asString() != mNewsResponse["Campaign"].asString() )
						success = true;
				}
			}

			if ( success )
				mFSM.setCondition("NewsReceived", true);
			else
				mFSM.setCondition("NewsFinished", true);
		}
	}
	else
	{
		mFSM.setCondition("NewsFinished", true);
	}
}

//*****************************************************************************
void VuCloudManager::onNewsDataEnter()
{
	const std::string &url = mNewsResponse["Url"].asString();
	if ( url.length() )
	{
		mHttpRequest = VuHttpClient::IF()->createRequest();
		VuHttpClient::IF()->getAsync(mHttpRequest, url.c_str());
	}
	else
	{
		mFSM.setCondition("NewsFinished", true);
	}
}

//*****************************************************************************
void VuCloudManager::onNewsDataExit()
{
	if ( mHttpRequest )
	{
		VuHttpClient::IF()->releaseRequest(mHttpRequest);
		mHttpRequest = VUNULL;
	}
}

//*****************************************************************************
void VuCloudManager::onNewsDataTick(float fdt)
{
	if ( mHttpRequest )
	{
		VuHttpClient::eStatus status = VuHttpClient::IF()->getStatus(mHttpRequest);
		if ( status != VuHttpClient::STATUS_WAITING_FOR_RESPONSE )
		{
			if ( status == VuHttpClient::STATUS_RESPONSE_RECEIVED )
			{
				const std::string &response = VuHttpClient::IF()->getResponse(mHttpRequest);
				if ( response.size() )
				{
					VuArray<VUBYTE> data(0);
					data.resize((int)response.size());
					VU_MEMCPY(&data[0], data.size(), &response[0], response.size());

					if ( validateNewsData(data) )
					{
						VuDataUtil::putValue(VuProfileManager::IF()->dataWrite()["Cloud"]["News"]["Data"], data);
						VuProfileManager::IF()->dataWrite()["Cloud"]["News"]["Campaign"] = mNewsResponse["Campaign"];
						VuProfileManager::IF()->dataWrite()["Cloud"]["News"]["Interstitial"] = mNewsResponse["Interstitial"];
						mNewsUnread = true;

						VuProfileManager::IF()->save();

						VuEventManager::IF()->broadcast("OnNewsReceived");
					}
					else
					{
						VUPRINTF("Unable to validate news data: %s\n", response.c_str());
					}
				}
			}

			mFSM.setCondition("NewsFinished", true);
		}
	}
	else
	{
		mFSM.setCondition("NewsFinished", true);
	}
}

//*****************************************************************************
void VuCloudManager::onNewsTrackEnter()
{
	// create http request
	mHttpRequest = VuHttpClient::IF()->createRequest();

	// URL
	char url[256] = "https://vectorunit-cloud.appspot.com/news_track";

	// data
	VuJsonContainer data;
	data["Game"].putValue(VuEngine::IF()->options().mGameName);
	data["Platform"].putValue(VUPLATFORM);
	data["Sku"].putValue(VuAssetFactory::IF()->getSku());
	data["Campaign"].putValue(VuProfileManager::IF()->dataRead()["Cloud"]["News"]["Campaign"].asString());
	data["UserID"].putValue(VuSys::IF()->getUserIdentifier());

	std::string strData;
	VuJsonWriter writer;
	writer.configCompact(true);
	writer.saveToString(data, strData);

	// header
	VuHttpClient::IF()->setContentHeader(mHttpRequest, "Content-Type", "application/json");
	VuHttpClient::IF()->setContentHeader(mHttpRequest, "Content-Length", (int)strData.size());

	// send request
	VuHttpClient::IF()->postAsync(mHttpRequest, url, strData);
}

//*****************************************************************************
void VuCloudManager::onNewsTrackExit()
{
	VuHttpClient::IF()->releaseRequest(mHttpRequest);
	mHttpRequest = VUNULL;

	mFSM.setCondition("NewsLaunched", false);
	mFSM.setCondition("NewsTrackFinished", false);
}

//*****************************************************************************
void VuCloudManager::onNewsTrackTick(float fdt)
{
	VuHttpClient::eStatus status = VuHttpClient::IF()->getStatus(mHttpRequest);
	if ( status != VuHttpClient::STATUS_WAITING_FOR_RESPONSE )
	{
		mFSM.setCondition("NewsTrackFinished", true);
	}
}

//*****************************************************************************
bool VuCloudManager::validateNewsData(const VuArray<VUBYTE> &newsData)
{
	if ( newsData.size() )
	{
		VUUINT32 size = VuScratchPad::SIZE - 1;
		if ( VuZLibUtil::gzipUncompressFromMemory(VuScratchPad::get(), &size, &newsData[0], newsData.size()) )
		{
			char *strDoc = static_cast<char*>(VuScratchPad::get());
			strDoc[size] = '\0';

			// check for and skip UTF-8 header
			if ( (VUUINT8)strDoc[0] == 0xef && (VUUINT8)strDoc[1] == 0xbb && (VUUINT8)strDoc[2] == 0xbf )
				strDoc += 3;

			VuJsonContainer doc;
			VuJsonReader reader;
			if ( reader.loadFromString(doc, strDoc) )
			{
				return true;
			}
		}
	}

	return false;
}
