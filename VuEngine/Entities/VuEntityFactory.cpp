//*****************************************************************************
//
//  Copyright (c) 2006-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Factory class
// 
//*****************************************************************************

#include "VuEntityFactory.h"
#include "VuEntity.h"
#include "VuEntityUtil.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuTemplateAsset.h"
#include "VuEngine/Assets/VuDBAsset.h"


// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuEntityFactory, VuEntityFactory);


// static prototypes
static VuEntity *CreateTemplatedEntity(const char *strType);


//*****************************************************************************
bool VuEntityFactory::init()
{
	mTypes.reserve(1024);

	// register templates
	registerTemplates();

	// load entity factory DB
	VuDBAsset *pDBAsset = VuAssetFactory::IF()->createAsset<VuDBAsset>("EntityFactoryDB", VuAssetFactory::OPTIONAL_ASSET);
	if ( pDBAsset )
	{
		const VuJsonContainer &db = pDBAsset->getDB();
		for ( int i = 0; i < db.numMembers(); i++ )
		{
			const std::string &path = db.getMemberKey(i);
			const VuJsonContainer &entries = db[path];
			for ( int j = 0; j < entries.size(); j++ )
			{
				const VuJsonContainer &entryData = entries[j];

				mTypes.resize(mTypes.size() + 1);
				VuTypeInfo &entry = mTypes.back();

				entry.mType = entryData[0].asString();
				entry.mPath = path;
				entry.mShortType = entryData[1].asString();
				entry.mCreateFn = VUNULL;

				// check for duplicate registration
				VUUINT32 hashedType = VuHash::fnv32String(entry.mType.c_str());
				if ( mLookup.find(hashedType) != mLookup.end() )
				{
					VUPRINTF("Duplicate entity type found in EntityFactoryDB: %s\n", entry.mType.c_str());
					VUASSERT(0, "Duplicate entity type found in EntityFactoryDB");
				}

				mLookup[hashedType] = (int)mTypes.size() - 1;
			}
		}
	}
	VuAssetFactory::IF()->releaseAsset(pDBAsset);

	return true;
}

//*****************************************************************************
void VuEntityFactory::registerEntity(const std::string &strType, VuEntity *(*createFn)(const char *strType))
{
	VUUINT32 hashedType = VuHash::fnv32String(strType.c_str());

	Lookup::iterator itLookup = mLookup.find(hashedType);
	if ( itLookup != mLookup.end() )
	{
		VuTypeInfo &typeInfo = mTypes[itLookup->second];

		// check for duplicate registration
		VUASSERT(typeInfo.mCreateFn == VUNULL, "Duplicate entity registration");

		typeInfo.mCreateFn = createFn;
	}
}

//*****************************************************************************
void VuEntityFactory::finalizeRegistration()
{
	// sort entity types
	mOrder.resize(mTypes.size());
	for ( int i = 0; i < (int)mOrder.size(); i++ )
		mOrder[i] = i;

	std::sort(mOrder.begin(), mOrder.end(), [this](int i0, int i1)
	{
		const VuTypeInfo &info0 = mTypes[i0];
		const VuTypeInfo &info1 = mTypes[i1];

		if ( info0.mPath.length() > 0 && info1.mPath.length() == 0 )
			return true;
		if ( info0.mPath.length() == 0 && info1.mPath.length() > 0 )
			return false;

		if ( info0.mPath != info1.mPath )
			return info0.mPath < info1.mPath;

		return info0.mType < info1.mType;
	});
}

//*****************************************************************************
VuEntity *VuEntityFactory::createEntity(const std::string &strType)
{
	VuEntity *pEntity = VUNULL;

	VUUINT32 hashedType = VuHash::fnv32String(strType.c_str());
	Lookup::iterator itLookup = mLookup.find(hashedType);
	if ( itLookup != mLookup.end() )
	{
		const VuTypeInfo &typeInfo = mTypes[itLookup->second];
		if ( typeInfo.mCreateFn )
			pEntity = typeInfo.mCreateFn(strType.c_str());
		else
			VUPRINTF("Entity type not registered: %s\n", strType.c_str());
	}

	return pEntity;
}


//*****************************************************************************
const std::string &VuEntityFactory::getShortType(const std::string &strType)
{
	VUUINT32 hashedType = VuHash::fnv32String(strType.c_str());

	Lookup::iterator itLookup = mLookup.find(hashedType);
	if ( itLookup != mLookup.end() )
		return mTypes[itLookup->second].mShortType;

	static std::string sUnknown("n/a");
	return sUnknown;
}

//*****************************************************************************
void VuEntityFactory::registerTemplates()
{
	const VuAssetFactory::AssetNames &templateNames = VuAssetFactory::IF()->getAssetNames<VuTemplateAsset>();
	for ( int i = 0; i < (int)templateNames.size(); i++ )
	{
		mTypes.resize(mTypes.size() + 1);
		VuTypeInfo &entry = mTypes.back();

		entry.mType = std::string("#") + templateNames[i];
		entry.mPath = VuEntityUtil::getPath(templateNames[i]);
		if ( entry.mPath.length() )
			entry.mPath = "Templates/" + entry.mPath;
		else
			entry.mPath = "Templates";
		entry.mShortType = VuEntityUtil::getName(templateNames[i]);
		entry.mCreateFn = CreateTemplatedEntity;

		// check for duplicate registration
		VUUINT32 hashedType = VuHash::fnv32String(entry.mType.c_str());
		VUASSERT(mLookup.find(hashedType) == mLookup.end(), "Duplicate template registration");

		mLookup[hashedType] = (int)mTypes.size() - 1;
	}
}

//*****************************************************************************
static VuEntity *CreateTemplatedEntity(const char *strType)
{
	VUASSERT(strType[0] == '#', "CreateTemplatedEntity() invalid type");

	VuEntity *pEntity = VUNULL;

	// create entity and set template asset
	if ( VuTemplateAsset *pTemplateAsset = VuAssetFactory::IF()->createAsset<VuTemplateAsset>(&strType[1]) )
		if ( (pEntity = VuEntityFactory::IF()->createEntity(pTemplateAsset->getEntityType())) != VUNULL)
			pEntity->applyTemplate(pTemplateAsset);

	return pEntity;
}
