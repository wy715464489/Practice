//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Fluids Mesh Asset class
// 
//*****************************************************************************

#include "VuTimedEventAsset.h"
#include "VuAssetUtil.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"

#define MAX_PARAMS_SIZE (4*1024)

IMPLEMENT_RTTI(VuTimedEventAsset, VuAsset);
IMPLEMENT_ASSET_REGISTRATION(VuTimedEventAsset);


//*****************************************************************************
static bool CompareEvents(const VuTimedEventAsset::VuEvent &event1, const VuTimedEventAsset::VuEvent &event2)
{
	return event1.mTime < event2.mTime;
}

//*****************************************************************************
void VuTimedEventAsset::schema(const VuJsonContainer &creationInfo, VuJsonContainer &schema)
{
	schema["DefaultPath"].putValue("TimedEvents");

	VuAssetUtil::addFileProperty(schema, "File", "json");
}

//*****************************************************************************
bool VuTimedEventAsset::bake(const VuJsonContainer &creationInfo, VuAssetBakeParams &bakeParams)
{
	VuBinaryDataWriter &writer = bakeParams.mWriter;

	const std::string &fileName = creationInfo["File"].asString();

	// load json document
	VuJsonContainer doc;
	VuJsonReader reader;
	if ( !reader.loadFromFile(doc, VuFile::IF()->getRootPath() + fileName) )
		return VUWARNING("Unable to load fluids mesh asset %s: %s", fileName.c_str(), reader.getLastError().c_str());

	Events events;
	events.resize(doc.size());
	for ( int i = 0; i < doc.size(); i++ )
	{
		events[i].mTime = doc[i]["Time"].asFloat();
		events[i].mType = doc[i]["Type"].asString();
		events[i].mParams = doc[i]["Params"];
	}

	// sort based on time
	std::sort(events.begin(), events.end(), CompareEvents);

	// write count
	writer.writeValue((int)events.size());

	VuJsonBinaryWriter jsonWriter;

	for ( int i = 0; i < (int)events.size(); i++ )
	{
		VuEvent *pEvent = &events[i];

		// write time
		writer.writeValue(pEvent->mTime);

		// write type
		writer.writeString(pEvent->mType);

		// write params
		VUBYTE tempParams[MAX_PARAMS_SIZE];
		int paramsSize = sizeof(tempParams);
		if ( !jsonWriter.saveToMemory(pEvent->mParams, tempParams, paramsSize) )
			return false;
		writer.writeValue(paramsSize);
		writer.writeData(tempParams, paramsSize);
	}
	
	return true;
}

//*****************************************************************************
bool VuTimedEventAsset::load(VuBinaryDataReader &reader)
{
	VuJsonBinaryReader jsonReader;

	// read count
	int eventCount;
	reader.readValue(eventCount);
	mEvents.resize(eventCount);

	for ( int i = 0; i < (int)mEvents.size(); i++ )
	{
		VuEvent *pEvent = &mEvents[i];

		// read time
		reader.readValue(pEvent->mTime);

		// read type
		reader.readString(pEvent->mType);

		// read params using scratch pad
		VUBYTE tempParams[MAX_PARAMS_SIZE];
		int paramsSize;
		reader.readValue(paramsSize);
		reader.readData(tempParams, paramsSize);
		if ( !jsonReader.loadFromMemory(pEvent->mParams, tempParams, paramsSize) )
			return false;
	}

	return true;
}

//*****************************************************************************
void VuTimedEventAsset::unload()
{
	mEvents.clear();
}
