#pragma once
/*! Core TCP object for sending/receiving data
 */

#include "gui_class.h"

using namespace System;
using namespace System::Text;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::Drawing;
using namespace System::IO;

public ref class TcpClientNest {
public:
	TcpClient ^tcpClient;
	bool authenticated;
	bool Identified;
	int authAttempts;
	int MaxMessageSize;
	String ^cookie;

	TcpClientNest::TcpClientNest() {
		tcpClient = gcnew TcpClient;
		authenticated = false;
		authAttempts = 0;
		cookie = "";
		Identified = false;
		MaxMessageSize = 1024*1024*1024;
	}

	TcpClientNest::~TcpClientNest() {
		if(!tcpClient->Connected) return;

		NetworkStream ^ns = tcpClient->GetStream();
		if(ns) {
			ns->Close();
		}
		tcpClient->Close();
		delete tcpClient;
	}

	void TcpClientNest::SetAuthenticated(void) {
		authenticated = true;
		MaxMessageSize = 1024*1024*1024;
	}
};


//! Core TCP object for sending/receiving data.
public ref class tcp_Core {
private:
protected:
	_GUIlog ^log;
	String ^p_log;
	System::Text::ASCIIEncoding^ encoder;
	bool p_connected;
	int MaxMessageSize;
	bool verbose;
	array<System::Byte> ^buffer;

	static const int BUFFER_SIZE = 1024; 
public:
	delegate void DelEventTextMessage(TcpClientNest ^%tcpClient,String ^text);
	delegate void DelEventData(TcpClientNest ^%tcpClient,array<Byte> ^%message);
	delegate void DelEventCommand(TcpClientNest ^%tcpClient,Byte command, array<Byte> ^%argument);
	DelEventCommand ^eventRequest;
	DelEventTextMessage ^eventTextMessage;
	DelEventTextMessage ^eventFileReceived;
	DelEventCommand ^eventResponse;
	DelEventData ^eventData;
	DelEventCommand ^eventAuthenticate;
		
	//! Constructor
	tcp_Core::tcp_Core() {
		p_log = "";
		verbose = true;
		log = _GUIlog::getInstance();
		encoder = gcnew System::Text::ASCIIEncoding();
		p_connected = false;
		
		buffer = gcnew array<System::Byte>(BUFFER_SIZE);
	}
	
	template<typename TYPE>
	void IncrementArray(array<TYPE^> ^%A) {
		A->Resize(A,A->Length+1);
	}

	//! Get last string from log
	property String ^getlog {
		String ^get() {
			return p_log;
		}
	}

	//! Function that handles client communication
	 void tcp_Core::handleClientComm(Object^ clientObj) {
		TcpClientNest ^client1;
		NetworkStream ^clientStream;

		try {
			client1 = (TcpClientNest^)clientObj;
			clientStream = client1->tcpClient->GetStream();
			StringBuilder ^messageBuilder = gcnew StringBuilder;
			int result = 0;

			while(true) {

				try	{
					GC::Collect();
					if(!client1->tcpClient->Connected) return;
					messageBuilder->Clear();
					result = readTcpString(clientStream, messageBuilder);
					if (result < 0) {
						log->write("This client has been disconnected from the server.", Color::Red);
						return;
					}

				} // End try
				catch(ThreadAbortException ^ex) {
					if(verbose) log->write("TCP Aborting: " + ex->Message, Color::Blue);
					break;
				}
				catch(System::Exception^ ex) {
					p_connected = false;
					if(verbose) log->write("TCP socket error: " + ex->Message, Color::Red);
					if(!client1->tcpClient->Connected) return;
					break;
				}

				// message has successfully been received
				if (result == 0) {
					if(eventTextMessage) {
						eventTextMessage(client1,messageBuilder->ToString());
					} else {
						if(verbose) log->write("debug(TCP): " + messageBuilder->ToString(), Color::Blue);
					}
				}
			}
		}
		catch(ThreadAbortException ^ex) {
			log->write("TCP: Thread disconnect: " + ex->Message, Color::Blue);
		}
		finally {
			//clientStream->Close();
			delete client1;
			GC::Collect();
		}

	}

	 int tcp_Core::readTcpString(NetworkStream ^%clientStream, StringBuilder ^messageBuilder) {
		int count = 0;
		int Byte;

		do {
			if(clientStream->CanRead) {				
				if (clientStream->DataAvailable) {
					Byte = clientStream->ReadByte();

					if (Byte == '\n') {
						buffer[count] = 0;
						messageBuilder->Append(encoder->GetString(buffer, 0, count));
						return 0;
					}

					buffer[count] = Byte;
					count++;
				} else {
					System::Threading::Thread::Sleep(1);
				}
			} else {
				log->write("TCP client(message): Can't read TCP stream, closing");
				return -1;
			}

			if (count == buffer->Length) {
				//Buffer is full, need to append to string and reset count.
				messageBuilder->Append(encoder->GetString(buffer, 0, count));
				count = 0;
				//log->write("TCP client(message): Error incoming data buffer overrun.");
				//return -1;
			}

		} while(true);

		return 0;
	}

	int tcp_Core::readTcp(NetworkStream ^%clientStream,const int size, array<Byte> ^%tcpdata) {
					
		int bytesRead = 0;
		int bytesToRead = size;
		do {
			if(clientStream->CanRead) {
				//log->write("TCP CanRead: [" + bytesRead + "/" +bytesToRead + "/"+ size+"]");
				
				bytesRead = clientStream->Read(tcpdata, size-bytesToRead , bytesToRead);			// blocks thread until a client sends a message
				bytesToRead -= bytesRead;
			} else {
				log->write("TCP client(message): Can't read TCP stream, closing");
				return -1;
			}

			if(bytesRead == 0) {
				log->write("TCP client(message): Abort data empty.");
				return -1;
			}
		} while(bytesToRead>0);

		return size-bytesToRead;
	}

	 //! Functions that sends a message to the server.
	public:	bool sendMessage(TcpClientNest ^client, String^ message) {
				array<Byte> ^buffer;

				// Initializing message
				if(!message->EndsWith("\n")) {
					message += "\n";
				}
				buffer = encoder->GetBytes(message);
				
				return sendClientCommunication(client, buffer);
		}

	public:	bool sendClientCommunication(TcpClientNest ^client1, array<Byte> ^buffer) {
		if(!client1) return false;
		if(!client1->tcpClient->Connected) {
			return false;
		}
		NetworkStream ^clientStream;

		// Initializing message
		try {
			clientStream = client1->tcpClient->GetStream();
			if ( clientStream->CanWrite ) {
				clientStream->Write(buffer, 0, buffer->Length);
			} else {
				log->write("TCP: Error can't write header to tcp stream...");
				return false;
			}
		}
		catch(System::ObjectDisposedException ^ex) {
			p_log = ex->Message;
			return false;				
		}
		catch(System::InvalidOperationException ^ex) {
			p_log = ex->Message;
			return false;
		}
		catch(Exception ^ex) {
			p_log = ex->Message;
			return false;
		}

		return true;
	}

//! Returns true is NetworkStream^ object is disposed
	protected: bool isDisposed(TcpClientNest ^%client1) {
			 NetworkStream ^clientStream;

			 try {
				 clientStream = client1->tcpClient->GetStream();
			 }
			 catch(System::ObjectDisposedException ^ex) {
				 p_log = ex->Message;
				 return true;
				 
			 }
			 catch(System::InvalidOperationException ^ex) {
				 p_log = ex->Message;
				 return true;
			 }
			 catch(Exception ^ex) {
				 p_log = ex->Message;
				 return true;
			 }
			 return false;
		 }
};