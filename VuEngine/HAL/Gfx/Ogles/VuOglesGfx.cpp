//*****************************************************************************
//
//  Copyright (c) 2010-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpenGL ES interface class for Gfx.
//
//*****************************************************************************

#include "VuOglesGfx.h"
#include "VuOglesGfxTypes.h"
#include "VuOglesTexture.h"
#include "VuOglesCubeTexture.h"
#include "VuOglesIndexBuffer.h"
#include "VuOglesVertexBuffer.h"
#include "VuOglesRenderTarget.h"
#include "VuOglesDepthRenderTarget.h"
#include "VuOglesShadowRenderTarget.h"
#include "VuOglesShaderProgram.h"
#include "VuOglesVertexDeclaration.h"
#include "VuOglesPipelineState.h"
#include "VuOglesDepthStencilState.h"
#include "VuEngine/VuEngine.h"
#include "VuEngine/Gfx/VuGfxUtil.h"
#include "VuEngine/Gfx/PostProcess/VuPostProcess.h"
#include "VuEngine/Math/VuVector4.h"
#include "VuEngine/Json/VuJsonContainer.h"


//*****************************************************************************
VuOglesGfx::VuOglesGfx():
	mDisplayWidth(0),
	mDisplayHeight(0),
	mCurRenderTargetWidth(0),
	mCurRenderTargetHeight(0),
	mCurViewport(0,0,1,1),
	mpCurVertexDeclaration(VUNULL),
	mpCurVertexBuffer(VUNULL),
	mpCurIndexBuffer(VUNULL),
	mPrevVertexBufferId(-1),
	mPrevIndexBufferId(-1),
	mCurVertexStride(0),
	mGlVersion(2),
	mDxtCompression(false),
	mPvrtcCompression(false),
	mContextDestroyed(false),
	mDefaultFramebuffer(0)
{
}

//*****************************************************************************
VuOglesGfx::~VuOglesGfx()
{
}

//*****************************************************************************
bool VuOglesGfx::init(VUHANDLE hWindow, VUHANDLE hDevice)
{
	if ( !VuGfx::init(hWindow, hDevice) )
		return false;

	// OpenGL ES version
	if ( VuEngine::IF()->options().mGfxApi == "OpenGL ES 3" )
		mGlVersion = 3;
	VUPRINTF("OpenGL ES version = %d\n", mGlVersion);

	// info
	if ( const char *str = (const char *)glGetString(GL_VENDOR) )
		VUPRINTF("GL_VENDOR = %s\n", str);

	if ( const char *str = (const char *)glGetString(GL_RENDERER) )
		VUPRINTF("GL_RENDERER = %s\n", str);

	if ( const char *str = (const char *)glGetString(GL_VERSION) )
		VUPRINTF("GL_VERSION = %s\n", str);

	if ( const char *str = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION) )
		VUPRINTF("GL_SHADING_LANGUAGE_VERSION = %s\n", str);

	// get extensions
	if ( const char *strExt = (const char *)glGetString(GL_EXTENSIONS) )
	{
		VUPRINTF("OpenGL Extensions:");
		char *str = new char[strlen(strExt) + 1];
		strcpy(str, strExt);

		const char *p = strtok(str, " ");
		while ( p )
		{
			VUPRINTF(" %s", p);
			mExtensions.insert(p);
			p = strtok(VUNULL, " ");
		}
		VUPRINTF("\n");
	}

	// info
	GLint maxTextureSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	VUPRINTF("GL_MAX_TEXTURE_SIZE = %d\n", maxTextureSize);

	GLint maxCubeMapTextureSize;
	glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &maxCubeMapTextureSize);
	VUPRINTF("GL_MAX_CUBE_MAP_TEXTURE_SIZE = %d\n", maxCubeMapTextureSize);

	GLint maxVaryingVectors;
	glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryingVectors);
	VUPRINTF("GL_MAX_VARYING_VECTORS = %d\n", maxVaryingVectors);

	GLint maxVertexAttribs;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	VUPRINTF("GL_MAX_VERTEX_ATTRIBS = %d\n", maxVertexAttribs);

	mDxtCompression = VuOglesGfx::IF()->getExtension("GL_EXT_texture_compression_dxt1") && VuOglesGfx::IF()->getExtension("GL_EXT_texture_compression_s3tc");
	mPvrtcCompression = VuOglesGfx::IF()->getExtension("GL_IMG_texture_compression_pvrtc");

	// initial display size
	int viewport[4] = {0,0,0,0};
	glGetIntegerv(GL_VIEWPORT, viewport);
	mDisplayWidth = viewport[2];
	mDisplayHeight = viewport[3];

	VUPRINTF("Display size: %d x %d\n", mDisplayWidth, mDisplayHeight);

	mCurRenderTargetWidth = mDisplayWidth;
	mCurRenderTargetHeight = mDisplayHeight;

	return true;
}

