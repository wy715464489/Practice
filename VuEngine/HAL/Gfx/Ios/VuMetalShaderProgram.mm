//*****************************************************************************
//
//  Copyright (c) 2014-2014 Vector Unit Inc
//  Confidential Trade Secrets
//
//  Metal implementation of the shader interface class.
//
//*****************************************************************************

#include "VuMetalShaderProgram.h"
#include "VuMetalGfx.h"
#include "VuEngine/Math/VuMatrix.h"
#include "VuEngine/Math/VuVector2.h"
#include "VuEngine/Math/VuVector3.h"
#include "VuEngine/Math/VuVector4.h"
#include "VuEngine/Util/VuColor.h"
#include "VuEngine/Util/VuHash.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"


// static variables
static struct MetalShaderData
{
	typedef std::map<VUUINT32, VuMetalShaderProgram *> ShaderPrograms;
	ShaderPrograms	mShaderPrograms;
} sMetalShaderData;


//*****************************************************************************
VuMetalShaderProgram::VuMetalShaderProgram():
	mConstantCount(0),
	mSamplerCount(0)
{
}

//*****************************************************************************
VuMetalShaderProgram::~VuMetalShaderProgram()
{
	sMetalShaderData.mShaderPrograms.erase(mHash);
}

//*****************************************************************************
VUHANDLE VuMetalShaderProgram::getConstantByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);
	
	for ( int i = 0; i < mConstantCount; i++ )
		if ( mConstants[i].mHashedName == hashedName )
			return (VUHANDLE)&mConstants[i];
	
	return VUNULL;
}

