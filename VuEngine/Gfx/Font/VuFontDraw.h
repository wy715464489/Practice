//*****************************************************************************
//
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  VuFontDraw class
// 
//  Special character codes:
//
//  {[r,g,b]}	- change font color on-the-fly
//  {[]}		- change back to color specified in font draw params
// 
//*****************************************************************************

#pragma once

#include "VuFontDrawParams.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuRect.h"
#include "VuEngine/Gfx/GfxSort/VuGfxSort.h"

class VuFont;
class VuVertexDeclaration;
class VuFontShaderFlavor;
class VuMatrix;
class VuTexture;
class VuFontDrawVert;
struct VuFontDrawData;

class VuFontDraw
{
public:
	VuFontDraw();
	~VuFontDraw();

	bool				init();
	void				release();

	// drawing
	void				drawString(float depth, VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, const VuRect &rect, int flags, float alpha = 1.0f, const VuVector2 &offset = VuVector2(0,0), VUUINT transType = VuGfxSort::TRANS_UI_MODULATE);
	static VuRect		measureString(VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, const VuRect &rect, int flags, float renderTargetAspectRatio, const VuVector2 &offset = VuVector2(0,0));

	// measuring
	static VuVector2	measureString(VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, float rectWidth, int flags, float renderTargetAspectRatio);
	static float		measureStringWidth(VuFont *pFont, const char *strTextUtf8, const VuFontDrawParams &params, float renderTargetAspectRatio);
	static float		measureStringWidth(VuFont *pFont, const wchar_t *strText16, const VuFontDrawParams &params, float renderTargetAspectRatio);

	// macros
	class VuMacroHandler { public: virtual bool	handleMacro(const char *macro, std::string &text) = 0; };
	void				addMacroHandler(VuMacroHandler *pHandler);
	void				removeMacroHandler(VuMacroHandler *pHandler);
	void				handleMacro(const char *macro, std::string &text);

private:
	struct DrawVertex
	{
		float		mXyz[3];
		float		mUv[2];
		VUUINT32	mColor;
	};

	struct ImageBufferEntry
	{
		VuMatrix	mTransform;
		DrawVertex	mVerts[4];
		VuTexture	*mpTexture;
	};

	typedef VuArray<VUUINT16> CharIndexBuffer;
	typedef VuArray<ImageBufferEntry> ImageBuffer;
	typedef VuArray<VuColor> ColorBuffer;

	enum { MAX_COLOR_BUFFER_SIZE = 10 };

	struct Context
	{
		Context() : mColorBufferSize(0) {}
		VuFont			*mpFont;
		VuRect			mClipRect;
		VuVector2		mScale;
		VuVector2		mExtraClipSize;
		CharIndexBuffer	*mpCharIndexBuffer;
		VuColor			mCurColor;
		VuColor			mColorBuffer[MAX_COLOR_BUFFER_SIZE];
		int				mColorBufferSize;
	};


	static void			submitDrawCommandsCallback(void *pData);
	void				submitDrawCommands(const VuFontDrawData *pDrawData, const char *strTextUtf8);
	static void			buildCharIndexBuffer(Context &context, const char *strTextUtf8, int tabSize, int flags);
	static VUUINT16		addToColorBuffer(Context &context, const VuColor &color);
	static const char	*handleWordBreak(const VuFont *pFont, const char *strTextUtf8, float width, std::wstring &unicodeBuffer, std::string &utf8Buffer);
	static VUUINT		wordBreakGetWidthCB(wchar_t, const void *pUserData);
	static const char	*handleMacros(const char *strTextUtf8, std::string &utf8Buffer);
	static const char	*forceUpperCase(const char *strTextUtf8, std::string &utf8Buffer);
	static const char	*forceLowerCase(const char *strTextUtf8, std::string &utf8Buffer);
	static float		calcVertStart(Context &context, const VuRect &rect, int flags);
	static float		calcLineWidth(Context &context, int bufPos);
	static float		calcHorzStart(const VuRect &rect, int flags, float lineWidth);
	void				drawLine(Context &context, const VuFontDrawParams &params, const VuRect &rect, int flags, float posY, int &bufPos, float depth, float offsetX);
	static VuRect		measureLine(Context &context, const VuFontDrawParams &params, const VuRect &rect, int flags, float posY, int &bufPos, float offsetX);
	static int			skipLine(Context &context, const VuFontDrawParams &params, int bufPos);
	void				drawImages();
	void				buildCaseLookupArrays();

	// parameters
	VuFontShaderFlavor	*mpCurFlavor;

	VuFontShaderFlavor	*mpShaderFlavors;
	CharIndexBuffer		mRenderCharIndexBuffer;
	std::wstring		mRenderTempUnicodeBuffer;
	std::string			mRenderTempUtf8Buffer;
	ImageBuffer			mImageBuffer;

	typedef std::vector<VuMacroHandler *> MacroHandlers;
	MacroHandlers		mMacroHandlers;

	// static members
	static CharIndexBuffer	smMeasureCharIndexBuffer;
	static std::wstring		smTempUnicodeBuffer;
	static std::string		smTempUtf8Buffers[2];

	typedef std::vector<VUUINT32> UnicodeArray;
	static UnicodeArray	smLowerToUpperCaseLookup;
	static UnicodeArray	smUpperToLowerCaseLookup;
};
