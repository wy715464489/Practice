//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Project class
// 
//*****************************************************************************

#include "VuProject.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonWriter.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/VuEntityFactory.h"
#include "VuEngine/Entities/VuEntityRepository.h"
#include "VuEngine/Assets/VuProjectAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuDataUtil.h"


//*****************************************************************************
void VuProject::destroy()
{
	if ( mpRootEntity )
	{
		mpRootEntity->removeRef();
		mpRootEntity = VUNULL;
	}
}

//*****************************************************************************
bool VuProject::create(const std::string &strType, const std::string &strName)
{
	destroy();

	mpRootEntity = VuEntityFactory::IF()->createEntity(strType);
	if ( !mpRootEntity )
		return false;

	mpRootEntity->setShortName(strName);

	return true;
}

//*****************************************************************************
bool VuProject::load(const VuProjectAsset *pProjectAsset)
{
	return load(pProjectAsset->getProject(), pProjectAsset->getProjectName());
}

//*****************************************************************************
bool VuProject::load(const std::string &strFileName)
{
	VuJsonContainer document;

	// load file
	VuJsonReader reader;
	if ( !reader.loadFromFile(document, strFileName) )
		return VUWARNING(reader.getLastError().c_str());

	if ( !load(document, strFileName) )
		return false;

	// load companion file (if it exists)
	reader.loadFromFile(mEditorData, strFileName + ".user");

	return true;
}

//*****************************************************************************
bool VuProject::save(const std::string &strFileName) const
{
	VuJsonContainer document;
	if ( !save(document) )
		return false;

	// save file
	VuJsonWriter writer;
	writer.configTrailingCommas(true);
	if ( !writer.saveToFile(document, strFileName) )
		return VUWARNING("Unable to write project to file '%s'.", strFileName.c_str());

	return true;
}

//*****************************************************************************
void VuProject::saveEditorData(const std::string &strFileName) const
{
	VuJsonWriter writer;
	writer.saveToFile(mEditorData, strFileName + ".user");
}

//*****************************************************************************
void VuProject::bake()
{
	mpRootEntity->bake();

	mAssetData.clear();

	// save project
	VuJsonContainer data;
	if ( !save(data) )
		VUWARNING("Unable to bake projet %s", getName().c_str());

	// add asset factory listener
	class AssetCollector : public VuAssetFactory::VuListener
	{
	public:
		virtual void onCreateAsset(VuAsset *pAsset)
		{
			addAsset(pAsset);
		}
		void addAsset(VuAsset *pAsset)
		{
			mAssetMap[pAsset->getType()].insert(pAsset->getAssetName());
			for ( int i = 0; i < pAsset->childAssets().size(); i++ )
			{
				if ( VuAsset *pChildAsset = VuAssetFactory::IF()->findAsset(pAsset->childAssets()[i]) )
					addAsset(pChildAsset);
			}
		}
		typedef std::set<std::string> AssetSet;
		typedef std::map<std::string, AssetSet> AssetMap;
		AssetMap	mAssetMap;
	};
	AssetCollector assetCollector;
	VuAssetFactory::IF()->addListener(&assetCollector);

	VuProject *pProject = new VuProject;

	if ( !pProject->load(data, getName()) )
		VUWARNING("Unable to bake projet %s", getName().c_str());

	pProject->removeRef();

	VuAssetFactory::IF()->removeListener(&assetCollector);

	// copy asset collector data to sortable container
	typedef std::set<std::string> AssetNames;
	typedef std::pair<std::string, AssetNames> AssetType;
	std::deque<AssetType> assetTypes;
	for ( const auto &iter : assetCollector.mAssetMap )
		assetTypes.push_back(iter);

	// sort
	std::sort(assetTypes.begin(), assetTypes.end(), [](const AssetType &type0, const AssetType &type1)
	{
		int pri0 = VuAssetFactory::IF()->getAssetTypePriority(type0.first);
		int pri1 = VuAssetFactory::IF()->getAssetTypePriority(type1.first);
		return pri0 < pri1;
	});

	// store in json container
	for ( const auto &iter : assetTypes )
	{
		VuJsonContainer &assetType = mAssetData.append();
		assetType.append().putValue(iter.first);
		for ( const auto &assetName : iter.second )
			assetType.append().putValue(assetName);
	}
}

//*****************************************************************************
void VuProject::clearBaked()
{
	mpRootEntity->clearBaked();
}

//*****************************************************************************
void VuProject::gameInitialize()
{
	if ( !mpRootEntity->isGameInitialized() )
	{
		VuEntityRepository::IF()->addProject(this);

		mpRootEntity->gameInitialize();

		mpRootEntity->handleEventRecursive("OnProjectInitialized");
	}
}

//*****************************************************************************
void VuProject::gameRelease()
{
	if ( mpRootEntity->isGameInitialized() )
	{
		mpRootEntity->handleEventRecursive("OnProjectRelease");

		mpRootEntity->gameRelease();

		VuEntityRepository::IF()->removeProject(this);
	}
}

//*****************************************************************************
void VuProject::gameReset()
{
	if ( !mpRootEntity->isGameInitialized() )
	{
		mpRootEntity->gameReset();
	}
}

//*****************************************************************************
VuEntity *VuProject::findEntity(const std::string &strLongName) const
{
	return mpRootEntity->findEntity(strLongName);
}

//*****************************************************************************
const std::string &VuProject::getName() const
{
	return mpRootEntity->getShortName();
}

//*****************************************************************************
bool VuProject::load(const VuJsonContainer &data, const std::string &strFileName)
{
	// file name determines project name
	std::string strProjectName = VuFileUtil::getName(strFileName);

	// create entity
	std::string strType = data["RootEntity"]["type"].asString();
	if ( strType.empty() )
		strType = "VuContainerEntity";
	if ( !create(strType, strProjectName) )
		return VUWARNING("Unable to create project root entity of type '%s'.", strType.c_str());

	// load entity
	mpRootEntity->load(data["RootEntity"]["data"]);

	// post-load
	mpRootEntity->postLoad();

	// load asset data
	mAssetData = data["AssetData"];

	return true;
}

//*****************************************************************************
bool VuProject::save(VuJsonContainer &data) const
{
	if ( !mpRootEntity )
		return false;

	// save scene
	data["RootEntity"]["type"].putValue(mpRootEntity->getCreationType());
	mpRootEntity->save(data["RootEntity"]["data"]);

	data["AssetData"] = mAssetData;

	// clean dangling empty arrays, objects, and null containers
	cleanSaveDataRecursive(data);

	return true;
}

//*****************************************************************************
void VuProject::cleanSaveDataRecursive(VuJsonContainer &data) const
{
	if ( data.isArray() )
	{
		for ( int i = 0; i < data.size(); i++ )
			cleanSaveDataRecursive(data[i]);

		if ( data.size() == 0 )
			data.clear();
	}
	else if ( data.isObject() )
	{
		for ( int i = 0; i < data.numMembers(); i++ )
		{
			const std::string &key = data.getMemberKey(i);
			cleanSaveDataRecursive(data[key]);
			if ( data[key].isNull() )
			{
				data.removeMember(key);
				i--;
			}
		}

		if ( data.numMembers() == 0 )
			data.clear();
	}
}