//*****************************************************************************
void VuOglesGfx::release()
{
	VU_SAFE_RELEASE(mpCurVertexDeclaration);
	VU_SAFE_RELEASE(mpCurVertexBuffer);
	VU_SAFE_RELEASE(mpCurIndexBuffer);
}

//*****************************************************************************
void VuOglesGfx::resize(VUHANDLE hDisplay, int width, int height)
{
	if ( width > 0 && height > 0 )
	{
		if ( mDisplayWidth != width || mDisplayHeight != height )
		{
			mDisplayWidth = width;
			mDisplayHeight = height;

			mCurRenderTargetWidth = mDisplayWidth;
			mCurRenderTargetHeight = mDisplayHeight;
			mCurViewport = VuRect(0,0,1,1);
			glViewport(0, 0, mDisplayWidth, mDisplayHeight);

			VUPRINTF("Display size: %d x %d\n", mDisplayWidth, mDisplayHeight);
		}
	}
}

//*****************************************************************************
void VuOglesGfx::getDisplaySize(VUHANDLE hDisplay, int &width, int &height)
{
	width = mDisplayWidth;
	height = mDisplayHeight;
}

//*****************************************************************************
VuRenderTarget *VuOglesGfx::createRenderTarget(int width, int height)
{
	return VuOglesRenderTarget::create(width, height);
}

//*****************************************************************************
VuDepthRenderTarget *VuOglesGfx::createDepthRenderTarget(int width, int height)
{
	return VuOglesDepthRenderTarget::create(width, height);
}

//*****************************************************************************
VuShadowRenderTarget *VuOglesGfx::createShadowRenderTarget(int width, int height, int count)
{
	return VuOglesShadowRenderTarget::create(width, height, count);
}

//*****************************************************************************
VuFxRenderTarget *VuOglesGfx::createFxRenderTarget(int width, int height, VuGfxFormat format)
{
	VUASSERT(0, "Not Implemented!");
	return VUNULL;
}

//*****************************************************************************
VuVertexBuffer *VuOglesGfx::createVertexBuffer(int size, VUUINT32 usageFlags)
{
	return VuOglesVertexBuffer::create(size, usageFlags);
}

//*****************************************************************************
VuIndexBuffer *VuOglesGfx::createIndexBuffer(int count, VUUINT32 usageFlags)
{
	return VuOglesIndexBuffer::create(count, usageFlags);
}

//*****************************************************************************
VuVertexDeclaration *VuOglesGfx::createVertexDeclaration(const VuVertexDeclarationParams &params, VuShaderProgram *pShaderProgram)
{
	return VuOglesVertexDeclaration::create(params, pShaderProgram);
}

//*****************************************************************************
VuTexture *VuOglesGfx::createTexture(int width, int height, VUUINT32 usageFlags, VuGfxFormat format, const VuTextureState &state)
{
	return VuOglesTexture::create(width, height, usageFlags, format, state);
}

//*****************************************************************************
VuTexture *VuOglesGfx::loadTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuOglesTexture::load(reader, skipLevels);
}

