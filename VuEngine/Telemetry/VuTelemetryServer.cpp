//*****************************************************************************
//
//  Copyright (c) 2007-2008 Ralf Knoesel
//  Copyright (c) 2008-2013 Vector Unit Inc
//  Confidential Trade Secrets
// 
//  Telemetry Server class
// 
//*****************************************************************************

#include "VuTelemetryServer.h"
#include "VuTelemetryPacket.h"
#include "VuEngine/Entities/VuEntity.h"
#include "VuEngine/Entities/VuEntityRepository.h"
#include "VuEngine/Entities/VuEntityUtil.h"
#include "VuEngine/Components/VuComponent.h"
#include "VuEngine/Properties/VuProperty.h"
#include "VuEngine/Properties/VuEnumProperty.h"
#include "VuEngine/Properties/VuAssetProperty.h"
#include "VuEngine/Pfx/VuPfx.h"
#include "VuEngine/Managers/VuTickManager.h"
#include "VuEngine/HAL/Net/VuNet.h"


#if VU_DISABLE_TELEMETRY

// internal data
class VuTelemetryServerImpl : public VuTelemetryServer
{
public:
	virtual bool	init() { return true; }
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTelemetryServer, VuTelemetryServerImpl);

#else // VU_DISABLE_TELEMETRY

// internal data
class VuTelemetryServerImpl : public VuTelemetryServer
{
public:
	VuTelemetryServerImpl() : mpSocket(VUNULL) {}

	virtual bool	init();
	virtual void	release();

private:
	void			tick(float fdt);

	void			handleSetPropertiesCommand(const VuJsonContainer &data, VuTcpSocket *pSocket) const;
	void			handleGetNamespaceRequestCommand(const VuJsonContainer &data, VuTcpSocket *pSocket) const;
	void			handleGetPropertiesRequestCommand(const VuJsonContainer &data, VuTcpSocket *pSocket) const;

	void			addProperties(VuJsonContainer &packet, VuProperties *pProperties) const;

	VuTcpSocket		*mpSocket;
};

// the interface
IMPLEMENT_SYSTEM_COMPONENT(VuTelemetryServer, VuTelemetryServerImpl);


//*****************************************************************************
bool VuTelemetryServerImpl::init()
{
	mpSocket = VuTcpSocket::create(TELEMETRY_SERVER_PORT);
	if ( mpSocket == VUNULL )
		return VUWARNING("VuTelemetryServer: Unable to create tcp socket.");

	if ( !mpSocket->setNonBlocking(true) )
		return VUWARNING("VuTelemetryServer: Unable to set socket to non-blocking.");

	if ( !mpSocket->setTimeOut(TELEMETRY_TIMEOUT_MS, TELEMETRY_TIMEOUT_MS) )
		return VUWARNING("VuTelemetryServer: Unable to set socket to non-blocking.");

	if ( !mpSocket->listen(8) )
		return VUWARNING("VuTelemetryServer: Unable to set socket to listen.");

	// register phased tick
	VuTickManager::IF()->registerHandler(this, &VuTelemetryServerImpl::tick, "Network");

	return true;
}

//*****************************************************************************
void VuTelemetryServerImpl::release()
{
	// unregister phased tick
	VuTickManager::IF()->unregisterHandlers(this);

	delete mpSocket;
}

//*****************************************************************************
void VuTelemetryServerImpl::tick(float fdt)
{
	while ( VuTcpSocket *pSocket = mpSocket->accept() )
	{
		if ( pSocket->setTimeOut(TELEMETRY_TIMEOUT_MS, TELEMETRY_TIMEOUT_MS) )
		{
			VuJsonContainer packet;
			if ( VuTelemetryPacket::receive(pSocket, packet) )
			{
				std::string packetType = packet["Type"].asString();
				if ( packetType == "SetProperties" )
				{
					handleSetPropertiesCommand(packet, pSocket);
				}
				else if ( packetType == "GetNamespaceRequest" )
				{
					handleGetNamespaceRequestCommand(packet, pSocket);
				}
				else if ( packetType == "GetPropertiesRequest" )
				{
					handleGetPropertiesRequestCommand(packet, pSocket);
				}
			}
		}

		delete pSocket;
	}
}

