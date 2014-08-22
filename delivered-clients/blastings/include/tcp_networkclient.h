#pragma once

#include "tcp_core.h"
#include "gui_class.h"

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Threading;


//! Network-client class
public ref class TcpNetworkClient : tcp_Core {
protected:
	delegate void WriteEvent(String ^%);
	String ^ip;
	int port;
private:	
	TcpClientNest^ client;
	Thread^ clientThread;
public:

	//! Constructor
	TcpNetworkClient() {
		p_log = "";	
		port = 0;
		ip = "";
		verbose = false;
	}

	//! Destructor
	~TcpNetworkClient() {
		this->!TcpNetworkClient();
	}
	//! Finalizer
	!TcpNetworkClient() {
		disconnectFromServer();
	}

//! Returns connection-status
public:	property bool connected {
		bool get() {
			return p_connected;
		}
	}

//! Returns Authenticated status
public:	property bool Authenticated {
			bool get() {
				return client->authenticated;
			}
		}

//! Connects to specified host/ip,port
public: bool connectToServer(String^ ip, int port) {
			this->ip = ip;
			this->port = port;

			return connectToServer();
		}

//! Connects to server using stored values
protected: bool connectToServer(void) {
		client = gcnew TcpClientNest();

		try {
			client->tcpClient->Connect(ip, port);
		}
		catch(Exception ^ex) {
			p_log = ex->Message;
			p_connected = false;
			return false;
		}

		p_connected = true;
		client->authenticated = false;
		// Initializing new client thread
		clientThread = gcnew Thread(gcnew ParameterizedThreadStart(this, &TcpNetworkClient::handleClientComm));

		// Starting new client thread
		clientThread->Start(client);
		log->write("Connected to: " + ip + " port: " + port);
		return true;
	}

public: void EnableNologinMode(void) {
			verbose = true;
		}

//! Reconnects to the server
public: bool reConnect(void) {
			bool status = false;

			if(!p_connected) {
				if( ( ip->Length > 3) && (port != 0)) {
					status = connectToServer();
				}
			}

			return status;
		}		

//! Functions that sends a message to the server.
public:	bool sendMessage(String^ message) {
			if(!client) return false;

			return sendMessage(client, message);
		}

//! Disconnects from the server
public:	void disconnectFromServer() {

//		sendRequest(DataHeader::COMMAND::Terminate);
		verbose = false;
		
		try {
			if(clientThread) {
				clientThread->Abort();
			}

			int x=0;
			while(true) {
				Thread::Sleep(10);
				if(!client->tcpClient->Connected) break;
				x++;
				if(x==100) {
					delete client;
					x=0;
				}
			}

		}catch(System::Threading::ThreadAbortException^ ex){
			log->write("~TcpNetworkClient: " + ex->Message, Color::Red);
		}catch(Exception^ ex){
			log->write("~TcpNetworkClient(sys): " + ex->Message, Color::Red);
		}

		p_connected = false;
	}

};