//*****************************************************************************
VuCubeTexture *VuOglesGfx::loadCubeTexture(VuBinaryDataReader &reader, int skipLevels)
{
	return VuOglesCubeTexture::load(reader, skipLevels);
}

//*****************************************************************************
VuShaderProgram *VuOglesGfx::loadShaderProgram(VuBinaryDataReader &reader)
{
	return VuOglesShaderProgram::load(reader);
}

//*****************************************************************************
VuPipelineState *VuOglesGfx::createPipelineState(VuShaderProgram *pSP, VuVertexDeclaration *pVD, const VuPipelineStateParams &params)
{
	return VuOglesPipelineState::create(pSP, pVD, params);
}

//*****************************************************************************
VuDepthStencilState *VuOglesGfx::createDepthStencilState(const VuDepthStencilStateParams &params)
{
	return VuOglesDepthStencilState::create(params);
}

//*****************************************************************************
void VuOglesGfx::getCurRenderTargetSize(int &width, int &height)
{
	width = mCurRenderTargetWidth;
	height = mCurRenderTargetHeight;
}

//*****************************************************************************
void VuOglesGfx::setDepthRenderTarget(VuDepthRenderTarget *pDepthRenderTarget)
{
	VuOglesDepthRenderTarget *pOglesDepthRenderTarget = (VuOglesDepthRenderTarget *)pDepthRenderTarget;
	glBindFramebuffer(GL_FRAMEBUFFER, pOglesDepthRenderTarget->mGlFramebuffer);

	checkFramebufferStatus();
	
	int width = pDepthRenderTarget->getWidth();
	int height = pDepthRenderTarget->getHeight();
	mCurRenderTargetWidth = width;
	mCurRenderTargetHeight = height;
	glViewport(0, 0, width, height);

	mCurViewport = VuRect(0,0,1,1);

	for ( int i = 0; i < 8; i++ )
		VuGfx::IF()->setTexture(i, VUNULL);

	// handle clear
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

//*****************************************************************************
void VuOglesGfx::setShadowRenderTarget(VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	VuOglesShadowRenderTarget *pOglesShadowRenderTarget = (VuOglesShadowRenderTarget *)pShadowRenderTarget;
	glBindFramebuffer(GL_FRAMEBUFFER, pOglesShadowRenderTarget->mFramebuffers[layer]);

	checkFramebufferStatus();

	int width = pShadowRenderTarget->getWidth();
	int height = pShadowRenderTarget->getHeight();
	mCurRenderTargetWidth = width;
	mCurRenderTargetHeight = height;
	glViewport(0, 0, width, height);

	mCurViewport = VuRect(0,0,1,1);

	for ( int i = 0; i < 8; i++ )
		VuGfx::IF()->setTexture(i, VUNULL);

	// handle clear
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

//*****************************************************************************
void VuOglesGfx::setFxRenderTarget(VuFxRenderTarget *pFxRenderTarget)
{
	VUASSERT(0, "Not Implemented!");
}

//*****************************************************************************
bool VuOglesGfx::clear(VUUINT32 flags, const VuColor &color, float depth)
{
	VuVector4 vColor = color.toVector4();

	GLbitfield mask = 0;
	if ( flags & VUGFX_CLEAR_COLOR )
	{
		mask |= GL_COLOR_BUFFER_BIT;
		glClearColor(vColor.mX, vColor.mY, vColor.mZ, vColor.mW);
	}
	if ( flags & VUGFX_CLEAR_DEPTH )
	{
		mask |= GL_DEPTH_BUFFER_BIT;
		glClearDepthf(depth);
	}

	glClear(mask);

	return true;
}

//*****************************************************************************
bool VuOglesGfx::setViewport(const VuRect &rect)
{
	if ( mCurViewport != rect )
	{
		VuRect screenRect = rect;
		screenRect.mY = 1.0f - rect.getBottom();
		screenRect *= VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);

		GLint x = VuRound(screenRect.mX);
		GLint y = VuRound(screenRect.mY);
		GLsizei width = VuRound(screenRect.mWidth);
		GLsizei height = VuRound(screenRect.mHeight);

		glViewport(x, y, width, height);

		mCurViewport = rect;
	}

	return true;
}

//*****************************************************************************
bool VuOglesGfx::setScissorRect(const VuRect *pRect)
{
	if ( pRect )
	{
		VuRect screenRect = (*pRect)*VuVector2(mCurRenderTargetWidth, mCurRenderTargetHeight);

		int x = VuRound(screenRect.mX);
		int y = VuRound(screenRect.mY);
		int width = VuRound(screenRect.mWidth);
		int height = VuRound(screenRect.mHeight);

		glScissor(x, mCurRenderTargetHeight - y - height, width, height);
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}

	return true;
}

//*****************************************************************************
bool VuOglesGfx::setVertexBuffer(VuVertexBuffer *pVertexBuffer)
{
	if ( pVertexBuffer != mpCurVertexBuffer )
	{
		VU_SAFE_RELEASE(mpCurVertexBuffer);

		mpCurVertexBuffer = (VuOglesVertexBuffer *)pVertexBuffer;
		mpCurVertexBuffer->addRef();
	}

	return true;
}

//*****************************************************************************
bool VuOglesGfx::setIndexBuffer(VuIndexBuffer *pIndexBuffer)
{
	if ( pIndexBuffer != mpCurIndexBuffer )
	{
		VU_SAFE_RELEASE(mpCurIndexBuffer);

		mpCurIndexBuffer = (VuOglesIndexBuffer *)pIndexBuffer;
		mpCurIndexBuffer->addRef();
	}

	return true;
}

//*****************************************************************************
void VuOglesGfx::setPipelineState(VuPipelineState *pPipelineState)
{
	VuOglesPipelineState *pOglesPipelineState = (VuOglesPipelineState *)pPipelineState;

	glUseProgram(pOglesPipelineState->mpOglesShaderProgram->mGlProgram);

	if ( pOglesPipelineState->mpVertexDeclaration != mpCurVertexDeclaration )
	{
		VU_SAFE_RELEASE(mpCurVertexDeclaration);

		mpCurVertexDeclaration = (VuOglesVertexDeclaration *)pOglesPipelineState->mpVertexDeclaration;
		mpCurVertexDeclaration->addRef();

		for ( int i = 0; i < mpCurVertexDeclaration->mDisabledAttribCount; i++ )
			glDisableVertexAttribArray(mpCurVertexDeclaration->mDisabledAttribs[i]);

		for ( int i = 0; i < mpCurVertexDeclaration->mEnabledAttribCount; i++ )
			glEnableVertexAttribArray(mpCurVertexDeclaration->mEnabledAttribs[i].mUsage);

		mPrevVertexBufferId = -1;

		mCurVertexStride = pPipelineState->mpVertexDeclaration->mParams.mStreams[0].mStride;
	}

	if ( pPipelineState->mParams.mAlphaBlendEnabled )
	{
		glEnable(GL_BLEND);
		glBlendFunc(pOglesPipelineState->mOglesSrcBlendFactor, pOglesPipelineState->mOglesDstBlendFactor);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	bool cwEnabled = pPipelineState->mParams.mColorWriteEnabled;
	glColorMask(cwEnabled, cwEnabled, cwEnabled, cwEnabled);
}

//*****************************************************************************
void VuOglesGfx::setDepthStencilState(VuDepthStencilState *pDepthStencilState)
{
	VuOglesDepthStencilState *pOglesDepthStencilState = (VuOglesDepthStencilState *)pDepthStencilState;

	if ( pOglesDepthStencilState->mParams.mDepthCompFunc == VUGFX_COMP_ALWAYS )
	{
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(pOglesDepthStencilState->mOglesDepthFunc);
	}

	glDepthMask(pOglesDepthStencilState->mParams.mDepthWriteEnabled);
}

//*****************************************************************************
void VuOglesGfx::setCullMode(VuGfxCullMode cullMode)
{
	if ( cullMode == VUGFX_CULL_NONE )
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(cullMode == VUGFX_CULL_CW ? GL_BACK : GL_FRONT );
	}
}

//*****************************************************************************
bool VuOglesGfx::setTexture(int sampler, VuBaseTexture *pBaseTexture)
{
	glActiveTexture(GL_TEXTURE0 + sampler);

	if ( pBaseTexture )
	{
		if ( pBaseTexture->isDerivedFrom(VuTexture::msRTTI) )
			glBindTexture(GL_TEXTURE_2D, static_cast<VuOglesTexture *>(pBaseTexture)->mGlTexture);

		else if ( pBaseTexture->isDerivedFrom(VuCubeTexture::msRTTI) )
			glBindTexture(GL_TEXTURE_CUBE_MAP, static_cast<VuOglesCubeTexture *>(pBaseTexture)->mGlTexture);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		if ( mGlVersion >= 3 )
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	return true;
}

//*****************************************************************************
bool VuOglesGfx::setDepthTexture(int sampler, VuShadowRenderTarget *pShadowRenderTarget, int layer)
{
	glActiveTexture(GL_TEXTURE0 + sampler);

	if ( pShadowRenderTarget )
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<VuOglesShadowRenderTarget *>(pShadowRenderTarget)->mGlTexture);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	return true;
}

//*****************************************************************************
void VuOglesGfx::drawPrimitive(VuGfxPrimitiveType primitiveType, int startVertex, int primitiveCount)
{
	if ( mPrevVertexBufferId != (VUUINT32)mpCurVertexBuffer )
	{
		mPrevVertexBufferId = (VUUINT32)mpCurVertexBuffer;
		glBindBuffer(GL_ARRAY_BUFFER, mpCurVertexBuffer->mGlBuffer);
		for ( int i = 0; i < mpCurVertexDeclaration->mEnabledAttribCount; i++ )
		{
			const VuOglesVertexDeclaration::VuAttrib &attrib = mpCurVertexDeclaration->mEnabledAttribs[i];
			glVertexAttribPointer(attrib.mUsage, attrib.mSize, attrib.mType, attrib.mNormalized, mCurVertexStride, (const void *)attrib.mOffset);
		}
	}

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	glDrawArrays(VuOglesGfxTypes::convert(primitiveType), startVertex, vertexCount);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuOglesGfx::drawIndexedPrimitive(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount)
{
	if ( mPrevVertexBufferId != (VUUINT32)mpCurVertexBuffer )
	{
		mPrevVertexBufferId = (VUUINT32)mpCurVertexBuffer;
		glBindBuffer(GL_ARRAY_BUFFER, mpCurVertexBuffer->mGlBuffer);
		for ( int i = 0; i < mpCurVertexDeclaration->mEnabledAttribCount; i++ )
		{
			const VuOglesVertexDeclaration::VuAttrib &attrib = mpCurVertexDeclaration->mEnabledAttribs[i];
			glVertexAttribPointer(attrib.mUsage, attrib.mSize, attrib.mType, attrib.mNormalized, mCurVertexStride, (const void *)attrib.mOffset);
		}
	}

	if ( mPrevIndexBufferId != (VUUINT32)mpCurIndexBuffer )
	{
		mPrevIndexBufferId = (VUUINT32)mpCurIndexBuffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mpCurIndexBuffer->mGlBuffer);
	}

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	glDrawElements(VuOglesGfxTypes::convert(primitiveType), vertexCount, GL_UNSIGNED_SHORT, (void *)(startIndex*2));

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuOglesGfx::drawIndexedPrimitiveVC(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int startIndex, int primitiveCount, VuVertexBuffer *pVertexColorBuffer)
{
	if ( mPrevVertexBufferId != (VUUINT32)mpCurVertexBuffer )
	{
		mPrevVertexBufferId = (VUUINT32)mpCurVertexBuffer;
		glBindBuffer(GL_ARRAY_BUFFER, mpCurVertexBuffer->mGlBuffer);
		for ( int i = 0; i < mpCurVertexDeclaration->mEnabledAttribCount; i++ )
		{
			const VuOglesVertexDeclaration::VuAttrib &attrib = mpCurVertexDeclaration->mEnabledAttribs[i];
			glVertexAttribPointer(attrib.mUsage, attrib.mSize, attrib.mType, attrib.mNormalized, mCurVertexStride, (const void *)attrib.mOffset);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, static_cast<VuOglesVertexBuffer *>(pVertexColorBuffer)->mGlBuffer);
	glVertexAttribPointer(7, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4, (const void *)0);

	if ( mPrevIndexBufferId != (VUUINT32)mpCurIndexBuffer )
	{
		mPrevIndexBufferId = (VUUINT32)mpCurIndexBuffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mpCurIndexBuffer->mGlBuffer);
	}

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	glDrawElements(VuOglesGfxTypes::convert(primitiveType), vertexCount, GL_UNSIGNED_SHORT, (void *)(startIndex*2));

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuOglesGfx::drawPrimitiveUP(VuGfxPrimitiveType primitiveType, int primitiveCount, const void *pVertexData)
{
	if ( mPrevVertexBufferId != 0 )
	{
		mPrevVertexBufferId = 0;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	const VUBYTE *pBase = static_cast<const VUBYTE *>(pVertexData);
	for ( int i = 0; i < mpCurVertexDeclaration->mEnabledAttribCount; i++ )
	{
		const VuOglesVertexDeclaration::VuAttrib &attrib = mpCurVertexDeclaration->mEnabledAttribs[i];
		glVertexAttribPointer(attrib.mUsage, attrib.mSize, attrib.mType, attrib.mNormalized, mCurVertexStride, pBase + attrib.mOffset);
	}

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	glDrawArrays(VuOglesGfxTypes::convert(primitiveType), 0, vertexCount);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
void VuOglesGfx::drawIndexedPrimitiveUP(VuGfxPrimitiveType primitiveType, int minIndex, int numVerts, int primitiveCount, const VUUINT16 *pIndexData, const void *pVertexData)
{
	if ( mPrevVertexBufferId != 0 )
	{
		mPrevVertexBufferId = 0;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if ( mPrevIndexBufferId != 0 )
	{
		mPrevIndexBufferId = 0;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	const VUBYTE *pBase = static_cast<const VUBYTE *>(pVertexData);
	for ( int i = 0; i < mpCurVertexDeclaration->mEnabledAttribCount; i++ )
	{
		const VuOglesVertexDeclaration::VuAttrib &attrib = mpCurVertexDeclaration->mEnabledAttribs[i];
		glVertexAttribPointer(attrib.mUsage, attrib.mSize, attrib.mType, attrib.mNormalized, mCurVertexStride, pBase + attrib.mOffset);
	}

	int vertexCount = calcVertexCount(primitiveType, primitiveCount);
	glDrawElements(VuOglesGfxTypes::convert(primitiveType), vertexCount, GL_UNSIGNED_SHORT, pIndexData);

	mPrimitiveCount += primitiveCount;
	mDrawCallCount++;
}

//*****************************************************************************
bool VuOglesGfx::getExtension(const char *name)
{
	return mExtensions.find(name) != mExtensions.end();
}

//*****************************************************************************
void VuOglesGfx::bindVertexBuffer(unsigned int buffer)
{
	mPrevVertexBufferId = -1;
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
}

//*****************************************************************************
void VuOglesGfx::bindIndexBuffer(unsigned int buffer)
{
	mPrevIndexBufferId = -1;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
}

//*****************************************************************************
void VuOglesGfx::checkFramebufferStatus()
{
#ifndef VURETAIL
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		VUWARNING("Failed to make complete framebuffer object %x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
#endif
}
