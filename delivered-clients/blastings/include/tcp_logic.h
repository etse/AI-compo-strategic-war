#pragma once

#include "tcp_networkclient.h"
#include "version_class.h"
#include "../RTS-AI/GameState.h"
#include "../RTS-AI/GameStateRenderer.h"
#include "../RTS-AI/AI.h"

using namespace System;

public ref class _Tcp_Logic {

protected:
	delegate void TextEvent(String^ text);
	delegate void LabelTextEvent(String^ text, Label ^label);

	_GUIlog ^log;
	TcpNetworkClient ^tcpConnect;
	double maxMove;
	double minMove;
	double totalMoveTime;
	double averageMove;
	double roundCounter;
	double maxRound;
	double minRound;
	double averageRound;
	double totalRoundTime;

	StringBuilder ^debugLog;

	DateTime  lastTime;
public:
	
	CheckBox ^ShowDebug;
	
	Label ^aiTime;
	Label ^roundTime;
	Label ^antLabel;
	GameState ^gameState;
	GameStateRenderer ^renderer;
	AI ^ai;

	_Tcp_Logic::_Tcp_Logic() {
		log = _GUIlog::getInstance();
		renderer = GameStateRenderer::getInstance();
		gameState = gcnew GameState();
		ai = gcnew AI(gameState);
		maxMove = 0.0;
		minMove = 999.0;
		totalMoveTime = 0.0;
		averageMove = 0.0;
		roundCounter = 0.0;
		maxRound = 0.0;
		minRound = 999.0;
		averageRound = 0.0;
		totalRoundTime = 0.0;

		debugLog = gcnew StringBuilder();
	}

	void dropHandles(void) {		
		if(tcpConnect->eventTextMessage) delete tcpConnect->eventTextMessage;
	}

	void AddHandler(TcpNetworkClient ^%tcpConnect) {
		this->tcpConnect = tcpConnect;
		tcpConnect->eventTextMessage = gcnew tcp_Core::DelEventTextMessage(this,&_Tcp_Logic::TCPTextHandler);
	}

	void saveDebugData() {
		try {
			StreamWriter^ pwriter = gcnew StreamWriter(Application::StartupPath + "\\debug.log", true);
			pwriter->Write(debugLog->ToString());
			pwriter->Flush();
			pwriter->Close();
		} catch (Exception ^Ex) {
			log->write("Error writing log file: " + Ex->Message);
		}
	}
	
	private: void TCPTextHandler(TcpClientNest ^%tcpClient,String ^text) {
			try {
				//log->write("server: " + text);
				DateTime begin = DateTime::UtcNow;
				if (!gameState->process(text)) {
					//Received status, skipping turn
					return;
				}

				//debugLog->AppendLine("round(" + gameState->roundCounter + ") server: " + text);
				String ^commands = ai->getCommands();
				roundCounter++;
				DateTime end = DateTime::UtcNow;
				debugLog->AppendLine("round(" + gameState->roundCounter + ") response: " + commands);
				tcpConnect->sendMessage(commands);
				//Calculate timing
				double currentMove = (end - begin).TotalMilliseconds;
				totalMoveTime += currentMove;
				if (currentMove < minMove) {
					minMove = currentMove;
				} else if (currentMove > maxMove) {
					maxMove = currentMove;
				}
				averageMove = totalMoveTime / roundCounter;
				if (((int)roundCounter % 10) == 0) {
					setAiTime("Ai Time: Current(" + currentMove + ") Min(" + minMove + ") Max(" + maxMove + ") Avg(" + averageMove + ")");
				}

				//System::Threading::Thread::Sleep(5);
				setLabelText("Ants: Standard(" + gameState->myStandardAnts->Count + ") Harvesters(" + gameState->myHarvesters->Count + ") Soldiers(" + gameState->mySoldiers->Count + ")", antLabel);
				//renderer->render(gameState);

				if (!(lastTime == DateTime::MinValue)) {
					DateTime now = DateTime::UtcNow;
					double currentRound = (now - lastTime).TotalMilliseconds;
					totalRoundTime += currentRound;
					if (currentRound < minRound) {
						minRound = currentRound;
					} else if (currentRound > maxRound) {
						maxRound = currentRound;
					}
					averageRound = totalRoundTime / roundCounter;
					if (((int)roundCounter % 10) == 0) {
						setRoundTime("Round Time: Current(" + currentRound + ") Min(" + minRound + ") Max(" + maxRound + ") Avg(" + averageRound + ")");
					}
					lastTime = now;
				} else {
					lastTime = DateTime::UtcNow;
				}
			} catch (Exception ^ex) {
				log->write("Exception caught: " + ex->Message + " stack: " + ex->StackTrace);
			}
		}

	void setAiTime(String ^text) {
		if (aiTime) {
			if (this->aiTime->InvokeRequired) {
				aiTime->BeginInvoke(gcnew TextEvent(this, &_Tcp_Logic::setAiTime), text);
				return;
			}
		}
		aiTime->Text = text;
	}
	void setRoundTime(String ^text) {
		if (roundTime) {
			if (this->roundTime->InvokeRequired) {
				roundTime->BeginInvoke(gcnew TextEvent(this, &_Tcp_Logic::setRoundTime), text);
				return;
			}
		}
		roundTime->Text = text;
	}

	void setLabelText(String ^text, Label ^label) {
		if (label && text) {
			if (label->InvokeRequired) {
				label->BeginInvoke(gcnew LabelTextEvent(this, &_Tcp_Logic::setLabelText), text, label);
				return;
			}
		}
		label->Text = text;
	}

};