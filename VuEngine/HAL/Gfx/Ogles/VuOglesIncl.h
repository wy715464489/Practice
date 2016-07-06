//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  OpgnGL ES implementation of the cube texture interface class.
// 
//*****************************************************************************

#pragma once


#if defined(VUANDROID)
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
	#include "VuEngine/HAL/Gfx/Android/gl3stub.h"
#elif defined(VUIOS)
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	#include <OpenGLES/ES3/gl.h>
	#include <OpenGLES/ES3/glext.h>
#elif defined(VUBB10)
	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#else
	#error Platform not defined!
#endif


/* GL_EXT_texture_compression_dxt1 */
#ifndef GL_EXT_texture_compression_dxt1
	#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif

/* GL_EXT_texture_compression_s3tc */
#ifndef GL_EXT_texture_compression_s3tc
	/* GL_COMPRESSED_RGB_S3TC_DXT1_EXT defined in GL_EXT_texture_compression_dxt1 already. */
	/* GL_COMPRESSED_RGBA_S3TC_DXT1_EXT defined in GL_EXT_texture_compression_dxt1 already. */
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif


#ifndef IMG_texture_compression_pvrtc
	#define COMPRESSED_RGB_PVRTC_4BPPV1_IMG   0x8C00
	#define COMPRESSED_RGB_PVRTC_2BPPV1_IMG   0x8C01
	#define COMPRESSED_RGBA_PVRTC_4BPPV1_IMG  0x8C02
	#define COMPRESSED_RGBA_PVRTC_2BPPV1_IMG  0x8C03
#endif
