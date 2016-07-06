//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Telemetry Packets
// 
//*****************************************************************************

#include "VuTelemetryPacket.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/HAL/Net/VuNet.h"
#include "VuEngine/Json/VuJsonBinaryReader.h"
#include "VuEngine/Json/VuJsonBinaryWriter.h"
#include "VuEngine/Util/VuEndianUtil.h"


static const VUUINT32 scTelemetryMagic = ('V'<<24)|('T'<<16)|('E'<<8)|('L');
static const VUUINT32 scTelemetryVersion = 1;


class VuTelemetryHeader
{
public:
	VUUINT32	mMagic;
	VUUINT32	mVersion;
	VUUINT32	mDataSize;

	void		fixEndianness();
	void		build(const VuJsonContainer &packet);
	bool		verify();
};


//*****************************************************************************
void VuTelemetryHeader::fixEndianness()
{
	if ( mMagic == VuEndianUtil::swap(scTelemetryMagic) )
	{
		VuEndianUtil::swapInPlace(mMagic);
		VuEndianUtil::swapInPlace(mVersion);
		VuEndianUtil::swapInPlace(mDataSize);
	}
}

//*****************************************************************************
void VuTelemetryHeader::build(const VuJsonContainer &packet)
{
	mMagic = scTelemetryMagic;
	mVersion = scTelemetryVersion;
	mDataSize = VuJsonBinaryWriter::calculateDataSize(packet);
}

//*****************************************************************************
bool VuTelemetryHeader::verify()
{
	if ( mMagic != scTelemetryMagic )
		return false;

	if ( mVersion != scTelemetryVersion )
		return false;

	return true;
}

//*****************************************************************************
bool VuTelemetryPacket::receive(VuTcpSocket *pSocket, VuJsonContainer &packet)
{
	#define CHUNK_SIZE 65536
	VuArray<VUBYTE> buffer(0);
	for ( ;; )
	{
		int prevSize = buffer.size();
		buffer.resize(prevSize + CHUNK_SIZE);
		int bytesRead = pSocket->recv(&buffer[prevSize], CHUNK_SIZE);
		if ( bytesRead <= 0 )
			return false;

		buffer.resize(prevSize + bytesRead);

		// decode header
		if ( buffer.size() >= sizeof(VuTelemetryHeader) )
		{
			VuTelemetryHeader header;
			VU_MEMCPY(&header, sizeof(header), &buffer[0], sizeof(header));
			header.fixEndianness();

			if ( !header.verify() )
				return false;

			int totalSize = sizeof(VuTelemetryHeader) + header.mDataSize;
			if ( buffer.size() >= totalSize )
			{
				VuJsonBinaryReader reader;
				if ( !reader.loadFromMemory(packet, &buffer[sizeof(VuTelemetryHeader)], buffer.size()) )
					return false;

				return true;
			}
		}
	}

// commented out to remove warning
//	return false;
}

//*****************************************************************************
bool VuTelemetryPacket::send(VuTcpSocket *pSocket, const VuJsonContainer &packet)
{
	// send header
	VuTelemetryHeader header;
	header.build(packet);
	if ( pSocket->send(&header, sizeof(header)) == sizeof(header) )
	{
		VuArray<VUBYTE> buffer;
		buffer.resize(header.mDataSize);

		VuJsonBinaryWriter writer;
		int size = header.mDataSize;
		if ( writer.saveToMemory(packet, &buffer[0], size) )
		{
			if ( pSocket->send(&buffer[0], size) == size )
			{
				return true;
			}
		}
	}

	return false;
}
