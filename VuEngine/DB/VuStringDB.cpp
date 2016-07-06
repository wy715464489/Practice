//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  String DB class
// 
//*****************************************************************************

#include "VuStringDB.h"
#include "VuEngine/Assets/VuAssetFactory.h"
#include "VuEngine/Assets/VuAssetBakery.h"
#include "VuEngine/Assets/VuStringAsset.h"
#include "VuEngine/HAL/File/VuFile.h"
#include "VuEngine/Util/VuFileUtil.h"
#include "VuEngine/Util/VuWordWrap.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/Json/VuJsonContainer.h"
#include "VuEngine/Json/VuJsonReader.h"
#include "VuEngine/Json/VuJsonWriter.h"
#include "VuEngine/Libs/tinyxml/tinyxml.h"
#include "VuEngine/Dev/VuDevConfig.h"


// internal data
class VuStringDBImpl : public VuStringDB
{
public:
	VuStringDBImpl();

	virtual bool init();

	virtual const std::string	&getString(const std::string &id) const;
	virtual const std::string	&getString(const char *id) const;
	virtual const std::string	&getStringSelf(const std::string &id) const;
	virtual const char			*getStringSelf(const char *id) const;

	virtual const std::string	&getMissingString() const;

	virtual const std::string	&getCurrentLanguageCode() const;
	virtual bool				setCurrentLanguageCode(const std::string &languageCode);
	virtual bool				isCurrentLanguageEastAsian() const { return mbEastAsian; }

	virtual int					getNumLanguages() const;
	virtual const std::string	&getLanguageCode(int index) const;
	virtual bool				doesLanguageExist(const std::string &language) const;

	virtual bool				reload();

	virtual bool				exportToFile(const std::string &absFileName);
	virtual bool				importFromFile(const std::string &absFileName);

private:
	void			consumeRow(std::string &input, std::vector<std::string> &row);
	bool			addStrings(std::string &languageCode);

	typedef std::hash_map<VUUINT32, std::string> StringMap;
	typedef std::vector<std::string> LanguageCodes;

	LanguageCodes	mLanguageCodes;
	StringMap		mStrings;
	std::string		mCurLangCode;
	bool			mbEastAsian;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuStringDB, VuStringDBImpl);


//*****************************************************************************
VuStringDBImpl::VuStringDBImpl():
	mbEastAsian(false)
{
}

//*****************************************************************************
bool VuStringDBImpl::init()
{
	mLanguageCodes = VuAssetFactory::IF()->getAssetNames<VuStringAsset>();

	std::string language = VuSys::IF()->getLanguage();
	if ( VuDevConfig::IF() )
		VuDevConfig::IF()->getParam("Language").getValue(language);
	setCurrentLanguageCode(language);

	return true;
}

//*****************************************************************************
const std::string &VuStringDBImpl::getString(const std::string &id) const
{
	return getString(id.c_str());
}

//*****************************************************************************
const std::string &VuStringDBImpl::getString(const char *id) const
{
	StringMap::const_iterator itString = mStrings.find(VuHash::fnv32String(id));
	if ( itString != mStrings.end() )
		return itString->second;

	return getMissingString();
}

//*****************************************************************************
const std::string &VuStringDBImpl::getStringSelf(const std::string &id) const
{
	StringMap::const_iterator itString = mStrings.find(VuHash::fnv32String(id.c_str()));
	if ( itString != mStrings.end() )
		return itString->second;

	return id;
}

//*****************************************************************************
const char *VuStringDBImpl::getStringSelf(const char *id) const
{
	StringMap::const_iterator itString = mStrings.find(VuHash::fnv32String(id));
	if ( itString != mStrings.end() )
		return itString->second.c_str();

	return id;
}

//*****************************************************************************
const std::string &VuStringDBImpl::getMissingString() const
{
	static const std::string missingString("MISSING_STRING");

	return missingString;
}

//*****************************************************************************
const std::string &VuStringDBImpl::getCurrentLanguageCode() const
{
	return mCurLangCode;
}

