#include "test_server.h"
#include "net/socket_ops.h"
#include <vector>
#include <string>
#include <sstream>
#include "common/log.h"
#include "proto/communication/common_enum.pb.h"

namespace testserver {

using common::LogSystem;
using net::MessageHeader;
using common::ERROR_LOG;
using common::DEBUG_LOG;
using common::INFO_LOG;
using std::make_pair;
using net::NetworkToAddress;
void TestServer::connect_gateway() {
	// connect gateways
	printf("connect gateway\n");
	net::InetAddress gateway_addr;
	std::vector<std::string> ip_port;

	if (!gateway_addr.init("127.0.0.1", 8888)) {
		ERROR_LOG("Failed to init gateway address:%s\n", "127.0.0.1");
		::exit(EXIT_FAILURE);
	}

	GatewayClient* gateway_client = new GatewayClient(loop_, gateway_addr,
			std::tr1::bind(&TestServer::handle_message, this, 1,  // gateway client id
					std::tr1::placeholders::_1), 50);
	gateway_client->connect();
	gateway_clients_.push_back(gateway_client);
}

void TestServer::reload() {
	reload_ = true;
}

void TestServer::exit() {
	exit_ = true;
}

void TestServer::init(net::EventLoop* loop) {
	loop_ = loop;
	// subsystem_conf_ = conf.subsystem_conf;
	sample_message_ = 1;
	id_ = 4001;

	// register message handler
	register_message_handler(1234,
			std::tr1::bind(&TestServer::UseCDkeyRequest, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	// subsystems register
	subsystem_register();

	// subsystems init
	subsystem_reload();

	// connect gateway
	connect_gateway();
}

int TestServer::id() const {
	return id_;
}

TestServer::TestServer() :
		sample_message_(1000), id_(0), exit_(false), reload_(false), keep_alive_(0), loop_(NULL) {
}

TestServer::~TestServer() {
	for (size_t i = 0; i < gateway_clients_.size(); i++) {
		delete gateway_clients_[i];
	}
}

bool TestServer::handle_message(int gateway_id, const Message& message) {
	/*
	 printf("CdkeyServer::handle_message, gateway_id:%d\n",
	 gateway_id);
	 printf("message type:%d body:%s\n", message.header().type,
	 message.body());
	 */
	DEBUG_LOG("message arrives: length:%u, type:%u,"
            "src_id=%u, dst_id=%u, owner_conn=%lu,"
            "src_ip=%s sequence:%u checksum:%u\n",
            message.header().length(),
            message.header().type,
            message.header().src_id,
            message.header().dst_id,
            message.owner(),
            NetworkToAddress(message.src_ip()),
            message.header().sequence(),
            message.header().checksum);


	// std::string data(message.body(), message.body_length());

	// const int type = message.header().type;
	// TestServerMessageHandlers::iterator it = message_handlers_.find(type);
	// if (it != message_handlers_.end()) {
	// 	if (random() % sample_message_ == 0) {
	// 		struct timeval begin, end;
	// 		gettimeofday(&begin, NULL);

	// 		it->second(gateway_id, message);

	// 		gettimeofday(&end, NULL);
	// 		uint64_t elapsed_in_us = (end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec);
	// 		INFO_LOG("CdkeyServerMessageSample\ttype:%d\tprocess_time_in_us:%lu\n", type, elapsed_in_us);
	// 	} else {
	// 		it->second(gateway_id, message);
	// 	}
	// } else {
	// 	ERROR_LOG("Unknown message type: %d\n", type);
	// }

	return true;
}

void TestServer::register_message_handler(int type, TestServerMessageHandler message_handler) {
	if (!message_handlers_.insert(make_pair(type, message_handler)).second) {
		ERROR_LOG("Failed to register CdkeyServer message handler,"
				"type %d already be registered\n", type);
		::exit(EXIT_FAILURE);
	}
}

void TestServer::send_message_through_gateway(const Message& message) {
	send_message_through_gateway(1, message);
}

void TestServer::send_message_through_gateway(int gateway_idx, const Message& message) {
	if (gateway_idx >= 0 && static_cast<size_t>(gateway_idx) < gateway_clients_.size()) {
		gateway_clients_[gateway_idx]->send(reinterpret_cast<const char*>(&message.header()), sizeof(message.header()));
		gateway_clients_[gateway_idx]->send(message.body(), message.body_length());
	} else {
		ERROR_LOG("gateway id failed, gateway_idx:%d, FUNCTION:%s\n", gateway_idx, __FUNCTION__);
		// TODO(Weitong): error log
	}
}

void TestServer::update() {

	// 每10秒钟持久化到硬盘
	if (keep_alive_ % 10 == 0) {
		common::FlushLog();
    // 定期回收tcp连接的buffer
    for (size_t i = 0; i < gateway_clients_.size(); i++) {
      gateway_clients_[i]->shrink_connection_buffer();
    }
	}

	if (exit_) {
		if (!gateway_clients_.empty()) {
			INFO_LOG("Disconnect from gateway before exit\n");
			for (size_t i = 0; i < gateway_clients_.size(); i++) {
				delete gateway_clients_[i];
			}
			gateway_clients_.resize(0);
		} else {
			INFO_LOG("Exit success!\n");
			::exit(EXIT_SUCCESS);
		}
	} else {
		if (reload_) {
			subsystem_reload();
			reload_ = false;
		}

		// TODO(Weitong): 防止公司路由器由于没有数据关闭连接
		if ((++keep_alive_ % 30 == 0) && !gateway_clients_.empty()) {
			MessageHeader header;
			header.set_length(sizeof(header));
			header.type = lm::GATEWAY_KEEPALIVE;
			header.src_id = id_;
			for (size_t i = 0; i < gateway_clients_.size(); i++) {
				gateway_clients_[i]->send(reinterpret_cast<const char*>(&header), sizeof(header));
			}
			INFO_LOG("Send keep alive message to gateways\n");
		}
	}
}

void TestServer::UseCDkeyRequest(int gateway_id, const Message& message) {
	
}

void TestServer::subsystem_reload() {
}

void TestServer::subsystem_register() {

}

}