//*****************************************************************************
int VuMetalShaderProgram::getSamplerIndexByName(const char *strName) const
{
	VUUINT32 hashedName = VuHash::fnv32String(strName);
	
	for ( int i = 0; i < mSamplerCount; i++ )
		if ( mSamplers[i].mHashedName == hashedName )
			return mSamplers[i].mIndex;

	return -1;
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantFloat(VUHANDLE handle, float fValue)
{
	setConstantValue(handle, &fValue, sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantFloat3(VUHANDLE handle, const float *pfValues)
{
	setConstantValue(handle, pfValues, 3*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantFloat4(VUHANDLE handle, const float *pfValues)
{
	setConstantValue(handle, pfValues, 4*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantInt(VUHANDLE handle, int iValue)
{
	setConstantValue(handle, &iValue, sizeof(iValue));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantMatrix(VUHANDLE handle, const VuMatrix &mat)
{
	setConstantValue(handle, &mat, 16*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantVector2(VUHANDLE handle, const VuVector2 &vec)
{
	setConstantValue(handle, &vec, 2*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantVector3(VUHANDLE handle, const VuVector3 &vec)
{
	setConstantValue(handle, &vec, 3*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantVector4(VUHANDLE handle, const VuVector4 &vec)
{
	setConstantValue(handle, &vec, 4*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantColor3(VUHANDLE handle, const VuColor &color)
{
	float aColor[3];
	color.toFloat3(aColor[0], aColor[1], aColor[2]);
	setConstantValue(handle, aColor, 3*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantColor4(VUHANDLE handle, const VuColor &color)
{
	float aColor[4];
	color.toFloat4(aColor[0], aColor[1], aColor[2], aColor[3]);
	setConstantValue(handle, aColor, 4*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantFloatArray(VUHANDLE handle, const float *pfValue, int count)
{
	setConstantValue(handle, pfValue, count*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantIntArray(VUHANDLE handle, const int *piValue, int count)
{
	setConstantValue(handle, piValue, count*sizeof(int));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantMatrixArray(VUHANDLE handle, const VuMatrix *pMat, int count, bool skinning)
{
	setConstantValue(handle, pMat, count*16*sizeof(float));
}

//*****************************************************************************
void VuMetalShaderProgram::setConstantVector4Array(VUHANDLE handle, const VuVector4 *pVec, int count)
{
	setConstantValue(handle, pVec, count*4*sizeof(float));
}

//*****************************************************************************
VuMetalShaderProgram *VuMetalShaderProgram::load(VuBinaryDataReader &reader)
{
	const char *strOgles2VS = reader.readString(); // skip Ogles2 VS
	const char *strOgles2PS = reader.readString(); // skip Ogles2 PS
	const char *strOgles3VS = reader.readString(); // skip Ogles3 VS
	const char *strOgles3PS = reader.readString(); // skip Ogles3 PS
	
	// read metal shader text
	const char *strMetalShader = reader.readString();

	// calculate hash
	VUUINT32 hash = VuHash::fnv32String(strMetalShader);
	
	// check if we already have the shader compiled
	MetalShaderData::ShaderPrograms::iterator iter = sMetalShaderData.mShaderPrograms.find(hash);
	if ( iter != sMetalShaderData.mShaderPrograms.end() )
	{
		iter->second->addRef();
		return iter->second;
	}
	
	// preprocess shader
	std::string shaderText;
	shaderText += "#include <metal_stdlib>\n";
	shaderText += "#include <simd/simd.h>\n";
	shaderText += "using namespace metal;\n";
	shaderText += strMetalShader;
	
	// create render pipeline state
	NSString *source = [NSString stringWithUTF8String:shaderText.c_str()];
	
	MTLCompileOptions *compileOptions = [[MTLCompileOptions alloc] init];
	compileOptions.fastMathEnabled = TRUE;

	NSError *error;
	id<MTLLibrary> mtlLibrary = [VuMetalGfx::getDevice() newLibraryWithSource:source options:compileOptions error:&error];
	if ( mtlLibrary == nil )
	{
		NSLog(@"%@",[error localizedDescription]);
		return VUNULL;
	}
	
	id<MTLFunction> vertexFunction = [mtlLibrary newFunctionWithName:@"vertexShader"];
	if ( vertexFunction == nil )
	{
		VUERROR("Unable to find 'vertexShader' in metal library!");
		return VUNULL;
	}

	id<MTLFunction> fragmentFunction = [mtlLibrary newFunctionWithName:@"fragmentShader"];
	if ( fragmentFunction == nil )
	{
		VUERROR("Unable to find 'fragmentFunction' in metal library!");
		return VUNULL;
	}
	
	// create shader program
	VuMetalShaderProgram *pShaderProgram = new VuMetalShaderProgram;
	pShaderProgram->mHash = hash;
	pShaderProgram->mShaders[VERTEX_SHADER].mMtlFunction = vertexFunction;
	pShaderProgram->mShaders[PIXEL_SHADER].mMtlFunction = fragmentFunction;
	
	// build constant/sampler tables
	if ( !pShaderProgram->buildTables() )
		return VUNULL;
	
	sMetalShaderData.mShaderPrograms[hash] = pShaderProgram;
	
	return pShaderProgram;
}

//*****************************************************************************
bool VuMetalShaderProgram::buildTables()
{
	// make fake vertex descriptor
	MTLVertexDescriptor *vd = [MTLVertexDescriptor new];
	NSArray *vertAttribs = [mShaders[VERTEX_SHADER].mMtlFunction vertexAttributes];
	int numAttribs = [vertAttribs count];
	for ( int i = 0; i < numAttribs; ++i )
	{
		MTLVertexAttribute *vertAttrib = vertAttribs[i];
		int attribIndex = [vertAttrib attributeIndex];
		MTLVertexAttributeDescriptor *desc = vd.attributes[attribIndex];
		
		// super haxored
		if ( [[vertAttrib name] isEqualToString:@"mBlendIndices"] )
			desc.format = MTLVertexFormatUInt; // Cant get the type, so just use a uint
		else
			desc.format = MTLVertexFormatFloat; // Cant get the type, so just use a float
		
		desc.bufferIndex = 0;
		desc.offset = i*4;
	}
	vd.layouts[0].stride = numAttribs*4;
	vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
	
	// create temporary pipeline to get reflection info (dodgy)
	MTLRenderPipelineDescriptor *pPipelineDescriptor = [MTLRenderPipelineDescriptor new];
	pPipelineDescriptor.vertexDescriptor = vd;
	pPipelineDescriptor.vertexFunction = mShaders[VERTEX_SHADER].mMtlFunction;
	pPipelineDescriptor.fragmentFunction = mShaders[PIXEL_SHADER].mMtlFunction;
	pPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;

	MTLRenderPipelineReflection *pReflection;
	MTLPipelineOption pipelineOptions = (MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo);
	NSError *error;
	id<MTLRenderPipelineState> renderPipelineState = [VuMetalGfx::getDevice() newRenderPipelineStateWithDescriptor:pPipelineDescriptor options:pipelineOptions reflection:&pReflection error:&error];
	if ( renderPipelineState == nil )
	{
		NSLog(@"%@",[error localizedDescription]);
		return false;
	}
	
	// reflect
	reflect(VERTEX_SHADER, pReflection.vertexArguments);
	reflect(PIXEL_SHADER, pReflection.fragmentArguments);
	
	return true;
}

//*****************************************************************************
void VuMetalShaderProgram::reflect(eShader shader, NSArray *args)
{
	Shader *pShader = &mShaders[shader];
	
	for ( MTLArgument *arg in args )
	{
		if ( arg.active )
		{
			MTLArgumentType type = arg.type;
			if ( type == MTLArgumentTypeBuffer )
			{
				if ( arg.bufferDataType == MTLDataTypeStruct && [arg.bufferStructType.members count] )
				{
					// reflect mapping
					for ( MTLStructMember *member in arg.bufferStructType.members )
					{
						VUASSERT(mConstantCount < MAX_CONSTANT_COUNT, "Too many constants in shader program!");
						Constant &constant = mConstants[mConstantCount];
						mConstantCount++;
						
						constant.mHashedName = VuHash::fnv32String([member.name UTF8String]);
						constant.mShader = shader;
						constant.mBuffer = pShader->mConstantBufferCount;
						constant.mOffset = member.offset;
					}

					// allocate buffer
					VUASSERT(pShader->mConstantBufferCount < MAX_CONSTANT_BUFFER_COUNT, "Too many constant buffers in shader!");
					ConstantBuffer &constantBuffer = pShader->mConstantBuffers[pShader->mConstantBufferCount];
					pShader->mConstantBufferCount++;
					
					constantBuffer.mIndex = arg.index;
					constantBuffer.mSize = arg.bufferDataSize;
					constantBuffer.mpData = new VUBYTE[constantBuffer.mSize];
				}
			}
			else if ( type == MTLArgumentTypeTexture )
			{
				VUASSERT(mSamplerCount < MAX_SAMPLER_COUNT, "Too many textures in shader program!");
				Sampler &sampler = mSamplers[mSamplerCount];
				mSamplerCount++;
				
				sampler.mHashedName = VuHash::fnv32String([arg.name UTF8String]);
				sampler.mIndex = arg.index;
			}
		}
	}
}