//*****************************************************************************
bool VuStringDBImpl::setCurrentLanguageCode(const std::string &languageCode)
{
	if ( languageCode == mCurLangCode )
		return true;

	mCurLangCode = languageCode;
	if ( find(mLanguageCodes.begin(), mLanguageCodes.end(), languageCode) == mLanguageCodes.end() )
		mCurLangCode = "en";

	// east asian?
	const char *aEastAsianLangs[] = { "ja", "ko", "zh-hant", "zh-hans" };
	mbEastAsian = false;
	for ( int i = 0; i < sizeof(aEastAsianLangs)/sizeof(aEastAsianLangs[0]); i++ )
		if ( mCurLangCode == aEastAsianLangs[i] )
			mbEastAsian = true;

	return reload();
}

//*****************************************************************************
int VuStringDBImpl::getNumLanguages() const
{
	return (int)mLanguageCodes.size();
}

//*****************************************************************************
const std::string &VuStringDBImpl::getLanguageCode(int index) const
{
	return mLanguageCodes[index];
}

//*****************************************************************************
bool VuStringDBImpl::doesLanguageExist(const std::string &language) const
{
	for ( LanguageCodes::const_iterator iter = mLanguageCodes.begin(); iter != mLanguageCodes.end(); iter++ )
		if ( language == *iter )
			return true;

	return false;
}

//*****************************************************************************
bool VuStringDBImpl::reload()
{
	mStrings.clear();

	if ( !addStrings(mCurLangCode) )
		return false;

	return true;
}

//*****************************************************************************
bool VuStringDBImpl::exportToFile(const std::string &absFileName)
{
	VUPRINTF("Exporting StringDB to '%s'\n", absFileName.c_str());

	typedef std::map<std::string, std::string> Strings;
	typedef std::map<std::string, Strings> Data;
	Data data;

	// gather data
	for ( int iLang = 0; iLang < getNumLanguages(); iLang++ )
	{
		const std::string &langCode = getLanguageCode(iLang);
		VuStringAsset *pStringAsset = VuAssetFactory::IF()->createAsset<VuStringAsset>(langCode);

		if ( pStringAsset->getDataContainer().hasMember("SkuOverrides") )
			VUPRINTF("WARNING: 'SkuOverrides' not currently supported!!!\n");

		const VuJsonContainer &strings = pStringAsset->getDataContainer()["Strings"];
		for ( int iString = 0; iString < strings.numMembers(); iString++ )
		{
			const std::string &key = strings.getMemberKey(iString);
			const std::string value = strings[key].asString();
			data[key][langCode] = value;
		}

		VuAssetFactory::IF()->releaseAsset(pStringAsset);
	}

	std::string output;

	// first row
	output += "key";
	for ( int iLang = 0; iLang < getNumLanguages(); iLang++ )
	{
		output += "\t";
		output += getLanguageCode(iLang);
	}

	// strings
	for ( Data::iterator itData = data.begin(); itData != data.end(); itData++ )
	{
		output += "\n";
		output += itData->first;
		for ( int iLang = 0; iLang < getNumLanguages(); iLang++ )
		{
			const std::string &langCode = getLanguageCode(iLang);

			output += "\t";
			std::string str = itData->second[langCode];
			output += str;
		}
	}

	// convert to unicode
	std::wstring unicode;
	VuUtf8::convertUtf8StringToWCharString(output.c_str(), unicode);

	// save data
	bool success = false;
	{
		// open file
		if ( VUHANDLE fp = VuFile::IF()->open(absFileName, VuFile::MODE_WRITE) )
		{
			// write UTF-16 header
			VUUINT16 utf16 = 0xfeff;
			VuFile::IF()->write(fp, &utf16, 2);

			// write data
			VuFile::IF()->write(fp, unicode.c_str(), (VUINT)unicode.length()*2);

			// close file
			VuFile::IF()->close(fp);

			success = true;
		}
	}

	if ( !success )
	{
		VUWARNING("Unable to write '%s'.\n", absFileName.c_str());
		return false;
	}

	return true;
}

