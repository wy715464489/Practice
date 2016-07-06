//*****************************************************************************
//
//  Copyright (c) 2009-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Functionality to deal with binary data.
// 
//*****************************************************************************

#pragma once

#include "VuEngine/Containers/VuArray.h"

class VuAabb;
class VuRect;
class VuVector2;
class VuVector3;
class VuVector4;
class VuMatrix;
class VuQuaternion;

class VuBinaryDataWriter
{
public:
	VuBinaryDataWriter(VuArray<VUBYTE> &data) : mData(data), mSwapEndian(false) {}

	void			configure(const std::string &platform);

	void			clear() { mData.clear(); }

	inline void		writeValue(const float &value);
	inline void		writeValue(const bool &value);
	inline void		writeValue(const VUINT8 &value);
	inline void		writeValue(const VUINT16 &value);
	inline void		writeValue(const VUINT32 &value);
	inline void		writeValue(const VUINT64 &value);
	inline void		writeValue(const VUUINT8 &value);
	inline void		writeValue(const VUUINT16 &value);
	inline void		writeValue(const VUUINT32 &value);
	inline void		writeValue(const VUUINT64 &value);
	void			writeValue(const VuAabb &value);
	void			writeValue(const VuRect &value);
	void			writeValue(const VuVector2 &value);
	void			writeValue(const VuVector3 &value);
	void			writeValue(const VuVector4 &value);
	void			writeValue(const VuMatrix &value);
	void			writeValue(const VuQuaternion &value);

	template<class T>
	inline void		writeValueCompat(const T &value);
	inline void		writeData(const void *pData, int size);

	inline int		size() const	{ return mData.size(); }
	inline void		*allocate(int size);
	inline void		reserve(int size);

	// strings
	inline void		writeValue(const std::string &value) { writeString(value); }
	inline void		writeValueCompat(const std::string &value) { writeString(value); }
	inline void		writeString(const std::string &value);
	inline void		writeString(const char *str);

	// arrays
	inline void		writeArray(const VuArray<int> &value);
	inline void		writeArray(const VuArray<float> &value);
	inline void		writeArray(const VuArray<VUUINT8> &value);
	inline void		writeArray(const VuArray<VUUINT16> &value);
	inline void		writeArray(const VuArray<VUUINT32> &value);
	void			writeArray(const VuArray<VuVector3> &value);
	template<class T>
	inline void		writeArrayCompat(const VuArray<T> &value);

	// pad
	inline void		writePad(int numBytes) { memset(allocate(numBytes), 0, numBytes); }

	bool			getSwapEndian() { return mSwapEndian; }

private:
	VuArray<VUBYTE>	&mData;
	bool			mSwapEndian;
};

class VuBinaryDataReader
{
public:
	VuBinaryDataReader() : mpData(VUNULL), mSize(0), mOffset(0) {}
	VuBinaryDataReader(const void *pData, int size) : mpData((VUBYTE *)pData), mSize(size), mOffset(0) {}
	VuBinaryDataReader(const VuArray<VUBYTE> &data) : mpData(&data.begin()), mSize(data.size()), mOffset(0) {}

	void				attach(const void *pData, int size);
	void				attach(const VuArray<VUBYTE> &data);

	template<class T>
	inline void			readValue(T &value)	{ readData(&value, sizeof(T)); }
	template<class T>
	inline void			readValueCompat(T &value);
	inline void			readData(void *pData, int size);

	inline int			size() const		{ return mSize; }
	inline int			offset() const		{ return mOffset; }
	inline int			remaining() const	{ return mSize - mOffset; }
	inline const void	*cur() const		{ return &mpData[mOffset]; }
	inline void			skip(int size);

	// specializations
	inline void			readValue(std::string &value) { readString(value); }
	inline void			readValueCompat(std::string &value) { readString(value); }
	inline void			readString(std::string &value);
	inline const char	*readString();
	template<class T>
	inline void			readArray(VuArray<T> &value);
	template<class T>
	inline void			readArrayCompat(VuArray<T> &value);

private:
	const VUBYTE		*mpData;
	int					mSize;
	int					mOffset;
};

#include "VuBinaryDataUtil.inl"
