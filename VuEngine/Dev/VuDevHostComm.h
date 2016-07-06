//*****************************************************************************
//
//  Copyright (c) 2011-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Dev Log
// 
//*****************************************************************************

#pragma once

#include "VuEngine/VuSystemComponent.h"
#include "VuEngine/Containers/VuArray.h"
#include "VuEngine/Util/VuBinaryDataUtil.h"

class VuEngine;
class VuTcpSocket;

#define DEV_HOST_COMM_PORT 28236


#if VU_DISABLE_DEV_HOST_COMM

	class VuDevHostComm : public VuSystemComponent
	{
		DECLARE_SYSTEM_COMPONENT(VuDevHostComm)

	protected:
		friend class VuEngine;
		virtual bool init(const std::string &host) { return true; }

	public:
		VuBinaryDataWriter	beginMessage(const char *msgType);
		bool				sendMessage(bool bWait) { return false; }
		VuBinaryDataReader	response() { return VuBinaryDataReader(mBuffer); }

		bool				isConnected() { return false; }
		const std::string	&hostName() { return mHostName; }

	private:
		std::string		mHostName;
		VuArray<VUBYTE>	mBuffer;
	};

#else

	struct VuDevHostCommHeader
	{
		VUUINT32	mMagic;
		VUUINT32	mVersion;
		VUUINT32	mDataSize;

		void		fixEndianness();
		void		build(const VuArray<VUBYTE> &data);
		bool		verify();
	};

	class VuDevHostComm : public VuSystemComponent, public VuSys::LogCallback
	{
		DECLARE_SYSTEM_COMPONENT(VuDevHostComm)

	protected:
		// called by engine
		friend class VuEngine;
		virtual bool init(const std::string &host);
		virtual void release();

	public:
		VuDevHostComm();

		void				reset();

		VuBinaryDataWriter	beginMessage(const char *msgType);
		bool				sendMessage(bool bWait);
		VuBinaryDataReader	response();

		bool				isConnected() { return mpSocket != VUNULL; }
		const std::string	&hostName();

	private:
		// VuSys::LogCallback
		virtual void	append(const char *str);

		std::string		mHostName;
		VuTcpSocket		*mpSocket;
		VUUINT32		mHostIpAddr;

		VuArray<VUBYTE>	mBuffer;
		VuArray<VUBYTE>	mLogBuffer;
	};

#endif // VU_DISABLE_DEV_CONSOLE