//*****************************************************************************
bool VuStringDBImpl::importFromFile(const std::string &absFileName)
{
	VUPRINTF("Importing StringDB from '%s'\n", absFileName.c_str());

	typedef std::map<std::string, std::string> Strings;
	typedef std::map<std::string, Strings> Data;
	Data data;

	bool success = false;

	// load data
	VuArray<wchar_t> unicode;
	{
		// open file
		if ( VUHANDLE fp = VuFile::IF()->open(absFileName, VuFile::MODE_READ) )
		{
			// read UTF-16 header
			VUUINT16 utf16;
			if ( VuFile::IF()->read(fp, &utf16, 2) == 2 )
			{
				if ( utf16 == 0xfeff )
				{
					// read data
					unicode.resize(VuFile::IF()->size(fp)/2 - 1);
					VuFile::IF()->read(fp, &unicode.begin(), unicode.size()*2);

					success = true;
				}
			}

			// close file
			VuFile::IF()->close(fp);
		}
	}

	if ( !success )
	{
		VUWARNING("Unable to read '%s'.\n", absFileName.c_str());
		return false;
	}

	// null-terminate unicode data
	unicode.push_back(0);

	// convert to utf-8
	std::string input;
	VuUtf8::appendUnicodeStringToUtf8String(&unicode.begin(), input);

	// parse header
	std::vector<std::string> header;
	consumeRow(input, header);
	if ( header.size() < 2 || header[0] != "key" )
	{
		VUWARNING("Unable to parse header.\n");
		return false;
	}

	// parse rows
	std::vector<std::string> row;
	while ( input.length() )
	{
		consumeRow(input, row);
		if ( row.size() == header.size() )
			for ( int i = 1; i < (int)row.size(); i++ )
				data[row[0]][header[i]] = row[i];
	}

	// write to assets
	for ( int iLang = 0; iLang < getNumLanguages(); iLang++ )
	{
		const std::string &langCode = getLanguageCode(iLang);
		VuStringAsset *pStringAsset = VuAssetFactory::IF()->createAsset<VuStringAsset>(langCode);

		if ( pStringAsset->getDataContainer().hasMember("SkuOverrides") )
			VUPRINTF("WARNING: 'SkuOverrides' not currently supported!!!\n");

		VuJsonContainer langData = pStringAsset->getDataContainer();
		VuJsonContainer &stringData = langData["Strings"];

		for ( Data::const_iterator itData = data.begin(); itData != data.end(); itData++ )
		{
			const std::string &key = itData->first;
			const Strings &strings = itData->second;

			Strings::const_iterator itString = strings.find(langCode);
			if ( itString != strings.end() )
				stringData[key].putValue(itString->second);
		}

		const VuJsonContainer &creationInfo = VuAssetBakery::IF()->getCreationInfo(VUPLATFORM, VuAssetFactory::IF()->getSku(), VuSys::IF()->getLanguage(), pStringAsset->getType(), pStringAsset->getAssetName());
		const std::string &fileName = creationInfo["File"].asString();
		VuJsonWriter writer;
		if ( !writer.saveToFile(langData, VuFile::IF()->getRootPath() + fileName) )
		{
			VUWARNING("Unable to write '%s'.\n", fileName.c_str());
			success = false;
		}

		VuAssetFactory::IF()->releaseAsset(pStringAsset);
	}

	if ( !success )
		return false;

	return true;
}

