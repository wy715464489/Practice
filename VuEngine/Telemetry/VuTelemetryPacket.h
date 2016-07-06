//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Telemetry Packets
// 
//*****************************************************************************

#pragma once

class VuTcpSocket;
class VuJsonContainer;


#define TELEMETRY_SERVER_PORT 28234
#define TELEMETRY_TIMEOUT_MS 2000 // milliseconds


namespace VuTelemetryPacket
{
	bool	receive(VuTcpSocket *pSocket, VuJsonContainer &packet);
	bool	send(VuTcpSocket *pSocket, const VuJsonContainer &packet);
};
