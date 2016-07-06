//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Console
// 
//*****************************************************************************

#include "VuDevHostComm.h"
#include "VuEngine/HAL/Net/VuNet.h"
#include "VuEngine/HAL/Thread/VuThread.h"


IMPLEMENT_SYSTEM_COMPONENT(VuDevHostComm, VuDevHostComm);


#if VU_DISABLE_DEV_HOST_COMM

VuBinaryDataWriter VuDevHostComm::beginMessage(const char *msgType)
{
	VuBinaryDataWriter request(mBuffer);
	request.clear();
	return request;
}

#else

static const VUUINT32 scDevHostCommMagic = ('V'<<24)|('D'<<16)|('H'<<8)|('C');
static const VUUINT32 scDevHostCommVersion = 4;

#define DEV_HOST_COMM_TIMEOUT_MS 1000 // milliseconds


//*****************************************************************************
VuDevHostComm::VuDevHostComm():
	mpSocket(VUNULL),
	mHostIpAddr(0)
{
}

//*****************************************************************************
bool VuDevHostComm::init(const std::string &host)
{
	mHostName = host;

	mpSocket = VuTcpSocket::create(0);
	if ( mpSocket == VUNULL )
		return VUWARNING("VuDevHostComm: Unable to create tcp socket.");

	// try a few times
	for ( int attempt = 0; attempt < 5; attempt++ )
	{
		if ( mpSocket->connect(host.c_str(), DEV_HOST_COMM_PORT, DEV_HOST_COMM_TIMEOUT_MS) )
		{
			VUPRINTF("VuDevHostComm: Connected to host '%s'.\n", host.c_str());
			break;
		}
		else
		{
			VUPRINTF("VuDevHostComm: Unable to connect to host '%s' (attempt %d).\n", host.c_str(), attempt + 1);
		}
	}

	VuSys::IF()->addLogCallback(this);

	VUPRINTF("DevHostComm Initialized...\n");

	return true;
}

//*****************************************************************************
void VuDevHostComm::release()
{
	VuSys::IF()->removeLogCallback(this);

	delete mpSocket;

	VUPRINTF("DevHostComm Released...\n");
}

//*****************************************************************************
void VuDevHostComm::reset()
{
	release();
	init(mHostName);
}

//*****************************************************************************
VuBinaryDataWriter VuDevHostComm::beginMessage(const char *msgType)
{
	VuDevHostCommHeader header;
	memset(&header, 0, sizeof(header));

	// create request
	VuBinaryDataWriter request(mBuffer);
	request.clear();
	request.writeData(&header, sizeof(header)); // placeholder
	request.writeString(msgType);

	return request;
}

//*****************************************************************************
bool VuDevHostComm::sendMessage(bool bWait)
{
	// fix header
	VuDevHostCommHeader header;
	header.build(mBuffer);
	VU_MEMCPY(&mBuffer[0], mBuffer.size(), &header, sizeof(header));

	int totalBytesWritten = 0;
	while ( totalBytesWritten < mBuffer.size() )
	{
		int bytesWritten = mpSocket->send(&mBuffer[totalBytesWritten], mBuffer.size() - totalBytesWritten);
		if ( bytesWritten <= 0 )
		{
			VuThread::IF()->sleep(DEV_HOST_COMM_TIMEOUT_MS);
			bytesWritten = mpSocket->send(&mBuffer[totalBytesWritten], mBuffer.size() - totalBytesWritten);
			if ( bytesWritten <= 0 )
				return false;
		}
		totalBytesWritten += bytesWritten;
	}

	if ( !bWait )
		return true;

	#define CHUNK_SIZE 65536
	mBuffer.resize(0);
	for ( ;; )
	{
		int prevSize = mBuffer.size();
		mBuffer.resize(prevSize + CHUNK_SIZE);
		int bytesRead = mpSocket->recv(&mBuffer[prevSize], CHUNK_SIZE);
		if ( bytesRead < 0 )
		{
			return false;
		}
		else if ( bytesRead > 0 )
		{
			mBuffer.resize(prevSize + bytesRead);

			// decode header
			VuBinaryDataReader reader(mBuffer);
			if ( mBuffer.size() >= sizeof(VuDevHostCommHeader) )
			{
				VuDevHostCommHeader header;
				reader.readData(&header, sizeof(header));
				header.fixEndianness();

				if ( header.verify() )
				{
					int totalSize = sizeof(VuDevHostCommHeader) + header.mDataSize;
					if ( mBuffer.size() == totalSize )
						return true;
					else if ( mBuffer.size() > totalSize )
						return false;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			// Resize the buffer back to what it used to be, we didn't actually get any data this time
			mBuffer.resize(prevSize);
		}
	}

// NOTE: unreachable code, commented out to remove warning
//	return false;
}

//*****************************************************************************
VuBinaryDataReader VuDevHostComm::response()
{
	VuBinaryDataReader reader(mBuffer);
	reader.skip(sizeof(VuDevHostCommHeader));
	return reader;
}

//*****************************************************************************
const std::string &VuDevHostComm::hostName()
{
	return mHostName;
}

//*****************************************************************************
void VuDevHostComm::append(const char *str)
{
	VuDevHostCommHeader header;
	memset(&header, 0, sizeof(header));

	mLogBuffer.clear();

	VuBinaryDataWriter msg(mLogBuffer);
	msg.writeData(&header, sizeof(header)); // placeholder
	msg.writeString("log");
	msg.writeString(str);

	// fix header
	header.build(mLogBuffer);
	VU_MEMCPY(&mLogBuffer[0], mLogBuffer.size(), &header, sizeof(header));

	mpSocket->send(&mLogBuffer[0], mLogBuffer.size());
}

//*****************************************************************************
void VuDevHostCommHeader::fixEndianness()
{
	if ( mMagic == VuEndianUtil::swap(scDevHostCommMagic) )
	{
		VuEndianUtil::swapInPlace(mMagic);
		VuEndianUtil::swapInPlace(mVersion);
		VuEndianUtil::swapInPlace(mDataSize);
	}
}

//*****************************************************************************
void VuDevHostCommHeader::build(const VuArray<VUBYTE> &data)
{
	mMagic = scDevHostCommMagic;
	mVersion = scDevHostCommVersion;
	mDataSize = data.size() - sizeof(VuDevHostCommHeader);
}

//*****************************************************************************
bool VuDevHostCommHeader::verify()
{
	if ( mMagic != scDevHostCommMagic )
		return false;

	if ( mVersion != scDevHostCommVersion )
		return false;

	return true;
}

#endif // VU_DISABLE_DEV_HOST_COMM

