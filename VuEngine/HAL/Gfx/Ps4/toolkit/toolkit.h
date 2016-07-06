/* SCE CONFIDENTIAL
PlayStation(R)4 Programmer Tool Runtime Library Release 02.000.071
* Copyright (C) 2013 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef _SCE_GNM_TOOLKIT_H
#define _SCE_GNM_TOOLKIT_H

#include <gnm.h>
#include <gnmx.h>
#include "geommath/geommath.h"
#include "allocator.h"
#include "memory_requests.h"
#include "embedded_shader.h"
#include "dataformat_interpreter.h"

namespace sce
{
	namespace Gnmx
	{
		namespace Toolkit
		{
			/** @brief Sets the viewport parameters associated with a viewport id. This implementation takes more familiar parameters,
					   but requires slightly more computation internally.
			   @param dcb The command buffer to which commands should be written.
			   @param viewportId           Id of the viewport (ranges from 0 through 15)
			   @param left                 Left hand edge of the viewport rectangle
			   @param top                  Upper hand edge of the viewport rectangle
			   @param right                Right hand edge of the viewport rectangle
			   @param bottom               Lower edge of the viewport rectangle
			   @param dmin                 Minimum Z Value from Viewport Transform.  Z values will be clamped by the DB to this value.
			   @param dmax                 Maximum Z Value from Viewport Transform.  Z values will be clamped by the DB to this value.
			   @par Definition Heading
			   Draw Command Buffer
			   */
			void setViewport(sce::Gnmx::GnmxDrawCommandBuffer *dcb, uint32_t viewportId, int32_t left, int32_t top, int32_t right, int32_t bottom, float dmin, float dmax);


			/** @brief Saves the first miplevel of a texture to a .tga file.
			 * @param texture The texture to save
			 * @param pixels CPU accessible pointer to the pixels of the texture you wish to save.
							 It's assumed that these pixels are 'untiled'
			 * @param tgaFileName The path to the TGA file to save to
			 * @return If the save is successful, the return value will be 0. Otherwise, a non-zero error code will be returned.
			 */
			int32_t saveTextureToTga(const sce::Gnm::Texture* texture, const char* filename);

			/** @brief Saves the first miplevel of a texture to a .pfm file.
			 * @param texture The texture to save
			 * @param pixels CPU accessible pointer to the pixels of the texture you wish to save.
							 It's assumed that these pixels are 'untiled'
			 * @param tgaFileName The path to the PFM file to save to
			 * @return If the save is successful, the return value will be 0. Otherwise, a non-zero error code will be returned.
			 */
			int32_t saveTextureToPfm(const sce::Gnm::Texture* texture, const uint8_t* pixels, const char *tgaFileName);

			/** @brief Flushes a render target from the CB cache, regardless of which of eight possible render target indices it's assigned to.
			 *         this is required when we don't know which render target indices are assigned to a render target, and we need to
			 *         ensure that a subsequent compute job won't hit memory that's lingering in the CB cache.
			 * @param dcb The command buffer to which commands should be written.
			 * @param renderTarget The render target to synchronize
			 * @par Definition Heading
				Draw Command Buffer
			 */
			void synchronizeRenderTargetGraphicsToCompute(sce::Gnmx::GnmxDrawCommandBuffer *dcb, const sce::Gnm::RenderTarget* renderTarget);

			 /** @brief Flushes a depth render target's Z buffer data from the DB cache.
			 *         this is required when we need to ensure that a subsequent compute job won't hit memory that's lingering in the DB cache.
			 * @param dcb The command buffer to which commands should be written.
			 * @param depthTarget The depth render target to synchronize Z buffer
			 * @par Definition Heading
			 Draw Command Buffer
			 */
			void synchronizeDepthRenderTargetZGraphicsToCompute(sce::Gnmx::GnmxDrawCommandBuffer *dcb, const sce::Gnm::DepthRenderTarget* depthTarget);

			/** @brief Waits for pixel end-of-shader, and then flushes the L2$. This should be sufficient when one
			 *         wishes to synchronize between a graphics job that outputs data and a compute job that reads the
			 *         same data, since both jobs will run on the GPU and will hit L2$, though each CU's L1$ may have a
			 *         stale copy of data that the previous job wrote.
			 * @param dcb The command buffer to which commands should be written.
			 * @par Definition Heading
			 Draw Command Buffer
			 */
			void synchronizeGraphicsToCompute( sce::Gnmx::GnmxDrawCommandBuffer *dcb );

			/** @brief Waits for compute end-of-shader, and then flushes the L1$. This should be sufficient when one
			 *         wishes to synchronize between a compute job that outputs data and a graphics job that reads the
			 *         same data, since both jobs will run on the GPU and will hit L2$, though each CU's L1$ may have a
			 *         stale copy of data that the previous job wrote.
			 * @param dcb The command buffer to which commands should be written.
			 * @par Definition Heading
			 Draw Command Buffer
			 */
			void synchronizeComputeToGraphics( sce::Gnmx::GnmxDrawCommandBuffer *dcb );

			/** @brief Waits for compute end-of-shader, and then flushes the L1$. This should be sufficient when one
			 *         wishes to synchronize between a compute job that outputs data and a subsequent compute job that reads the
			 *         same data, since both jobs will run on the GPU and will hit L2$, though a given CU's L1$ may have a
			 *         stale copy of data that the previous job wrote.
			 * @param dcb The command buffer to which commands should be written.
			 * @par Definition Heading
			 Draw Command Buffer
			 */
			void synchronizeComputeToCompute( sce::Gnmx::GnmxDrawCommandBuffer *dcb );

			/** @brief Waits for compute end-of-shader, and then flushes the L1$. This should be sufficient when one
			 *         wishes to synchronize between a compute job that outputs data and a subsequent compute job that reads the
			 *         same data, since both jobs will run on the GPU and will hit L2$, though a given CU's L1$ may have a
			 *         stale copy of data that the previous job wrote.
			 * @param dcb The command buffer to which commands should be written.
			 * @par Definition Heading
			 Dispatch Command Buffer
			 */
			void synchronizeComputeToCompute( sce::Gnmx::GnmxDispatchCommandBuffer *dcb );

			/** @brief Calculates ONION and GARLIC memory requirements of Toolkit
				@param memoryRequests a pointer to a MemoryRequests object
			*/
			void addToMemoryRequests(Gnmx::Toolkit::MemoryRequests *memoryRequests);

			/** @brief Initializes Toolkit with a list of ONION and GARLIC memory requirements that has been fulfilled by the user.
				@param memoryRequests a pointer to a MemoryRequests object
				*/
			void initializeWithMemoryRequests(Gnmx::Toolkit::MemoryRequests *memoryRequests);


			/** @brief Allocate and map a memory block for the tessellation factor ring buffer */
			void* allocateTessellationFactorRingBuffer();

			/** @brief Deallocate the allocated memory block for the tessellation factor ring buffer */
			void deallocateTessellationFactorRingBuffer();

			namespace SurfaceUtil
			{
				/** @brief Resolves an MSAA render target, so that it can be subsequently be sampled as a texture.
					@param gfxc The graphics context to write the commands to.
					@param destTarget The single-sampled render target which will be the destination of the resolve operation.
					@param srcTarget The multi-sampled render target which will be the source of the resolve operation.
					@note In order for the resolve to succeed, the following conditions must all be met:
					      - srcTarget and destTarget must have the same MicroTileMode (kMicroTileModeDisplay vs. kMicroTileModeThin).
						  - srcTarget and destTarget must have the same DataFormat
						  - srcTarget and destTarget must not have a tile mode of kTileModeDisplay_LinearAligned.
						  - destTarget must be a single-sample target (getNumFragments() == kNumFragments1)
						  - destTarget must not use CMASK fast-clear.
					*/
				void resolveMsaaBuffer(sce::Gnmx::GfxContext &gfxc, sce::Gnm::RenderTarget *destTarget, sce::Gnm::RenderTarget *srcTarget);

				/** @brief Uses CPU to fill a region of memory with a 32-bit value.
				 * @param destAddr The address to begin filling.
				 * @param dwords The number of dwords to fill.
				 * @param val The 32-bit value to set memory to.
				 */
				void fillDwordsWithCpu(uint32_t* destAddr, uint32_t dwords, uint32_t val);

				/** @brief Uses CP DMA to fill a region of memory with a 32-bit value.
				 * @param gfxc The graphics context to use.
				 * @param destAddr The address to begin filling.
				 * @param dwords The number of dwords to fill.
				 * @param val The 32-bit value to set memory to.
				 */
				void fillDwordsWithDma(sce::Gnmx::GfxContext &gfxc, void *destAddr, uint32_t dwords, uint32_t val);

				/** @brief Uses a compute shader to clear memory to a repeating 32-bit pattern.
				 * @param gfxc The graphics context to use.
				 * @param destAddr The address to begin filling.
				 * @param dwords the number of dwords to fill.
				 * @param val the 32-bit value to set memory to.
				 */
				void fillDwordsWithCompute(sce::Gnmx::GfxContext &gfxc, void *destAddr, uint32_t dwords, uint32_t value);

				/** @brief Uses a compute shader to clear memory, by writing a repeating bit pattern to it as an RW_DataBuffer<uint>.
				 * @param gfxc The graphics context to use
				 * @param destination The memory to clear
				 * @param destUints The number of uints in the destination buffer
				 * @param source Pointer to srcUints uint32_t values. Must be in GPU visible memory (not on stack.)
				 * @param srcUints The number of uints in the source buffer
				 */
				void clearMemoryToUints(sce::Gnmx::GfxContext &gfxc, void *destination, uint32_t destUints, uint32_t *source, uint32_t srcUints);

				/** @brief Uses a compute shader to clear a buffer to a constant value, by writing a repeating bit pattern to it as an RW_DataBuffer<uint>.
				 * @param gfxc The graphics context to use
				 * @param buffer The buffer to clear
				 * @param source Pointer to data to clear the buffer with
				 * @param sourceUints Number of uints in source data
				 */
				void clearBuffer(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Buffer* buffer, uint32_t *source, uint32_t sourceUints);

				/** @brief Uses a compute shader to clear a buffer to a constant value.
				 * @param gfxc The graphics context to use
				 * @param buffer The buffer to clear
				 * @param vector The vector to clear the buffer to
				 */
				void clearBuffer(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Buffer* buffer, const Vector4& vector);

				/** @brief Uses a compute shader to clear a texture to a solid color, by writing a repeating bit pattern to it as an RW_DataBuffer<uint>.
				 * @param gfxc The graphics context to use
				 * @param texture The texture to clear
				 * @param source Pointer to data to clear the texture with
				 * @param sourceUints Number of uints in source data
				 */
				void clearTexture(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Texture* texture, uint32_t *source, uint32_t sourceUints);

				/** @brief Uses a compute shader to clear a texture to a solid color.
				 * @param gfxc The graphics context to use
				 * @param texture The texture to clear
				 * @param color The color to clear the texture to
				 */
				void clearTexture(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Texture* texture, const Vector4& color);

				/** @brief Uses a compute shader to clear a render target to a solid color, by writing a repeating bit pattern to it as an RW_DataBuffer<uint>.
				 * @param gfxc The graphics context to use
				 * @param renderTarget The render target to clear
				 * @param source Pointer to data to clear the render target with
				 * @param sourceUints Number of uints in source data
				 */
				void clearRenderTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget* renderTarget, uint32_t *source, uint32_t sourceUints);

				/** @brief Uses a compute shader to clear a render target to a solid color.
				 * @param gfxc The graphics context to use
				 * @param renderTarget The render target to clear
				 * @param color The color to clear the render target to
				 */
				void clearRenderTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget* renderTarget, const Vector4& color);

				/** @brief Uses a compute shader to copy a buffer using RwBuffers with no format conversion.
				 * @param gfxc The graphics context to use
				 * @param bufferDst The buffer to copy to, which must be a multiple of 64 dwords (256 bytes) in size.
				 * @param bufferSrc The buffer to copy from, which must exactly match the size and format of bufferDst
				 */
				void copyBuffer(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Buffer* bufferDst, const sce::Gnm::Buffer* bufferSrc);

				/** @brief Uses a compute shader to copy a texture.
				 *
				 *         The function will fail if the following criteria are not met:
				 *         - The source and destination textures must not be NULL.
				 *         - The source texture must have at least as many mip levels as the destination texture.
				 *         - The source texture must have at least as many array slices as the destination texture.
				 *         - The source and destination textures must have the same dimensionality (according to their getDimension() methods). Only 1D, 2D and 3D textures are currently supported.
				 *         - The source and destination textures must have the same width/height/depth, in all relevant dimensions (according to their dimensionality).
				 * @param gfxc The graphics context to use.
				 * @param textureDst The texture to copy to.
				 * @param textureSrc The texture to copy from.
				 */
				void copyTexture(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Texture* textureDst, const sce::Gnm::Texture* textureSrc);

				/** @brief Uses a pixel shader to copy a texture to a render target.
				 *
				 * @param gfxc The graphics context to use.
				 * @param renderTargetDestination The render target to copy to
				 * @param textureSource The texture to copy from
				 */
				void copyTextureToRenderTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget* renderTargetDestination, const sce::Gnm::Texture* textureSource);

				/** @brief Uses a compute shader to copy a render target using RwBuffers with no format conversion.
				 * @param gfxc The graphics context to use
				 * @param renderTargetDst The render target to copy to
				 * @param renderTargetSrc The render target to copy from, which must exactly match the size and format of renderTargetDst
				 */
				void copyRenderTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget* renderTargetDst, const sce::Gnm::RenderTarget* renderTargetSrc);

				/** @brief Uses a compute shader to copy a depth render target using RwBuffers with no format conversion.
				 * @param gfxc The graphics context to use
				 * @param depthTargetDst The depth render target to copy Z to
				 * @param depthTargetSrc The depth render target to copy Z from, which must exactly match the size and format of depthTargetDst
				 */
				void copyDepthTargetZ(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget* depthTargetDst, const sce::Gnm::DepthRenderTarget* depthTargetSrc);

				/** @brief Clears a depth render target by enabling depth clear state, and rendering a RECT primitive.
				 * @param gfxc The graphics context to use
				 * @param depthTarget The depth render target to clear
				 */
				void clearDepthTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const float depthValue);

				/** @brief Clears a stencil buffer by enabling stencil clear state, and rendering a RECT primitive.
				 * @param gfxc The graphics context to use
				 * @param depthTarget The depth render target to clear the stencil buffer of
				 */
				void clearStencilTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const uint8_t stencilValue);

				/** @brief Clears a depth render target and its stencil buffer by enabling depth and stencil clear state, and rendering a RECT primitive.
				 * @param gfxc The graphics context to use
				 * @param depthTarget The depth render target to clear
				 */
				void clearDepthStencilTarget(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const float depthValue, const uint8_t stencilValue);

				/** @brief Clears a depth render target's stencil buffer by setting it to a constant value.
				 */
				void clearStencilSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const uint8_t value);

				/** @brief Clears a depth render target's stencil buffer by setting it to the constant value of zero.
				 */
				void clearStencilSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget);

				/** @brief Quickly clears a depth render target by setting all dwords in its HTILE buffer to a single value.
                 *         This version of the function takes as its argument an Htile structure capable of expressing any dword value.
				 */
				void clearHtileSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const Gnm::Htile htile);

				/** @brief Quickly clears a depth render target by setting all dwords in its HTILE buffer to a single value.
                 *         This version of the function takes as its argument minZ and maxZ values, capable of expressing only HiZ-only dword values.
				 */
				void clearHtileSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const float minZ, const float maxZ);

				/** @brief Quickly clears a depth render target by setting all dwords in its HTILE buffer to a single value.
                 *         This version of the function takes as its argument a Z value, capable of expressing only HiZ-only dword values.
				 */
				void clearHtileSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget, const float Z);

				/** @brief Quickly clears a depth render target by setting all dwords in its HTILE buffer to a single value.
                 *         This version of the function takes no Z argument; minZ and maxZ of 1.f is implied. This makes sense if
                 *         the Z test is < or <=.
				 */
				void clearHtileSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget);

				/** @brief Quickly clears a render target by zeroing out its CMASK buffer.
				 */
				void clearCmaskSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget *target);

				/** @brief Applies the second stage of a fast clear operation to a render target: the CMASK
				 *         is examined and all untouched areas of the CB are replaced with the fast clear color.
				 *         The following render state is stomped by this function, and may need to be reset:
				 *         - Render target slot 0 [function sets to target]
				 *         - screen viewport [function sets to target's width/height]
				 *         - vertex shader / pixel shader [function sets to undefined values]
				 *         - depth stencil control [function disables zwrite and sets compareFunc=ALWAYS]
				 *         - blend control [function disables blending]
				 *         - MRT color mask [function sets to 0xF]
				 *         - color control [function sets to Normal/SrcCopy]
				 *         - primitive type (set to kPrimitiveTypeRectList)
				 *         The following render state is assumed to have been set up by the caller:
				 *         - active shader stages [function assumes only VS/PS are enabled]
				 */
				void eliminateFastClear(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget *target);

				/** @brief Decompresses a function's FMASK surface so that all pixels have valid FMASK data.
				 *         The following render state is stomped by this function, and may need to be reset:
				 *         - Render target slot 0 [function sets to target]
				 *         - screen viewport [function sets to target's width/height]
				 *         - vertex shader / pixel shader [function sets to undefined values]
				 *         - depth stencil control [function disables zwrite and sets compareFunc=ALWAYS]
				 *         - blend control [function disables blending]
				 *         - MRT color mask [function sets to 0xF]
				 *         - color control [function sets to Normal/SrcCopy]
				 *         - primitive type (set to kPrimitiveTypeRectList)
				 *         The following render state is assumed to have been set up by the caller:
				 *         - active shader stages [function assumes only VS/PS are enabled]
				 */
				void decompressFmaskSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::RenderTarget *target);

				void renderFullScreenQuad(sce::Gnmx::GfxContext &gfxc);

				/** @brief Decompresses a function's depth buffer so that all unwritten pixels have valid depth data
				 *         The following render state is stomped by this function, and may need to be reset:
				 *         - Depth render target [function sets to depthTarget]
				 *         - screen viewport [function sets to depthTarget's pitch/height]
				 *         - vertex shader / pixel shader [function sets to undefined values]
				 *         - depth stencil control [function disables zwrite and sets compareFunc=NEVER]
				 *         - depth render control [function disables stencil and depth compression]
				 *         - blend control [function disables blending]
				 *         - MRT color mask [function sets to 0xF]
				 *         - color control [function sets to Normal/SrcCopy]
				 *         - primitive type (set to kPrimitiveTypeRectList)
				 *         The following render state is assumed to have been set up by the caller:
				 *         - active shader stages [function assumes only VS/PS are enabled]
				 */
				void decompressDepthSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *depthTarget);

				/** @brief Decompresses a depth surface to a copy, without decompressing the original depth surface.
				 *         The following render state is stomped by this function, and may need to be reset:
				 *         - Depth render target [function sets to depthTarget]
				 *         - screen viewport [function sets to depthTarget's pitch/height]
				 *         - vertex shader / pixel shader [function sets to undefined values]
				 *         - depth stencil control [function disables zwrite and sets compareFunc=NEVER]
				 *         - depth render control [function disables stencil and depth compression]
				 *         - blend control [function disables blending]
				 *         - MRT color mask [function sets to 0xF]
				 *         - color control [function sets to Normal/SrcCopy]
				 *         - primitive type (set to kPrimitiveTypeRectList)
				 *         - render override control
				 *         The following render state is assumed to have been set up by the caller:
				 *         - active shader stages [function assumes only VS/PS are enabled]
				 */
				void decompressDepthSurfaceToCopy(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *destination, const sce::Gnm::DepthRenderTarget *source);

				/** @brief Copies one compressed depth surface to another, without decompressing either surface.
				 *         The following render state is stomped by this function, and may need to be reset:
				 *         - Depth render target [function sets to depthTarget]
				 *         - screen viewport [function sets to depthTarget's pitch/height]
				 *         - vertex shader / pixel shader [function sets to undefined values]
				 *         - depth stencil control [function disables zwrite and sets compareFunc=NEVER]
				 *         - depth render control [function disables stencil and depth compression]
				 *         - blend control [function disables blending]
				 *         - MRT color mask [function sets to 0xF]
				 *         - color control [function sets to Normal/SrcCopy]
				 *         - primitive type (set to kPrimitiveTypeRectList)
				 *         - render override control
				 *         The following render state is assumed to have been set up by the caller:
				 *         - active shader stages [function assumes only VS/PS are enabled]
				 */
				void copyCompressedDepthSurface(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::DepthRenderTarget *destination, const sce::Gnm::DepthRenderTarget *source);

				/** @brief Generates mip maps for a texture by averaging each 2x2 texels of mip0 and storing in mip1,
				 *         then averaging each 2x2 texels of mip1 and storing in mip2, et cetera.
				 *         This binds each destination mip as a render target, and stalls between each pass to
				 *         synchronize the CB$ and L2$.
				 *         This function is optimized for convenience and familiarity, and not for performance.
				 *         The following render state is stomped by this function, and may need to be reset:
				 *         - Depth render target [function sets to depthTarget]
				 *         - screen viewport [function sets to depthTarget's pitch/height]
				 *         - vertex shader / pixel shader [function sets to undefined values]
				 *         - depth stencil control [function disables zwrite and sets compareFunc=NEVER]
				 *         - depth render control [function disables stencil and depth compression]
				 *         - blend control [function disables blending]
				 *         - MRT color mask [function sets to 0xF]
				 *         - primitive type (set to kPrimitiveTypeRectList)
				 *         The following render state is assumed to have been set up by the caller:
				 *         - active shader stages [function assumes only VS/PS are enabled]
				 */
				void generateMipMaps(sce::Gnmx::GfxContext &gfxc, const sce::Gnm::Texture *texture);
			};

			class Timers
			{
				class Timer
				{
				public:
					uint64_t m_begin;
					uint64_t m_end;
				};
				Timer*   m_timer;
				uint32_t m_timers;
			public:
				void *m_addr;
				static Gnm::SizeAlign getRequiredBufferSizeAlign(uint32_t timers);
				void initialize(void *addr, uint32_t timers);
				void begin(sce::Gnmx::GnmxDrawCommandBuffer& dcb, uint32_t timer);
				void end(sce::Gnmx::GnmxDrawCommandBuffer& dcb, uint32_t timer);
				void begin(sce::Gnmx::GnmxDispatchCommandBuffer& dcb, uint32_t timer);
				void end(sce::Gnmx::GnmxDispatchCommandBuffer& dcb, uint32_t timer);
				uint64_t readTimerInGpuClocks(uint32_t timer) const;
				double readTimerInMilliseconds(uint32_t timer) const;
			};

			/** @brief This submits a command buffer on the GPU, and then stalls the CPU until the GPU is done.
			 */
			void submitAndStall(sce::Gnmx::GnmxDrawCommandBuffer& dcb);

			/** @brief This submits a command buffer on the GPU, and then stalls the CPU until the GPU is done.
			 */
			void submitAndStall(sce::Gnmx::GfxContext& gfxc);
		}
	}
}
#endif /* _SCE_GNM_TOOLKIT_H */