//*****************************************************************************
void VuTelemetryServerImpl::handleSetPropertiesCommand(const VuJsonContainer &data, VuTcpSocket *pSocket) const
{
	std::string root = VuEntityUtil::getRoot(data["Name"].asString());
	std::string name = VuEntityUtil::subtractRoot(data["Name"].asString());

	const VuJsonContainer &propertyData = data["Properties"];
	for ( int i = 0; i < propertyData.numMembers(); i++ )
	{
		const std::string &propertyName = propertyData.getMemberKey(i);
		const VuJsonContainer &propertyValue = propertyData[propertyName];

		if ( root == "Entities" )
		{
			if ( VuEntity *pEntity = VuEntityRepository::IF()->findEntity(name.c_str()) )
				if ( VuProperty *pProperty = pEntity->getProperty(propertyName) )
					pProperty->setCurrent(propertyValue);
		}
		else if ( root == "Pfx" )
		{
			if ( VuProperties *pProperties = VuPfx::IF()->getProperties(name.c_str()) )
				if ( VuProperty *pProperty = pProperties->get(propertyName) )
					pProperty->setCurrent(propertyValue);
		}
	}

	// send response
	VuJsonContainer response;
	VuTelemetryPacket::send(pSocket, response);
}

//*****************************************************************************
void VuTelemetryServerImpl::handleGetNamespaceRequestCommand(const VuJsonContainer &data, VuTcpSocket *pSocket) const
{
	// build packet
	VuJsonContainer packet;
	packet["Type"].putValue("GenericResponse");
	packet["RequestId"].putValue(data["RequestId"].asInt());

	// send entity data
	packet["Data"].clear();
	VuEntityRepository::IF()->getEntityData(packet["Data"]["Entities"]);
	VuTelemetryPacket::send(pSocket, data);

	// send pfx data
	packet["Data"].clear();
	VuPfx::IF()->getNamespace(packet["Data"]["Pfx"]);
	VuTelemetryPacket::send(pSocket, data);
}

//*****************************************************************************
void VuTelemetryServerImpl::handleGetPropertiesRequestCommand(const VuJsonContainer &data, VuTcpSocket *pSocket) const
{
	// build packet
	VuJsonContainer packet;
	packet["Type"].putValue("GenericResponse");
	packet["RequestId"].putValue(data["RequestId"].asInt());

	std::string root = VuEntityUtil::getRoot(data["Name"].asString());
	std::string name = VuEntityUtil::subtractRoot(data["Name"].asString());

	if ( root == "Entities" )
	{
		if ( VuEntity *pEntity = VuEntityRepository::IF()->findEntity(name.c_str()) )
		{
			addProperties(packet["Data"], &pEntity->getProperties());
			for ( VuComponent *pComponent = pEntity->getComponentList().getFirst(); pComponent; pComponent = pComponent->getNextComponent() )
			{
				if ( pComponent->getProperties().hasProperties() )
				{
					VuJsonContainer &folderData = packet["Data"].append();
					folderData["Type"].putValue("Folder");
					folderData["Name"].putValue(pComponent->getShortComponentType());
					addProperties(folderData["Data"], &pComponent->getProperties());
				}
			}
		}
	}
	else if ( root == "Pfx" )
	{
		addProperties(packet["Data"], VuPfx::IF()->getProperties(name.c_str()));
	}

	// send packet
	VuTelemetryPacket::send(pSocket, data);
}

//*****************************************************************************
void VuTelemetryServerImpl::addProperties(VuJsonContainer &data, VuProperties *pProperties) const
{
	if ( pProperties )
	{
		for ( VuProperty *pProperty = pProperties->getFirst(); pProperty != VUNULL; pProperty = pProperty->getNextProperty() )
		{
			if ( pProperty->isEnabled() )
			{
				VuJsonContainer &propertyData = data.append();

				propertyData["Name"].putValue(pProperty->getName());
				propertyData["Type"].putValue(pProperty->getType());
				pProperty->getCurrent(propertyData["Value"]);
				pProperty->getDefault(propertyData["Default"]);

				if ( pProperty->getType() == VuProperty::ASSET ||
					 pProperty->getType() == VuProperty::AUDIO )
				{
					// treat asset & audio properties like strings because there are just too many
					// enum choices to send over the network
					propertyData["Type"].putValue(VuProperty::STRING);
				}
				else if ( pProperty->getType() == VuProperty::ENUM_STRING )
				{
					VuStringEnumProperty *pStringEnumProperty = static_cast<VuStringEnumProperty *>(pProperty);
					for ( int i = 0; i < pStringEnumProperty->getChoiceCount(); i++ )
						propertyData["Choices"][i].putValue(pStringEnumProperty->getChoice(i));
				}
				else if ( pProperty->getType() == VuProperty::ENUM_INT )
				{
					VuIntEnumProperty *pIntEnumProperty = static_cast<VuIntEnumProperty *>(pProperty);
					for ( int i = 0; i < pIntEnumProperty->getChoiceCount(); i++ )
						propertyData["Choices"][i].putValue(pIntEnumProperty->getChoiceName(i));
				}
			}
		}
	}
}

#endif // VU_DISABLE_TELEMETRY
