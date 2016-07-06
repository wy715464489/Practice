//*****************************************************************************
//
//  Copyright (c) 2012-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  JPEG utility.
// 
//*****************************************************************************

#include <setjmp.h>
#include "VuJpeg.h"
#include "VuEngine/Libs/jpeg/jpeglib.h"


struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

//*****************************************************************************
bool VuJpeg::decompress(const VuArray<VUBYTE> &jpeg, VuArray<VUBYTE> &rgb, int &width, int &height)
{
	width = 0;
	height = 0;

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/

		jpeg_destroy_decompress(&cinfo);

		return false;
	}

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_mem_src(&cinfo, (unsigned char *)&jpeg.begin(), jpeg.size());

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/ 

	rgb.resize(cinfo.output_width * 3 * cinfo.output_height);

	// how big is this thing gonna be?
	width = cinfo.output_width;
	height = cinfo.output_height;

	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */

		int row = cinfo.output_scanline-1;
		int offset = row * width *3;
		VUBYTE *jpegline = buffer[0];

		if (cinfo.out_color_components==3)
		{
			VU_MEMCPY(&rgb[offset], width*3, jpegline, width*3);
		}
		else if (cinfo.out_color_components==1)
		{
			VUBYTE *src = jpegline;
			VUBYTE *dst = &rgb[offset];
			for ( int count = 0; count < width; count++ )
			{
				dst[0] = src[0];
				dst[1] = src[0];
				dst[2] = src[0];
				dst += 3;
				src += 1;
			}
		}
	}

	/* Step 7: Finish decompression */

	(void) jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	return true;
}

//*****************************************************************************
bool VuJpeg::compress(VuArray<VUBYTE> &jpeg, const VuArray<VUBYTE> &rgb, int width, int height, int quality)
{
	struct jpeg_compress_struct cinfo;
	/* More stuff */
	int row_stride;			/* physical row widthPix in image buffer */

	bool grayscale;
	if ( rgb.size() == width*height )
		grayscale = true;
	else if ( rgb.size() == width*height*3 )
		grayscale = false;
	else
		return false;

	struct my_error_mgr jerr;

	/* Step 1: allocate and initialize JPEG compression object */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/

		jpeg_destroy_compress(&cinfo);

		return false;
	}

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	unsigned char *outbuffer = VUNULL;
	unsigned long outsize = VUNULL;

	jpeg_mem_dest(&cinfo, &outbuffer, &outsize);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = width; 	/* image widthPix and height, in pixels */
	cinfo.image_height = height;
	if (grayscale) {
		cinfo.input_components = 1;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
	} else {
		cinfo.input_components = 3;		/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	}


	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/

	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = width * 3;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		VUBYTE *outRow;
		if (grayscale) {
			outRow = (unsigned char *)&rgb[cinfo.next_scanline * width];
		} else {
			outRow = (unsigned char *)&rgb[cinfo.next_scanline * width * 3];
		}

		(void) jpeg_write_scanlines(&cinfo, &outRow, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);

	/* After finish_compress, we can close the output file. */
	jpeg.resize(outsize);
	VU_MEMCPY(&jpeg.begin(), outsize, outbuffer, outsize);
	free(outbuffer);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */

	return true;
}
