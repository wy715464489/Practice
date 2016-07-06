//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Telemetry Client class
// 
//*****************************************************************************

#include "VuTelemetryClient.h"
#include "VuTelemetryPacket.h"
#include "VuEngine/HAL/Net/VuNet.h"
#include "VuEngine/Json/VuJsonContainer.h"


#if VU_DISABLE_TELEMETRY

// internal data
class VuTelemetryClientImpl : public VuTelemetryClient
{
public:
	virtual bool	init() { return true; }

	virtual bool	getNamespace(const std::string &strAddress, VuJsonContainer &namespaceData) { return false; }
	virtual bool	getProperties(const std::string &strAddress, const std::string &strLongName, VuJsonContainer &propertyData) { return false; }
	virtual bool	setProperties(const std::string &strAddress, const std::string &strLongName, const VuJsonContainer &propertyData) { return false; }
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTelemetryClient, VuTelemetryClientImpl);

#else // VU_DISABLE_TELEMETRY

// internal data
class VuTelemetryClientImpl : public VuTelemetryClient
{
public:
	VuTelemetryClientImpl() {}

	virtual bool	init();

	virtual bool	getNamespace(const std::string &strAddress, VuJsonContainer &namespaceData);
	virtual bool	getProperties(const std::string &strAddress, const std::string &strLongName, VuJsonContainer &propertyData);
	virtual bool	setProperties(const std::string &strAddress, const std::string &strLongName, const VuJsonContainer &propertyData);

private:
	bool	sendData(const std::string &strAddress, const char *strType, VuJsonContainer &data, VuJsonContainer &response);
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTelemetryClient, VuTelemetryClientImpl);


//*****************************************************************************
bool VuTelemetryClientImpl::init()
{
	return true;
}

//*****************************************************************************
bool VuTelemetryClientImpl::getNamespace(const std::string &strAddress, VuJsonContainer &namespaceData)
{
	// send request
	VuJsonContainer data;
	return sendData(strAddress, "GetNamespaceRequest", data, namespaceData);
}

//*****************************************************************************
bool VuTelemetryClientImpl::getProperties(const std::string &strAddress, const std::string &strLongName, VuJsonContainer &propertyData)
{
	// build data
	VuJsonContainer data;
	data["Name"].putValue(strLongName);

	// send request
	return sendData(strAddress, "GetPropertiesRequest", data, propertyData);
}

//*****************************************************************************
bool VuTelemetryClientImpl::setProperties(const std::string &strAddress, const std::string &strLongName, const VuJsonContainer &propertyData)
{
	// build data
	VuJsonContainer data;
	data["Name"].putValue(strLongName);
	data["Properties"] = propertyData;

	// send request
	VuJsonContainer response;
	return sendData(strAddress, "SetProperties", data, response);
}

//*****************************************************************************
bool VuTelemetryClientImpl::sendData(const std::string &strAddress, const char *strType, VuJsonContainer &data, VuJsonContainer &response)
{
	bool success = false;

	// create socket
	if ( VuTcpSocket *pSocket = VuTcpSocket::create(0) )
	{
		if ( pSocket->setTimeOut(TELEMETRY_TIMEOUT_MS, TELEMETRY_TIMEOUT_MS) )
		{
			// connect to server
			if ( pSocket->connect(strAddress.c_str(), TELEMETRY_SERVER_PORT, TELEMETRY_TIMEOUT_MS) )
			{
				// build packet
				data["Type"].putValue(strType);

				if ( VuTelemetryPacket::send(pSocket, data) )
				{
					success = VuTelemetryPacket::receive(pSocket, response);
				}
			}
		}

		delete pSocket;
	}

	return success;
}

#endif // VU_DISABLE_TELEMETRY
