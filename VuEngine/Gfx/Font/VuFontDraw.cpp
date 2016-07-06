//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuFontDraw class
// 
//*****************************************************************************

#include "VuFontDraw.h"
#include "VuFont.h"
#include "VuEngine/HAL/Gfx/VuGfx.h"
#include "VuEngine/HAL/Gfx/VuShaderProgram.h"
#include "VuEngine/HAL/Gfx/VuVertexDeclaration.h"
#include "VuEngine/HAL/Gfx/VuPipelineState.h"
#include "VuEngine/HAL/Gfx/VuTexture.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuMathUtil.h"
#include "VuEngine/Memory/VuScratchPad.h"
#include "VuEngine/Managers/VuViewportManager.h"
#include "VuEngine/Util/VuUtf8.h"
#include "VuEngine/Util/VuWordWrap.h"
#include "VuEngine/Assets/VuTextureAsset.h"
#include "VuEngine/Assets/VuCompiledShaderAsset.h"
#include "VuEngine/Assets/VuAssetFactory.h"


// special character indices
#define INDEX_INVALID		0xffff
#define INDEX_NEWLINE		0xfffe
#define INDEX_COLOR_PREV	0xfffd
#define INDEX_COLOR_HI		0xfff9
#define INDEX_COLOR_LO		0xfff0
#define INDEX_SPECIAL_START	0xfff0

#define MAX_COLOR_COUNT (INDEX_COLOR_HI - INDEX_COLOR_LO + 1)

// flavors
enum eFlavor { FLAVOR_SIMPLE, FLAVOR_OUTLINE, FLAVOR_DRAW_IMAGE, FLAVOR_COUNT };

class VuFontShaderFlavor
{
public:
	VuFontShaderFlavor();
	~VuFontShaderFlavor();

	bool	create(const char *shaderName, const VuVertexDeclarationParams &vdParams);

	VuGfxSortMaterial		*mpMaterial;

	VUHANDLE				mhConstTransform;
	VUHANDLE				mhConstDistMinMax;
	VUHANDLE				mhConstOutlineMinMax;
	VUHANDLE				mhConstOutlineColor;
};

// data passed to gfx sort system
struct VuFontDrawData
{
	VuFont				*mpFont;
	eFlavor				mFlavor;
	VuFontDrawParams	mParams;
	VuRect				mRect;
	int					mFlags;
	float				mTextScale;
	float				mDepth;
	VuVector2			mOffset;
	VuRect				mClipRect;
};

// static members
VuFontDraw::CharIndexBuffer	VuFontDraw::smMeasureCharIndexBuffer;
std::wstring				VuFontDraw::smTempUnicodeBuffer;
std::string					VuFontDraw::smTempUtf8Buffers[2];
VuFontDraw::UnicodeArray	VuFontDraw::smLowerToUpperCaseLookup;
VuFontDraw::UnicodeArray	VuFontDraw::smUpperToLowerCaseLookup;


//*****************************************************************************
VuFontDraw::VuFontDraw():
	mpCurFlavor(VUNULL)
{
	mpShaderFlavors = new VuFontShaderFlavor[FLAVOR_COUNT];

	// reserve some space to start with
	mRenderCharIndexBuffer.reserve(4*1024);
	mImageBuffer.reserve(16);

	smMeasureCharIndexBuffer.reserve(4*1024);

	VU_COMPILE_TIME_ASSERT(MAX_COLOR_COUNT == MAX_COLOR_BUFFER_SIZE);
}

//*****************************************************************************
VuFontDraw::~VuFontDraw()
{
	delete[] mpShaderFlavors;
}

//*****************************************************************************
bool VuFontDraw::init()
{
	// create vertex declaration
	VuVertexDeclarationParams vdParams;
	vdParams.mElements.push_back(VuVertexDeclarationElement(0,  0, VUGFX_DECL_TYPE_FLOAT3,  VUGFX_DECL_USAGE_POSITION, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 12, VUGFX_DECL_TYPE_FLOAT2,  VUGFX_DECL_USAGE_TEXCOORD, 0));
	vdParams.mElements.push_back(VuVertexDeclarationElement(0, 20, VUGFX_DECL_TYPE_UBYTE4N, VUGFX_DECL_USAGE_COLOR,    0));
	vdParams.mStreams.push_back(VuVertexDeclarationStream(24));

	// create shader flavors
	{
		if ( !mpShaderFlavors[FLAVOR_SIMPLE].create("Font/Simple", vdParams) )
			return false;

		if ( !mpShaderFlavors[FLAVOR_OUTLINE].create("Font/Outline", vdParams) )
			return false;

		if ( !mpShaderFlavors[FLAVOR_DRAW_IMAGE].create("Font/DrawImage", vdParams) )
			return false;
	}

	// build case lookup arrays
	buildCaseLookupArrays();

	return true;
}

//*****************************************************************************
void VuFontDraw::release()
{
	VUASSERT(mMacroHandlers.empty(), "VuFontDraw macro handler leak");
}