//*****************************************************************************
void VuStringDB::dumpCharacterMap(const std::string &languageCodes, std::wstring &characters)
{
	std::string utf8;
	char strLanguageCodes[256];
	VU_STRCPY(strLanguageCodes, sizeof(strLanguageCodes), languageCodes.c_str());
	char *nextToken = VUNULL;

	const char *code = VU_STRTOK(strLanguageCodes, ";", &nextToken);
	while ( code )
	{
		VuStringAsset *pStringAsset = VuAssetFactory::IF()->createAsset<VuStringAsset>(code);

		const VuJsonContainer &charCodes = pStringAsset->getDataContainer()["CharCodes"];
		const VuJsonContainer &strings = pStringAsset->getDataContainer()["Strings"];
		const VuJsonContainer &skuOverrides = pStringAsset->getDataContainer()["SkuOverrides"];

		// read strings
		for ( int iString = 0; iString < strings.numMembers(); iString++ )
		{
			const std::string &key = strings.getMemberKey(iString);
			utf8 += strings[key].asString();
		}

		// handle sku overrides
		for ( int iSku = 0; iSku < skuOverrides.numMembers(); iSku++ )
		{
			const std::string &sku = skuOverrides.getMemberKey(iSku);
			const VuJsonContainer &skuOverride = skuOverrides[sku];
			for ( int iString = 0; iString < skuOverrides.numMembers(); iString++ )
			{
				const std::string &key = skuOverride.getMemberKey(iString);
				utf8 += skuOverride[key].asString();
			}
		}

		// replace special character escape strings with codes
		for ( int iCode = 0; iCode < charCodes.numMembers(); iCode++ )
		{
			const std::string &key = charCodes.getMemberKey(iCode);
			const std::string &value = charCodes[key].asString();

			for ( size_t index = utf8.find(key); index != std::string::npos; index = utf8.find(key) )
				utf8.replace(index, key.length(), value);
		}

		VuAssetFactory::IF()->releaseAsset(pStringAsset);

		code = VU_STRTOK(VUNULL, ";", &nextToken);
	}

	// convert to unicode
	VuUtf8::convertUtf8StringToWCharString(utf8.c_str(), characters);

	// unique
	std::set<wchar_t> charSet;
	for ( int i = 0; i < (int)characters.size(); i++ )
		charSet.insert(characters[i]);

	characters.clear();
	for ( std::set<wchar_t>::const_iterator iter = charSet.begin(); iter != charSet.end(); iter++ )
		characters.push_back(*iter);
}

//*****************************************************************************
void VuStringDBImpl::consumeRow(std::string &input, std::vector<std::string> &row)
{
	row.clear();
	row.push_back("");

	while ( input.length() )
	{
		char c = input[0];
		input.erase(0, 1);

		if ( c == '\n' || c == '\r' )
			return;
		else if ( c == '\t' )
			row.push_back("");
		else if ( c == '\"' )
			;
		else
			row.back().push_back(c);
	}
}

//*****************************************************************************
bool VuStringDBImpl::addStrings(std::string &languageCode)
{
	// don't condense white space
	TiXmlBase::SetCondenseWhiteSpace(false);

	VuStringAsset *pStringAsset = VuAssetFactory::IF()->createAsset<VuStringAsset>(languageCode);

	const VuJsonContainer &options = pStringAsset->getDataContainer()["Options"];
	const VuJsonContainer &charCodes = pStringAsset->getDataContainer()["CharCodes"];
	const VuJsonContainer &strings = pStringAsset->getDataContainer()["Strings"];
	const VuJsonContainer &skuOverride = pStringAsset->getDataContainer()["SkuOverrides"][VuAssetFactory::IF()->getSku()];

	// read/set word wrap options
	if ( mStrings.empty() )
	{
		VUUINT wordWrapOptions = WordWrap_GetOption();

		bool value;
		if ( options["WW_PROHIBITION"].getValue(value) )
			wordWrapOptions = value ? wordWrapOptions |= WW_PROHIBITION : wordWrapOptions &= ~WW_PROHIBITION;
		if ( options["WW_NOHANGULWRAP"].getValue(value) )
			wordWrapOptions = value ? wordWrapOptions |= WW_NOHANGULWRAP : wordWrapOptions &= ~WW_NOHANGULWRAP;

		WordWrap_SetOption(wordWrapOptions);
	}

	// read strings
	for ( int iString = 0; iString < strings.numMembers(); iString++ )
	{
		const std::string &key = strings.getMemberKey(iString);
		mStrings[VuHash::fnv32String(key.c_str())] = strings[key].asString();
	}

	// read sku overrides
	for ( int iString = 0; iString < skuOverride.numMembers(); iString++ )
	{
		const std::string &key = skuOverride.getMemberKey(iString);
		mStrings[VuHash::fnv32String(key.c_str())] = skuOverride[key].asString();
	}

	// replace special character escape strings with codes
	for ( int iCode = 0; iCode < charCodes.numMembers(); iCode++ )
	{
		const std::string &key = charCodes.getMemberKey(iCode);
		const std::string &value = charCodes[key].asString();

		for ( StringMap::iterator iter = mStrings.begin(); iter != mStrings.end(); iter++ )
			for ( size_t index = iter->second.find(key); index != std::string::npos; index = iter->second.find(key) )
				iter->second.replace(index, key.length(), value);
	}

	// clean up
	VuAssetFactory::IF()->releaseAsset(pStringAsset);

	return true;
}