//*****************************************************************************
void VuFontDraw::drawString(float depth, VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, const VuRect &rect, int flags, float alpha, const VuVector2 &offset, VUUINT transType)
{
	if ( !pFont )
		return;

	if ( params.mSize <= 0 )
		return;

	if ( !strTextUtf8 || !strTextUtf8[0] )
		return;

	int tempBuffer = 0;

	// take care of macros
	while ( strstr(strTextUtf8, "[[") )
	{
		strTextUtf8 = handleMacros(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}

	// take care of upper/lower case
	if ( params.mFlags & VuFontDrawParams::FORCE_UPPER_CASE )
	{
		strTextUtf8 = forceUpperCase(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}
	if ( params.mFlags & VuFontDrawParams::FORCE_LOWER_CASE )
	{
		strTextUtf8 = forceLowerCase(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}

	// apply transform (rotation not supported)
	const VuMatrix &mat = VuGfxUtil::IF()->getMatrix();
	VuVector2 trans(mat.getTrans().mX, mat.getTrans().mY);
	VuVector2 scale(mat.getAxisX().mX, mat.getAxisY().mY);
	VuRect screenRect = (rect*scale) + trans;
	VuVector2 screenOffset = offset*scale;
	VuRect screenClipRect(0,0,1,1);
	if ( params.mClip )
		screenClipRect = VuRect::intersection(screenClipRect, (params.mClipRect*scale) + trans);

	// determine flavor (material)
	eFlavor flavor = params.mOutlineWeight > 0 ? FLAVOR_OUTLINE : FLAVOR_SIMPLE;
	VuGfxSortMaterial *pMaterial = mpShaderFlavors[flavor].mpMaterial;

	if ( flags & VUGFX_TEXT_DRAW_SYNCHRONOUS )
	{
		// draw now
		VuFontDrawData fdd;

		fdd.mpFont = pFont;
		fdd.mFlavor = flavor;
		fdd.mParams = params;
		fdd.mRect = screenRect;
		fdd.mFlags = flags;
		fdd.mTextScale = VuGfxUtil::IF()->getTextScale();
		fdd.mDepth = depth;
		fdd.mOffset = screenOffset;
		fdd.mClipRect = screenClipRect;

		fdd.mParams.mColor.mA = (VUUINT8)VuRound(fdd.mParams.mColor.mA*alpha);
		fdd.mParams.mOutlineColor.mA = (VUUINT8)VuRound(fdd.mParams.mOutlineColor.mA*alpha);

		VuGfx::IF()->setPipelineState(pMaterial->mpPipelineState);

		submitDrawCommands(&fdd, strTextUtf8);
	}
	else
	{
		// submit command
		int textSize = (int)strlen(strTextUtf8) + 1;
		int allocSize = sizeof(VuFontDrawData) + textSize;
		VuFontDrawData *pData = static_cast<VuFontDrawData *>(VuGfxSort::IF()->allocateCommandMemory(allocSize));
		char *strData = reinterpret_cast<char *>(pData + 1);

		pData->mpFont = pFont;
		pData->mFlavor = flavor;
		pData->mParams = params;
		pData->mRect = screenRect;
		pData->mFlags = flags;
		pData->mTextScale = VuGfxUtil::IF()->getTextScale();
		pData->mDepth = depth;
		pData->mOffset = screenOffset;
		pData->mClipRect = screenClipRect;
		VU_STRCPY(strData, textSize, strTextUtf8);

		pData->mParams.mColor.mA = (VUUINT8)VuRound(pData->mParams.mColor.mA*alpha);
		pData->mParams.mOutlineColor.mA = (VUUINT8)VuRound(pData->mParams.mOutlineColor.mA*alpha);

		VuGfxSort::IF()->submitDrawCommand<true>(transType, pMaterial, VUNULL, &submitDrawCommandsCallback, depth);
	}
}

//*****************************************************************************
VuRect VuFontDraw::measureString(VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, const VuRect &rect, int flags, float renderTargetAspectRatio, const VuVector2 &offset)
{
	VuRect outRect(1e9f,1e9f,-2e9f,-2e9f);

	if ( !pFont )
		return outRect;

	if ( params.mSize <= 0 )
		return outRect;

	if ( !strTextUtf8 || !strTextUtf8[0] )
		return outRect;

	// apply transform (rotation not supported)
	const VuMatrix &mat = VuGfxUtil::IF()->getMatrix();
	VuVector2 gfxTrans(mat.getTrans().mX, mat.getTrans().mY);
	VuVector2 gfxScale(mat.getAxisX().mX, mat.getAxisY().mY);
	VuRect screenRect = (rect*gfxScale) + gfxTrans;
	VuVector2 screenOffset = offset*gfxScale;
	VuRect screenClipRect(0,0,1,1);
	if ( params.mClip )
		screenClipRect = VuRect::intersection(screenClipRect, (params.mClipRect*gfxScale) + gfxTrans);

	// determine clip rect
	VuRect clipRect = screenClipRect;
	if ( flags & VUGFX_TEXT_DRAW_CLIP )
		clipRect = VuRect::intersection(clipRect, screenRect);

	// if invalid result, we're done
	if ( clipRect.mWidth < FLT_EPSILON || clipRect.mHeight < FLT_EPSILON )
		return outRect;

	// use aspect ratio to calculate scale
	VuVector2 scale;
	scale.mX = (params.mSize/FONT_DRAW_SCALE_Y)*params.mStretch/renderTargetAspectRatio;
	scale.mY = (params.mSize/FONT_DRAW_SCALE_Y);

	// transform softness from screens-space to distance-space
	float softnessDist = (0.01f*params.mSoftness)/(2.0f*pFont->mMaxRadius);

	// transform outline params from screen-space to distance-space
	float outlineDist = (0.01f*params.mOutlineWeight)/(2.0f*pFont->mMaxRadius);
//	float outlineSoftnessDist = outlineDist > 0.0f ? (0.01f*params.mOutlineSoftness)/(2.0f*pFont->mMaxRadius) : 0.0f;

	// calculate kill distance
	float killDist = 1.0f - 0.01f*params.mWeight*0.5f - outlineDist;
	VuVector2 distMinMax(killDist - 0.5f*softnessDist, killDist + 0.5f*softnessDist);

	// determine extra font-space distance to consider when clipping right and bottom due to font weight/softness/outline/slant
	VuVector2 extraClipSize;
	extraClipSize.mX = (0.5f - distMinMax.mX)*2.0f*pFont->mMaxRadius + pFont->mAscender*params.mSlant;
	extraClipSize.mY = (0.5f - distMinMax.mX)*2.0f*pFont->mMaxRadius;

	// apply scale after distances have been calculated
	scale *= VuGfxUtil::IF()->getTextScale();

	int tempBuffer = 0;

	// take care of macros
	while ( strstr(strTextUtf8, "[[") )
	{
		strTextUtf8 = handleMacros(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}

	// take care of upper/lower case
	if ( params.mFlags & VuFontDrawParams::FORCE_UPPER_CASE )
	{
		strTextUtf8 = forceUpperCase(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}
	if ( params.mFlags & VuFontDrawParams::FORCE_LOWER_CASE )
	{
		strTextUtf8 = forceLowerCase(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}

	// take care of word-breaks
	if ( flags & VUGFX_TEXT_DRAW_WORDBREAK )
	{
		float width = screenRect.mWidth/scale.mX - extraClipSize.mX;
		if ( width > FLT_EPSILON )
		{
			strTextUtf8 = handleWordBreak(pFont, strTextUtf8, width, smTempUnicodeBuffer, smTempUtf8Buffers[tempBuffer]);
			tempBuffer = !tempBuffer;
		}
	}

	Context context;
	context.mpFont = pFont;
	context.mClipRect = clipRect;
	context.mScale = scale;
	context.mExtraClipSize = extraClipSize;
	context.mpCharIndexBuffer = &smMeasureCharIndexBuffer;
	context.mCurColor = params.mColor;

	// build char index buffer utf-8 text
	buildCharIndexBuffer(context, strTextUtf8, params.mTabSize, flags);

	// calculate vertical starting position, handling vert alignment
	float posY = calcVertStart(context, screenRect, flags);
	posY += screenOffset.mY;

	// draw one line at a time, handling horz alignment
	int bufPos = 0;
	while ( bufPos < smMeasureCharIndexBuffer.size() )
	{
		VuRect lineRect = measureLine(context, params, screenRect, flags, posY, bufPos, screenOffset.mX);
		outRect.add(lineRect);

		posY += scale.mY;
	}

	if ( outRect.isValid() )
	{
		outRect = VuRect::intersection(outRect, clipRect);

		// transform back through gfx xform
		outRect = (outRect - gfxTrans)/gfxScale;
	}

	return outRect;
}

//*****************************************************************************
VuVector2 VuFontDraw::measureString(VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, float rectWidth, int flags, float renderTargetAspectRatio)
{
	// determine metrics
	VuVector2 scale, extraClipSize;

	// use aspect ratio to calculate scale
	scale.mX = (params.mSize/FONT_DRAW_SCALE_Y)*params.mStretch/renderTargetAspectRatio;
	scale.mY = (params.mSize/FONT_DRAW_SCALE_Y);

	// transform softness from screens-space to distance-space
	float softnessDist = (0.01f*params.mSoftness)/(2.0f*pFont->mMaxRadius);

	// transform outline params from screen-space to distance-space
	float outlineDist = (0.01f*params.mOutlineWeight)/(2.0f*pFont->mMaxRadius);

	// calculate kill distance
	float killDist = 1.0f - 0.01f*params.mWeight*0.5f - outlineDist;
	VuVector2 distMinMax(killDist - 0.5f*softnessDist, killDist + 0.5f*softnessDist);

	// determine extra font-space distance to consider when clipping right and bottom due to font weight/softness/outline/slant
	extraClipSize.mX = (0.5f - distMinMax.mX)*2.0f*pFont->mMaxRadius + pFont->mAscender*params.mSlant;
	extraClipSize.mY = (0.5f - distMinMax.mX)*2.0f*pFont->mMaxRadius;

	int tempBuffer = 0;

	// take care of macros
	while ( strstr(strTextUtf8, "[[") )
	{
		strTextUtf8 = handleMacros(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}

	// take care of upper/lower case
	if ( params.mFlags & VuFontDrawParams::FORCE_UPPER_CASE )
	{
		strTextUtf8 = forceUpperCase(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}
	if ( params.mFlags & VuFontDrawParams::FORCE_LOWER_CASE )
	{
		strTextUtf8 = forceLowerCase(strTextUtf8, smTempUtf8Buffers[tempBuffer]);
		tempBuffer = !tempBuffer;
	}

	// take care of word-breaks
	if ( flags & VUGFX_TEXT_DRAW_WORDBREAK )
	{
		float width = rectWidth/scale.mX - extraClipSize.mX;
		if ( width > FLT_EPSILON )
		{
			strTextUtf8 = handleWordBreak(pFont, strTextUtf8, width, smTempUnicodeBuffer, smTempUtf8Buffers[tempBuffer]);
			tempBuffer = !tempBuffer;
		}
	}

	Context context;
	context.mpFont = pFont;
	context.mClipRect = VuRect(0,0,1,1);
	context.mScale = scale;
	context.mExtraClipSize = extraClipSize;
	context.mpCharIndexBuffer = &smMeasureCharIndexBuffer;
	context.mCurColor = params.mColor;

	// build char index buffer utf-8 text
	buildCharIndexBuffer(context, strTextUtf8, params.mTabSize, flags);

	// calculate size in font space
	VuVector2 size(0.0f, 0.0f);

	// parse text
	float fCurLineWidth = 0;
	for ( VUUINT16 *curChar = &smMeasureCharIndexBuffer.begin(); curChar != &smMeasureCharIndexBuffer.end(); curChar++ )
	{
		if ( *curChar >= INDEX_SPECIAL_START )
		{
			if ( *curChar == INDEX_NEWLINE )
			{
				//handle newline
				size.mX = VuMax(size.mX, fCurLineWidth);
				size.mY += 1.0f;
				fCurLineWidth = 0;
			}
		}
		else
		{
			// look up char index for unicode character
			fCurLineWidth += pFont->mCharacters[*curChar].mAdvance;
		}
	}
	size.mX = VuMax(size.mX, fCurLineWidth);

	// handle ascent, descent
	size.mY += pFont->mAscender - pFont->mDescender;

	// handle extra clip
	size.mX += extraClipSize.mX;
	size.mY += 2.0f*extraClipSize.mY;

	// apply scale
	size *= scale;

	return size;
}

//*****************************************************************************
float VuFontDraw::measureStringWidth(VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, float renderTargetAspectRatio)
{
	// calculate size in font space
	float width = 0.0f;

	// determine tab width
	float tabWidth = 0;
	VUUINT16 indexSpace = pFont->codeToIndex(' ');
	if ( indexSpace != INDEX_INVALID )
		tabWidth = params.mTabSize*pFont->mCharacters[indexSpace].mAdvance;

	// parse text
	float fCurLineWidth = 0;
	while ( *strTextUtf8 )
	{
		if ( strTextUtf8[0] == '\n' )
		{
			//handle newline
			width = VuMax(width, fCurLineWidth);
			fCurLineWidth = 0;
			strTextUtf8++;
		}
		else if ( strTextUtf8[0] == '\t' )
		{
			//handle tab
			fCurLineWidth += tabWidth;
			strTextUtf8++;
		}
		else if ( strTextUtf8[0] == '{' && strTextUtf8[1] == '[' )
		{
			const char *strNext = strstr(strTextUtf8, "]}");
			if ( !strNext )
				break; // error

			strTextUtf8 = strNext + 2;
		}
		else
		{
			// translate utf-8 character to unicode
			VUUINT32 charCode;
			int nBytes = VuUtf8::convertUtf8ToUnicode(strTextUtf8, charCode);
			if ( nBytes )
			{
				// look up char index for unicode character
				VUUINT16 index = pFont->codeToIndex(charCode);
				if ( index != INDEX_INVALID )
					fCurLineWidth += pFont->mCharacters[index].mAdvance;

				strTextUtf8 += nBytes;
			}
			else
			{
				// bad utf-8 character, skip it!
				strTextUtf8 += 1;
			}
		}
	}
	width = VuMax(width, fCurLineWidth);

	// use aspect ratio to calculate scale
	float scale = (params.mSize/FONT_DRAW_SCALE_Y)*params.mStretch/renderTargetAspectRatio;

	// apply scale
	width *= scale;

	return width;
}

//*****************************************************************************
float VuFontDraw::measureStringWidth(VuFont *pFont, const wchar_t *strText16, const VuFontDrawParams &params, float renderTargetAspectRatio)
{
	// calculate size in font space
	float width = 0.0f;

	// determine tab width
	float tabWidth = 0;
	VUUINT16 indexSpace = pFont->codeToIndex(' ');
	if ( indexSpace != INDEX_INVALID )
		tabWidth = params.mTabSize*pFont->mCharacters[indexSpace].mAdvance;

	// parse text
	float fCurLineWidth = 0;
	while ( *strText16 )
	{
		if ( strText16[0] == '\n' )
		{
			//handle newline
			width = VuMax(width, fCurLineWidth);
			fCurLineWidth = 0;
			strText16++;
		}
		else if ( strText16[0] == '\t' )
		{
			//handle tab
			fCurLineWidth += tabWidth;
			strText16++;
		}
		else if ( strText16[0] == '{' && strText16[1] == '[' )
		{
			const wchar_t *strNext = wcsstr(strText16, L"]}");
			if ( !strNext )
				break; // error

			strText16 = strNext + 2;
		}
		else
		{
			// look up char index for unicode character
			VUUINT16 index = pFont->codeToIndex(*strText16);
			if ( index != INDEX_INVALID )
				fCurLineWidth += pFont->mCharacters[index].mAdvance;

			strText16++;
		}
	}
	width = VuMax(width, fCurLineWidth);

	// use aspect ratio to calculate scale
	float scale = (params.mSize/FONT_DRAW_SCALE_Y)*params.mStretch/renderTargetAspectRatio;

	// apply scale
	width *= scale;

	return width;
}

//*****************************************************************************
void VuFontDraw::addMacroHandler(VuMacroHandler *pHandler)
{
	mMacroHandlers.push_back(pHandler);
}

//*****************************************************************************
void VuFontDraw::removeMacroHandler(VuMacroHandler *pHandler)
{
	auto iter = std::find(mMacroHandlers.begin(), mMacroHandlers.end(), pHandler);
	if ( iter != mMacroHandlers.end() )
		mMacroHandlers.erase(iter);
}

//*****************************************************************************
void VuFontDraw::handleMacro(const char *macro, std::string &text)
{
	for ( MacroHandlers::iterator iter = mMacroHandlers.begin(); iter != mMacroHandlers.end(); iter++ )
		if ( (*iter)->handleMacro(macro, text) )
			return;

	text += macro; // not handled
}

//*****************************************************************************
void VuFontDraw::submitDrawCommandsCallback(void *pData)
{
	VuFontDrawData *pFontDrawData = static_cast<VuFontDrawData *>(pData);
	char *strData = reinterpret_cast<char *>(pFontDrawData + 1);

	VuGfxUtil::IF()->fontDraw()->submitDrawCommands(pFontDrawData, strData);
}

//*****************************************************************************
void VuFontDraw::submitDrawCommands(const VuFontDrawData *pDrawData, const char *strTextUtf8)
{
	const VuFontDrawParams &params = pDrawData->mParams;
	const VuRect &screenRect = pDrawData->mRect;
	int flags = pDrawData->mFlags;

	VuFont *pFont = pDrawData->mpFont;
	mpCurFlavor = &mpShaderFlavors[pDrawData->mFlavor];

	// set texture
	VuGfx::IF()->setTexture(0, pFont->mpTexture);

	// determine clip rect
	bool scissor = params.mClip;
	VuRect clipRect = pDrawData->mClipRect;
	if ( flags & VUGFX_TEXT_DRAW_CLIP )
	{
		clipRect = VuRect::intersection(clipRect, screenRect);
		scissor = true;
	}

	// if invalid result, we're done
	if ( clipRect.mWidth < FLT_EPSILON || clipRect.mHeight < FLT_EPSILON )
		return;

	if ( scissor )
	{
		// set scissor rect
		VuRect scissorRect = VuMathUtil::applySafeZone(clipRect, VuViewportManager::IF()->getSafeZone());
		VuGfx::IF()->setScissorRect(&scissorRect);
	}

	// use aspect ratio to calculate scale
	int renderTargetWidth, renderTargetHeight;
	VuGfx::IF()->getCurRenderTargetSize(renderTargetWidth, renderTargetHeight);
	float aspectRatio = (float)renderTargetWidth/renderTargetHeight;
	VuVector2 scale;
	scale.mX = (params.mSize/FONT_DRAW_SCALE_Y)*params.mStretch/aspectRatio;
	scale.mY = (params.mSize/FONT_DRAW_SCALE_Y);

	// transform softness from screens-space to distance-space
	float softnessDist = (0.01f*params.mSoftness)/(2.0f*pFont->mMaxRadius);

	// transform outline params from screen-space to distance-space
	float outlineDist = (0.01f*params.mOutlineWeight)/(2.0f*pFont->mMaxRadius);
	float outlineSoftnessDist = outlineDist > 0.0f ? (0.01f*params.mOutlineSoftness)/(2.0f*pFont->mMaxRadius) : 0.0f;

	// calculate kill distance
	float killDist = 1.0f - 0.01f*params.mWeight*0.5f - outlineDist;
	VuVector2 distMinMax(killDist - 0.5f*softnessDist, killDist + 0.5f*softnessDist);

	// determine extra font-space distance to consider when clipping right and bottom due to font weight/softness/outline/slant
	VuVector2 extraClipSize;
	extraClipSize.mX = (0.5f - distMinMax.mX)*2.0f*pFont->mMaxRadius + pFont->mAscender*params.mSlant;
	extraClipSize.mY = (0.5f - distMinMax.mX)*2.0f*pFont->mMaxRadius;

	// apply scale after distances have been calculated
	scale *= pDrawData->mTextScale;

	// set constants
	VuShaderProgram *pSP = mpCurFlavor->mpMaterial->mpShaderProgram;
	pSP->setConstantVector2(mpCurFlavor->mhConstDistMinMax, distMinMax);
	if ( mpCurFlavor->mhConstOutlineMinMax )
		pSP->setConstantVector2(mpCurFlavor->mhConstOutlineMinMax, VuVector2(killDist + outlineDist, killDist + outlineDist + outlineSoftnessDist));
	if ( mpCurFlavor->mhConstOutlineColor )
		pSP->setConstantColor4(mpCurFlavor->mhConstOutlineColor, params.mOutlineColor);

	// take care of word-breaks
	if ( flags & VUGFX_TEXT_DRAW_WORDBREAK )
	{
		float width = screenRect.mWidth/scale.mX - extraClipSize.mX;
		if ( width > FLT_EPSILON )
			strTextUtf8 = handleWordBreak(pFont, strTextUtf8, width, mRenderTempUnicodeBuffer, mRenderTempUtf8Buffer);
	}

	// create context
	Context context;
	context.mpFont = pFont;
	context.mClipRect = clipRect;
	context.mScale = scale;
	context.mExtraClipSize = extraClipSize;
	context.mpCharIndexBuffer = &mRenderCharIndexBuffer;
	context.mCurColor = params.mColor;

	// build char index buffer utf-8 text
	buildCharIndexBuffer(context, strTextUtf8, params.mTabSize, flags);

	// calculate vertical starting position, handling vert alignment
	float posY = calcVertStart(context, screenRect, flags);
	posY += pDrawData->mOffset.mY;

	// draw one line at a time, handling horz alignment
	int bufPos = 0;
	while ( bufPos < mRenderCharIndexBuffer.size() )
	{
		drawLine(context, params, screenRect, flags, posY, bufPos, pDrawData->mDepth, pDrawData->mOffset.mX);
		posY += scale.mY;
	}

	// draw images
	drawImages();

	// restore scissor state
	if ( scissor )
		VuGfx::IF()->setScissorRect(VUNULL);
}

//*****************************************************************************
void VuFontDraw::buildCharIndexBuffer(Context &context, const char *strTextUtf8, int tabSize, int flags)
{
	const VuFont *pFont = context.mpFont;
	CharIndexBuffer &buffer = *context.mpCharIndexBuffer;

	// determine char index for space
	VUUINT16 indexSpace = pFont->codeToIndex(' ');

	buffer.clear();
	while ( *strTextUtf8 )
	{
		if ( strTextUtf8[0] == '\n' )
		{
			//handle newline
			buffer.push_back(INDEX_NEWLINE);
			strTextUtf8++;
		}
		else if ( strTextUtf8[0] == '\t' )
		{
			//handle tab
			for ( int i = 0; i < tabSize; i++ )
				buffer.push_back(indexSpace);
			strTextUtf8++;
		}
		else if ( strTextUtf8[0] == '{' && strTextUtf8[1] == '[' )
		{
			const char *strNext = strstr(strTextUtf8, "]}");
			if ( !strNext )
				break; // error

			VUUINT32 charCode = INDEX_COLOR_PREV;
			int r, g, b, a = 255;
			if ( VU_SSCANF(strTextUtf8, "{[%d,%d,%d,%d]}", &r, &g, &b, &a) >= 3 )
				charCode = (VUUINT32)addToColorBuffer(context, VuColor((VUUINT8)r, (VUUINT8)g, (VUUINT8)b, (VUUINT8)a));
			buffer.push_back((VUUINT16)charCode);

			strTextUtf8 = strNext + 2;
		}
		else
		{
			// translate utf-8 character to unicode
			VUUINT32 charCode;
			int nBytes = VuUtf8::convertUtf8ToUnicode(strTextUtf8, charCode);
			if ( nBytes )
			{
				// look up char index for unicode character
				VUUINT16 index = pFont->codeToIndex(charCode);
				if ( index != INDEX_INVALID )
					buffer.push_back(index);

				strTextUtf8 += nBytes;
			}
			else
			{
				// bad utf-8 character, skip it!
				strTextUtf8 += 1;
			}
		}
	}
}

//*****************************************************************************
VUUINT16 VuFontDraw::addToColorBuffer(Context &context, const VuColor &color)
{
	// try to find color
	for ( int i = 0; i < context.mColorBufferSize; i++ )
		if ( context.mColorBuffer[i] == color )
			return (VUUINT16)(INDEX_COLOR_LO + i);

	// try to add color
	if ( context.mColorBufferSize < MAX_COLOR_COUNT )
	{
		context.mColorBuffer[context.mColorBufferSize++] = color;
		return (VUUINT16)(INDEX_COLOR_LO + context.mColorBufferSize - 1);
	}

	// no room
	return INDEX_COLOR_PREV;
}

//*****************************************************************************
const char *VuFontDraw::handleWordBreak(const VuFont *pFont, const char *strTextUtf8, float width, std::wstring &unicodeBuffer, std::string &utf8Buffer)
{
	utf8Buffer.clear();

	// convert to unicode16
	VuUtf8::convertUtf8StringToWCharString(strTextUtf8, unicodeBuffer);

	const wchar_t *wsz = unicodeBuffer.c_str();

	do
	{
		const wchar_t *wszEOL = VUNULL;
		const wchar_t *wszNext = WordWrap_FindNextLine( wsz, (VUUINT)(width*1000), wordBreakGetWidthCB, pFont, &wszEOL );

		if ( wszEOL )
		{
			int count = (int)((wszEOL+1) - wsz);
			for ( int i = 0; i < count; i++ )
			{
				VuUtf8::appendUnicodeToUtf8String(wsz[i], utf8Buffer);
			}
		}

		if ( wszNext )
		{
			utf8Buffer += "\n";
		}
		wsz = wszNext;
	} while( wsz );

	return utf8Buffer.c_str();
}

//*****************************************************************************
VUUINT VuFontDraw::wordBreakGetWidthCB(wchar_t unicodeChar, const void *pUserData)
{
	const VuFont *pFont = static_cast<const VuFont *>(pUserData);

	int charWidth = 0;

	VUUINT16 index = pFont->codeToIndex(unicodeChar);
	if ( index != INDEX_INVALID )
	{
		charWidth = (VUUINT)(pFont->mCharacters[index].mAdvance*1000);
	}

	return charWidth;
}

//*****************************************************************************
const char *VuFontDraw::handleMacros(const char *strTextUtf8, std::string &utf8Buffer)
{
	utf8Buffer.clear();

	while ( *strTextUtf8 )
	{
		if ( strTextUtf8[0] == '[' && strTextUtf8[1] == '[' )
		{
			strTextUtf8 += 2;
			if ( const char *strEnd = strstr(strTextUtf8, "]]") )
			{
				char strMacro[256];
				int length = (int)(strEnd - strTextUtf8);
				VU_STRNCPY(strMacro, sizeof(strMacro), strTextUtf8, length);
				strMacro[length] = '\0';

				VuGfxUtil::IF()->fontDraw()->handleMacro(strMacro, utf8Buffer);

				strTextUtf8 = strEnd + 2;
			}
		}
		else
		{
			VUUINT32 charCode;
			int nBytes = VuUtf8::convertUtf8ToUnicode(strTextUtf8, charCode);
			if ( nBytes )
			{
				VuUtf8::appendUnicodeToUtf8String(charCode, utf8Buffer);
				strTextUtf8 += nBytes;
			}
			else
			{
				// bad utf-8 character, skip it!
				strTextUtf8 += 1;
			}
		}
	}

	return utf8Buffer.c_str();
}

//*****************************************************************************
const char *VuFontDraw::forceUpperCase(const char *strTextUtf8, std::string &utf8Buffer)
{
	utf8Buffer.clear();

	while ( *strTextUtf8 )
	{
		VUUINT32 charCode;
		int nBytes = VuUtf8::convertUtf8ToUnicode(strTextUtf8, charCode);
		if ( nBytes )
		{
			if ( charCode < smLowerToUpperCaseLookup.size() )
				charCode = smLowerToUpperCaseLookup[charCode];
			VuUtf8::appendUnicodeToUtf8String(charCode, utf8Buffer);

			strTextUtf8 += nBytes;
		}
		else
		{
			// bad utf-8 character, skip it!
			strTextUtf8 += 1;
		}
	}

	return utf8Buffer.c_str();
}

//*****************************************************************************
const char *VuFontDraw::forceLowerCase(const char *strTextUtf8, std::string &utf8Buffer)
{
	utf8Buffer.clear();

	while ( *strTextUtf8 )
	{
		VUUINT32 charCode;
		int nBytes = VuUtf8::convertUtf8ToUnicode(strTextUtf8, charCode);
		if ( nBytes )
		{
			if ( charCode < smUpperToLowerCaseLookup.size() )
				charCode = smUpperToLowerCaseLookup[charCode];
			VuUtf8::appendUnicodeToUtf8String(charCode, utf8Buffer);

			strTextUtf8 += nBytes;
		}
		else
		{
			// bad utf-8 character, skip it!
			strTextUtf8 += 1;
		}
	}

	return utf8Buffer.c_str();
}

//*****************************************************************************
float VuFontDraw::calcVertStart(Context &context, const VuRect &rect, int flags)
{
	const VuFont *pFont = context.mpFont;
	const VuVector2 &scale = context.mScale;
	const VuVector2 &extraClipSize = context.mExtraClipSize;
	const CharIndexBuffer &buffer = *context.mpCharIndexBuffer;

	// handle alignment to top
	if ( !(flags & (VUGFX_TEXT_DRAW_BOTTOM|VUGFX_TEXT_DRAW_BASELINE|VUGFX_TEXT_DRAW_VCENTER)) )
		return rect.getTop() + scale.mY*(pFont->mAscender + extraClipSize.mY);

	// count # of lines
	int lineCount = 1;
	for ( const VUUINT16 *pIndex = &buffer.begin(); pIndex != &buffer.end(); pIndex++ )
		if ( *pIndex == INDEX_NEWLINE )
			lineCount++;

	// handle alignment to bottom
	if ( flags & VUGFX_TEXT_DRAW_BOTTOM )
		return rect.getBottom() + scale.mY*(pFont->mDescender - extraClipSize.mY) - scale.mY*(lineCount - 1);

	// handle alignment to baseline
	if ( flags & VUGFX_TEXT_DRAW_BASELINE )
		return rect.getBottom() - scale.mY*(lineCount - 1);

	// handle alignment to center
	float totalHeight = scale.mY*(pFont->mAscender - pFont->mDescender) + scale.mY*(lineCount - 1);
	float top = rect.getCenter().mY - 0.5f*totalHeight;
	return top + scale.mY*pFont->mAscender;
}

//*****************************************************************************
float VuFontDraw::calcLineWidth(Context &context, int bufPos)
{
	const VuFont *pFont = context.mpFont;
	const VuVector2 &scale = context.mScale;
	const VuVector2 &extraClipSize = context.mExtraClipSize;
	const CharIndexBuffer &buffer = *context.mpCharIndexBuffer;

	float caretPos = 0;
	for ( const VUUINT16 *pIndex = &buffer[bufPos]; pIndex != &buffer.end(); pIndex++ )
	{
		if ( *pIndex < INDEX_SPECIAL_START )
			caretPos += pFont->mCharacters[*pIndex].mAdvance;
		else if ( *pIndex == INDEX_NEWLINE )
			break;
	}

	// account for extra clip size due to font weight/softness/outline/slant
	caretPos += extraClipSize.mX;

	// transform to screen space
	caretPos *= scale.mX;

	return caretPos;
}

//*****************************************************************************
float VuFontDraw::calcHorzStart(const VuRect &rect, int flags, float lineWidth)
{
	// handle alignment to left
	if ( !(flags & (VUGFX_TEXT_DRAW_RIGHT|VUGFX_TEXT_DRAW_HCENTER)) )
		return rect.getLeft();

	// handle alignment to right
	if ( flags & VUGFX_TEXT_DRAW_RIGHT )
		return rect.getRight() - lineWidth;

	// handle alignment to center
	return rect.getCenter().mX - 0.5f*lineWidth;
}

//*****************************************************************************
void VuFontDraw::drawLine(Context &context, const VuFontDrawParams &params, const VuRect &rect, int flags, float posY, int &bufPos, float depth, float offsetX)
{
	const VuFont *pFont = context.mpFont;
	const VuRect &clipRect = context.mClipRect;
	const VuVector2 &scale = context.mScale;
	const VuVector2 &extraClipSize = context.mExtraClipSize;

	// check for empty line
	if ( mRenderCharIndexBuffer[bufPos] == INDEX_NEWLINE )
	{
		bufPos++;
		return;
	}

	// vertical trivial rejection of whole line
	if ( (posY - scale.mY*(pFont->mDescender - extraClipSize.mY) < clipRect.getTop()) ||
		 (posY - scale.mY*(pFont->mAscender + extraClipSize.mY) > clipRect.getBottom()) )
	{
		bufPos = skipLine(context, params, bufPos);
		return;
	}

	// determine line witdh
	float lineWidth = calcLineWidth(context, bufPos);

	// calculate horizontal starting position, handling horz alignment
	float posX = calcHorzStart(rect, flags, lineWidth);
	posX += offsetX;

	// trivial horizontal rejection of whole line
	if ( posX + lineWidth < clipRect.getLeft() || posX > clipRect.getRight() )
	{
		bufPos = skipLine(context, params, bufPos);
		return;
	}

	// calculate transform for line
	VuMatrix transform;
	transform.loadIdentity();
	transform.scale(VuVector3(scale.mX, scale.mY, 1));
	transform.translate(VuVector3(posX, posY, 0));

	// transform horizontal clipping bounds to font space
	float leftBound = (clipRect.getLeft() - posX)/scale.mX;
	float rightBound = (clipRect.getRight() - posX)/scale.mX;

	// use scratch pad to build verts (in screen-space, so that trivial rejection is simplified)
	void *pScratchPad = VuScratchPad::get(VuScratchPad::GRAPHICS);
	DrawVertex *pVert = (DrawVertex *)pScratchPad;
	int remainingSpace = VuScratchPad::SIZE;

	float caretPos = 0;
	int quadCount = 0;
	while ( bufPos < mRenderCharIndexBuffer.size() )
	{
		VUUINT16 charIndex = mRenderCharIndexBuffer[bufPos++];
		if ( charIndex >= INDEX_SPECIAL_START )
		{
			if ( charIndex == INDEX_NEWLINE )
				break;
			else if ( charIndex == INDEX_COLOR_PREV )
				context.mCurColor = params.mColor;
			else if ( charIndex >= INDEX_COLOR_LO && charIndex <= INDEX_COLOR_HI )
				context.mCurColor = context.mColorBuffer[charIndex - INDEX_COLOR_LO];
		}
		else
		{
			const VuFont::CharEntry *pChar = &pFont->mCharacters[charIndex];

			// if we've gone past the right clipping bounds, we're done (not exact, but fast)
			if ( caretPos > rightBound )
			{
				bufPos = skipLine(context, params, bufPos);
				break;
			}

			// only consider characters to the right of left clipping bounds (not exact, but fast)
			if ( caretPos + pChar->mAdvance >= leftBound )
			{
				// add verts for char
				if ( remainingSpace >= 4*sizeof(DrawVertex) )
				{
					// src (tex) coords
					pVert[0].mUv[0] = pChar->mSrcL; pVert[0].mUv[1] = pChar->mSrcT;
					pVert[1].mUv[0] = pChar->mSrcL; pVert[1].mUv[1] = pChar->mSrcB;
					pVert[2].mUv[0] = pChar->mSrcR; pVert[2].mUv[1] = pChar->mSrcB;
					pVert[3].mUv[0] = pChar->mSrcR; pVert[3].mUv[1] = pChar->mSrcT;

					// dst (screen) coords
					pVert[0].mXyz[0] = caretPos + pChar->mDstL; pVert[0].mXyz[1] = pChar->mDstT; pVert[0].mXyz[2] = depth;
					pVert[1].mXyz[0] = caretPos + pChar->mDstL; pVert[1].mXyz[1] = pChar->mDstB; pVert[1].mXyz[2] = depth;
					pVert[2].mXyz[0] = caretPos + pChar->mDstR; pVert[2].mXyz[1] = pChar->mDstB; pVert[2].mXyz[2] = depth;
					pVert[3].mXyz[0] = caretPos + pChar->mDstR; pVert[3].mXyz[1] = pChar->mDstT; pVert[3].mXyz[2] = depth;

					// handle slanted text
					pVert[0].mXyz[0] -= params.mSlant*pChar->mDstT;
					pVert[1].mXyz[0] -= params.mSlant*pChar->mDstB;
					pVert[2].mXyz[0] -= params.mSlant*pChar->mDstB;
					pVert[3].mXyz[0] -= params.mSlant*pChar->mDstT;

					// color
					pVert[0].mColor = context.mCurColor;
					pVert[1].mColor = context.mCurColor;
					pVert[2].mColor = context.mCurColor;
					pVert[3].mColor = context.mCurColor;

					if ( pChar->mIsImage )
					{
						// this is an image (rare case)
						// add an image to the image buffer (to be drawn later during end() call)
						const VuFont::ImageEntry *pImage = &pFont->mImages[pChar->mImageIndex];

						ImageBufferEntry entry;
						entry.mTransform = transform;
						VU_MEMCPY(entry.mVerts, sizeof(entry.mVerts), pVert, 4*sizeof(DrawVertex));
						entry.mpTexture = pImage->mpTextureAsset->getTexture();

						mImageBuffer.push_back(entry);
					}
					else
					{
						pVert += 4;
						remainingSpace -= 4*sizeof(DrawVertex);
						quadCount += 1;
					}
				}
			}

			caretPos += pChar->mAdvance;
		}
	}

	if ( pVert > (DrawVertex *)pScratchPad )
	{
		// set transform
		VuShaderProgram *pSP = mpCurFlavor->mpMaterial->mpShaderProgram;
		pSP->setConstantMatrix(mpCurFlavor->mhConstTransform, transform);

		const VUUINT16 *pIndexData = VuGfxUtil::IF()->getQuadIndexBuffer(quadCount);

		VuGfx::IF()->drawIndexedPrimitiveUP(
			VUGFX_PT_TRIANGLELIST,	// PrimitiveType
			0,						// MinVertexIndex
			quadCount*4,			// NumVertices
			quadCount*2,			// PrimitiveCount
			pIndexData,				// IndexData
			pScratchPad				// VertexStreamZeroData
		);
	}
}

//*****************************************************************************
VuRect VuFontDraw::measureLine(Context &context, const VuFontDrawParams &params, const VuRect &rect, int flags, float posY, int &bufPos, float offsetX)
{
	VuRect lineRect(1e9f,1e9f,-2e9f,-2e9f);

	const VuFont *pFont = context.mpFont;
	const VuRect &clipRect = context.mClipRect;
	const VuVector2 &scale = context.mScale;
	const VuVector2 &extraClipSize = context.mExtraClipSize;
	const CharIndexBuffer &buffer = *context.mpCharIndexBuffer;

	// check for empty line
	if ( buffer[bufPos] == INDEX_NEWLINE )
	{
		bufPos++;
		return lineRect;
	}

	// vertical trivial rejection of whole line
	if ( (posY - scale.mY*(pFont->mDescender - extraClipSize.mY) < clipRect.getTop()) ||
		 (posY - scale.mY*(pFont->mAscender + extraClipSize.mY) > clipRect.getBottom()) )
	{
		bufPos = skipLine(context, params, bufPos);
		return lineRect;
	}

	// determine line witdh
	float lineWidth = calcLineWidth(context, bufPos);

	// calculate horizontal starting position, handling horz alignment
	float posX = calcHorzStart(rect, flags, lineWidth);
	posX += offsetX;

	// trivial horizontal rejection of whole line
	if ( posX + lineWidth < clipRect.getLeft() || posX > clipRect.getRight() )
	{
		bufPos = skipLine(context, params, bufPos);
		return lineRect;
	}

	// calculate transform for line
	VuMatrix transform;
	transform.loadIdentity();
	transform.scale(VuVector3(scale.mX, scale.mY, 1));
	transform.translate(VuVector3(posX, posY, 0));

	// transform horizontal clipping bounds to font space
	float leftBound = (clipRect.getLeft() - posX)/scale.mX;
	float rightBound = (clipRect.getRight() - posX)/scale.mX;

	float caretPos = 0;
//	int quadCount = 0;
	while ( bufPos < buffer.size() )
	{
		VUUINT16 charIndex = buffer[bufPos++];
		if ( charIndex >= INDEX_SPECIAL_START )
		{
			if ( charIndex == INDEX_NEWLINE )
				break;
		}
		else
		{
			const VuFont::CharEntry *pChar = &pFont->mCharacters[charIndex];

			// if we've gone past the right clipping bounds, we're done (not exact, but fast)
			if ( caretPos > rightBound )
			{
				bufPos = skipLine(context, params, bufPos);
				break;
			}

			// only consider characters to the right of left clipping bounds (not exact, but fast)
			if ( caretPos + pChar->mAdvance >= leftBound )
			{
				// dst (screen) coords
				float x0 = caretPos + pChar->mDstL - params.mSlant*pChar->mDstT;
				float x1 = caretPos + pChar->mDstL - params.mSlant*pChar->mDstB;
				float x2 = caretPos + pChar->mDstR - params.mSlant*pChar->mDstB;
				float x3 = caretPos + pChar->mDstR - params.mSlant*pChar->mDstT;

				VuRect charRect;
				charRect.mX = VuMin(x0, x1);
				charRect.mY = pChar->mDstT;
				charRect.mWidth = VuMax(x2, x3) - charRect.mX;
				charRect.mHeight = pChar->mDstB - charRect.mY;

				lineRect.add(charRect);
			}

			caretPos += pChar->mAdvance;
		}
	}

	// transform to screen
	if ( lineRect.isValid() )
	{
		lineRect *= scale;
		lineRect += VuVector2(posX, posY);
	}

	return lineRect;
}

//*****************************************************************************
int VuFontDraw::skipLine(Context &context, const VuFontDrawParams &params, int bufPos)
{
	const CharIndexBuffer &buffer = *context.mpCharIndexBuffer;

	while ( bufPos < buffer.size() )
	{
		VUUINT16 charIndex = buffer[bufPos++];
		if ( charIndex >= INDEX_SPECIAL_START )
		{
			if ( charIndex == INDEX_NEWLINE )
				break;
			else if ( charIndex == INDEX_COLOR_PREV )
				context.mCurColor = params.mColor;
			else if ( charIndex >= INDEX_COLOR_LO && charIndex <= INDEX_COLOR_HI )
				context.mCurColor = context.mColorBuffer[charIndex - INDEX_COLOR_LO];
		}
	}

	return bufPos;
}

//*****************************************************************************
void VuFontDraw::drawImages()
{
	if ( mImageBuffer.size() == 0 )
		return;

	VuFontShaderFlavor *pPrevFlavor = mpCurFlavor;

	mpCurFlavor = &mpShaderFlavors[FLAVOR_DRAW_IMAGE];
	VuPipelineState *pPS = mpCurFlavor->mpMaterial->mpPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	VuShaderProgram *pSP = pPS->mpShaderProgram;

	for ( ImageBufferEntry *pIBE = &mImageBuffer.begin(); pIBE != &mImageBuffer.end(); pIBE++ )
	{
		VuGfx::IF()->setTexture(0, pIBE->mpTexture);
		pSP->setConstantMatrix(mpCurFlavor->mhConstTransform, pIBE->mTransform);

		const VUUINT16 *pIndexData = VuGfxUtil::IF()->getQuadIndexBuffer(1);
		VuGfx::IF()->drawIndexedPrimitiveUP(VUGFX_PT_TRIANGLELIST, 0, 4, 2, pIndexData, pIBE->mVerts);
	}

	// clean up
	mpCurFlavor = pPrevFlavor;
	pPS = mpCurFlavor->mpMaterial->mpPipelineState;
	VuGfx::IF()->setPipelineState(pPS);

	mImageBuffer.clear();
}

//*****************************************************************************
void VuFontDraw::buildCaseLookupArrays()
{
	// lower to upper case
	smLowerToUpperCaseLookup.resize(256);
	for ( int i = 0; i < 256; i++ )
		smLowerToUpperCaseLookup[i] = i;

	smLowerToUpperCaseLookup[0x61] = 0x41; // A
	smLowerToUpperCaseLookup[0x62] = 0x42; // B
	smLowerToUpperCaseLookup[0x63] = 0x43; // C
	smLowerToUpperCaseLookup[0x64] = 0x44; // D
	smLowerToUpperCaseLookup[0x65] = 0x45; // E
	smLowerToUpperCaseLookup[0x66] = 0x46; // F
	smLowerToUpperCaseLookup[0x67] = 0x47; // G
	smLowerToUpperCaseLookup[0x68] = 0x48; // H
	smLowerToUpperCaseLookup[0x69] = 0x49; // I
	smLowerToUpperCaseLookup[0x6A] = 0x4A; // J
	smLowerToUpperCaseLookup[0x6B] = 0x4B; // K
	smLowerToUpperCaseLookup[0x6C] = 0x4C; // L
	smLowerToUpperCaseLookup[0x6D] = 0x4D; // M
	smLowerToUpperCaseLookup[0x6E] = 0x4E; // N
	smLowerToUpperCaseLookup[0x6F] = 0x4F; // O
	smLowerToUpperCaseLookup[0x70] = 0x50; // P
	smLowerToUpperCaseLookup[0x71] = 0x51; // Q
	smLowerToUpperCaseLookup[0x72] = 0x52; // R
	smLowerToUpperCaseLookup[0x73] = 0x53; // S
	smLowerToUpperCaseLookup[0x74] = 0x54; // T
	smLowerToUpperCaseLookup[0x75] = 0x55; // U
	smLowerToUpperCaseLookup[0x76] = 0x56; // V
	smLowerToUpperCaseLookup[0x77] = 0x57; // W
	smLowerToUpperCaseLookup[0x78] = 0x58; // X
	smLowerToUpperCaseLookup[0x79] = 0x59; // Y
	smLowerToUpperCaseLookup[0x7A] = 0x5A; // Z
	smLowerToUpperCaseLookup[0xE0] = 0xC0; // A GRAVE
	smLowerToUpperCaseLookup[0xE1] = 0xC1; // A ACUTE
	smLowerToUpperCaseLookup[0xE2] = 0xC2; // A CIRCUMFLEX
	smLowerToUpperCaseLookup[0xE3] = 0xC3; // A TILDE
	smLowerToUpperCaseLookup[0xE4] = 0xC4; // A DIAERESIS
	smLowerToUpperCaseLookup[0xE5] = 0xC5; // A RING
	smLowerToUpperCaseLookup[0xE6] = 0xC6; // A E
	smLowerToUpperCaseLookup[0xE7] = 0xC7; // C CEDILLA
	smLowerToUpperCaseLookup[0xE8] = 0xC8; // E GRAVE
	smLowerToUpperCaseLookup[0xE9] = 0xC9; // E ACUTE
	smLowerToUpperCaseLookup[0xEA] = 0xCA; // E CIRCUMFLEX
	smLowerToUpperCaseLookup[0xEB] = 0xCB; // E DIAERESIS
	smLowerToUpperCaseLookup[0xEC] = 0xCC; // I GRAVE
	smLowerToUpperCaseLookup[0xED] = 0xCD; // I ACUTE
	smLowerToUpperCaseLookup[0xEE] = 0xCE; // CIRCUMFLEX
	smLowerToUpperCaseLookup[0xEF] = 0xCF; // DIAERESIS
	smLowerToUpperCaseLookup[0xF0] = 0xD0; // ETH
	smLowerToUpperCaseLookup[0xF1] = 0xD1; // N TILDE
	smLowerToUpperCaseLookup[0xF2] = 0xD2; // O GRAVE
	smLowerToUpperCaseLookup[0xF3] = 0xD3; // O ACUTE
	smLowerToUpperCaseLookup[0xF4] = 0xD4; // O CIRCUMFLEX
	smLowerToUpperCaseLookup[0xF5] = 0xD5; // O TILDE
	smLowerToUpperCaseLookup[0xF6] = 0xD6; // O DIAERESIS
	smLowerToUpperCaseLookup[0xF8] = 0xD8; // O SLASH
	smLowerToUpperCaseLookup[0xF9] = 0xD9; // U GRAVE
	smLowerToUpperCaseLookup[0xFA] = 0xDA; // U ACUTE
	smLowerToUpperCaseLookup[0xFB] = 0xDB; // U CIRCUMFLEX
	smLowerToUpperCaseLookup[0xFC] = 0xDC; // U DIAERESIS
	smLowerToUpperCaseLookup[0xFD] = 0xDD; // Y ACUTE
	smLowerToUpperCaseLookup[0xFE] = 0xDE; // THORN


	// upper to lower case

	smUpperToLowerCaseLookup.resize(256);
	for ( int i = 0; i < 256; i++ )
		smUpperToLowerCaseLookup[i] = i;

	smUpperToLowerCaseLookup[0x41] = 0x61; // A
	smUpperToLowerCaseLookup[0x42] = 0x62; // B
	smUpperToLowerCaseLookup[0x43] = 0x63; // C
	smUpperToLowerCaseLookup[0x44] = 0x64; // D
	smUpperToLowerCaseLookup[0x45] = 0x65; // E
	smUpperToLowerCaseLookup[0x46] = 0x66; // F
	smUpperToLowerCaseLookup[0x47] = 0x67; // G
	smUpperToLowerCaseLookup[0x48] = 0x68; // H
	smUpperToLowerCaseLookup[0x49] = 0x69; // I
	smUpperToLowerCaseLookup[0x4A] = 0x6A; // J
	smUpperToLowerCaseLookup[0x4B] = 0x6B; // K
	smUpperToLowerCaseLookup[0x4C] = 0x6C; // L
	smUpperToLowerCaseLookup[0x4D] = 0x6D; // M
	smUpperToLowerCaseLookup[0x4E] = 0x6E; // N
	smUpperToLowerCaseLookup[0x4F] = 0x6F; // O
	smUpperToLowerCaseLookup[0x50] = 0x70; // P
	smUpperToLowerCaseLookup[0x51] = 0x71; // Q
	smUpperToLowerCaseLookup[0x52] = 0x72; // R
	smUpperToLowerCaseLookup[0x53] = 0x73; // S
	smUpperToLowerCaseLookup[0x54] = 0x74; // T
	smUpperToLowerCaseLookup[0x55] = 0x75; // U
	smUpperToLowerCaseLookup[0x56] = 0x76; // V
	smUpperToLowerCaseLookup[0x57] = 0x77; // W
	smUpperToLowerCaseLookup[0x58] = 0x78; // X
	smUpperToLowerCaseLookup[0x59] = 0x79; // Y
	smUpperToLowerCaseLookup[0x5A] = 0x7A; // Z
	smUpperToLowerCaseLookup[0xC0] = 0xE0; // A GRAVE
	smUpperToLowerCaseLookup[0xC1] = 0xE1; // A ACUTE
	smUpperToLowerCaseLookup[0xC2] = 0xE2; // A CIRCUMFLEX
	smUpperToLowerCaseLookup[0xC3] = 0xE3; // A TILDE
	smUpperToLowerCaseLookup[0xC4] = 0xE4; // A DIAERESIS
	smUpperToLowerCaseLookup[0xC5] = 0xE5; // A RING
	smUpperToLowerCaseLookup[0xC6] = 0xE6; // A E
	smUpperToLowerCaseLookup[0xC7] = 0xE7; // C CEDILLA
	smUpperToLowerCaseLookup[0xC8] = 0xE8; // E GRAVE
	smUpperToLowerCaseLookup[0xC9] = 0xE9; // E ACUTE
	smUpperToLowerCaseLookup[0xCA] = 0xEA; // E CIRCUMFLEX
	smUpperToLowerCaseLookup[0xCB] = 0xEB; // E DIAERESIS
	smUpperToLowerCaseLookup[0xCC] = 0xEC; // I GRAVE
	smUpperToLowerCaseLookup[0xCD] = 0xED; // I ACUTE
	smUpperToLowerCaseLookup[0xCE] = 0xEE; // I CIRCUMFLEX
	smUpperToLowerCaseLookup[0xCF] = 0xEF; // I DIAERESIS
	smUpperToLowerCaseLookup[0xD0] = 0xF0; // ETH
	smUpperToLowerCaseLookup[0xD1] = 0xF1; // N TILDE
	smUpperToLowerCaseLookup[0xD2] = 0xF2; // O GRAVE
	smUpperToLowerCaseLookup[0xD3] = 0xF3; // O ACUTE
	smUpperToLowerCaseLookup[0xD4] = 0xF4; // O CIRCUMFLEX
	smUpperToLowerCaseLookup[0xD5] = 0xF5; // O TILDE
	smUpperToLowerCaseLookup[0xD6] = 0xF6; // O DIAERESIS
	smUpperToLowerCaseLookup[0xD8] = 0xF8; // O SLASH
	smUpperToLowerCaseLookup[0xD9] = 0xF9; // U GRAVE
	smUpperToLowerCaseLookup[0xDA] = 0xFA; // U ACUTE
	smUpperToLowerCaseLookup[0xDB] = 0xFB; // U CIRCUMFLEX
	smUpperToLowerCaseLookup[0xDC] = 0xFC; // U DIAERESIS
	smUpperToLowerCaseLookup[0xDD] = 0xFD; // Y ACUTE
	smUpperToLowerCaseLookup[0xDE] = 0xFE; // THORN
}

//*****************************************************************************
VuFontShaderFlavor::VuFontShaderFlavor():
	mpMaterial(VUNULL),
	mhConstTransform(VUNULL),
	mhConstDistMinMax(VUNULL),
	mhConstOutlineMinMax(VUNULL),
	mhConstOutlineColor(VUNULL)
{
}

//*****************************************************************************
VuFontShaderFlavor::~VuFontShaderFlavor()
{
	VuGfxSort::IF()->releaseMaterial(mpMaterial);
}

//*****************************************************************************
bool VuFontShaderFlavor::create(const char *shaderName, const VuVertexDeclarationParams &vdParams)
{
	VuCompiledShaderAsset *pShaderAsset = VuAssetFactory::IF()->createAsset<VuCompiledShaderAsset>(shaderName);

	// create vertex declaration
	VuVertexDeclaration *pVD = VuGfx::IF()->createVertexDeclaration(vdParams, pShaderAsset->getShaderProgram());

	// create pipeline state
	VuPipelineStateParams psParams;
	psParams.mAlphaBlendEnabled = true;
	VuPipelineState *pPS = VuGfx::IF()->createPipelineState(pShaderAsset->getShaderProgram(), pVD, psParams);

	// create material
	VuGfxSortMaterialDesc desc;
	mpMaterial = VuGfxSort::IF()->createMaterial(pPS, desc);

	// clean up
	VuAssetFactory::IF()->releaseAsset(pShaderAsset);
	pVD->removeRef();
	pPS->removeRef();

	// get shader constants
	VuShaderProgram *pSP = mpMaterial->mpShaderProgram;
	mhConstTransform = pSP->getConstantByName("gTransform");
	mhConstDistMinMax = pSP->getConstantByName("gDistMinMax");
	mhConstOutlineMinMax = pSP->getConstantByName("gOutlineMinMax");
	mhConstOutlineColor = pSP->getConstantByName("gOutlineColor");

	return true;
}
