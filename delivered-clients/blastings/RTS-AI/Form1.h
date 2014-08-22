#pragma once
#include "..\include\gui_class.h"
#include "..\include\tcp_networkclient.h"
#include "..\include\tcp_logic.h"
#include "..\include\datatypes.h"
#include "..\include\func_painter.h"
#include "..\include\version_class.h"
#include "GameStateRenderer.h"

namespace RTSAI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  btn_Connect;
	private: System::Windows::Forms::Panel^  panel_Connection;
	protected:

	private: System::Windows::Forms::Label^  lbl_Server;
	private: System::Windows::Forms::TextBox^  txtBox_Server;
	private: System::Windows::Forms::Label^  lbl_Port;
	private: System::Windows::Forms::TextBox^  txtBox_Port;
	private: System::Windows::Forms::Label^  lbl_Name;
	private: System::Windows::Forms::TextBox^  txtBox_Name;
	private: System::Windows::Forms::ListView^  listViewLog;
	private: System::Windows::Forms::PictureBox^  pictureBox_Canvas;
	private: System::Windows::Forms::Label^  lbl_RoundTime;
	private: System::Windows::Forms::Label^  lbl_AITime;
	private: System::Windows::Forms::Panel^  panel1;
	private: System::Windows::Forms::Button^  btn_debugLog;
	private: System::Windows::Forms::Button^  btn_disconnect;
	private: System::Windows::Forms::Label^  lbl_ant;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->btn_Connect = (gcnew System::Windows::Forms::Button());
			this->panel_Connection = (gcnew System::Windows::Forms::Panel());
			this->lbl_Server = (gcnew System::Windows::Forms::Label());
			this->txtBox_Server = (gcnew System::Windows::Forms::TextBox());
			this->lbl_Port = (gcnew System::Windows::Forms::Label());
			this->txtBox_Port = (gcnew System::Windows::Forms::TextBox());
			this->lbl_Name = (gcnew System::Windows::Forms::Label());
			this->txtBox_Name = (gcnew System::Windows::Forms::TextBox());
			this->btn_debugLog = (gcnew System::Windows::Forms::Button());
			this->btn_disconnect = (gcnew System::Windows::Forms::Button());
			this->lbl_RoundTime = (gcnew System::Windows::Forms::Label());
			this->lbl_AITime = (gcnew System::Windows::Forms::Label());
			this->listViewLog = (gcnew System::Windows::Forms::ListView());
			this->pictureBox_Canvas = (gcnew System::Windows::Forms::PictureBox());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->lbl_ant = (gcnew System::Windows::Forms::Label());
			this->panel_Connection->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox_Canvas))->BeginInit();
			this->panel1->SuspendLayout();
			this->SuspendLayout();
			// 
			// btn_Connect
			// 
			this->btn_Connect->Location = System::Drawing::Point(3, 105);
			this->btn_Connect->Name = L"btn_Connect";
			this->btn_Connect->Size = System::Drawing::Size(75, 23);
			this->btn_Connect->TabIndex = 0;
			this->btn_Connect->Text = L"Connect";
			this->btn_Connect->UseVisualStyleBackColor = true;
			this->btn_Connect->Click += gcnew System::EventHandler(this, &Form1::btn_Connect_Click);
			// 
			// panel_Connection
			// 
			this->panel_Connection->BackColor = System::Drawing::Color::Transparent;
			this->panel_Connection->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel_Connection->Controls->Add(this->lbl_Server);
			this->panel_Connection->Controls->Add(this->txtBox_Server);
			this->panel_Connection->Controls->Add(this->lbl_Port);
			this->panel_Connection->Controls->Add(this->txtBox_Port);
			this->panel_Connection->Controls->Add(this->lbl_Name);
			this->panel_Connection->Controls->Add(this->txtBox_Name);
			this->panel_Connection->Controls->Add(this->btn_debugLog);
			this->panel_Connection->Controls->Add(this->btn_disconnect);
			this->panel_Connection->Controls->Add(this->btn_Connect);
			this->panel_Connection->Location = System::Drawing::Point(13, 566);
			this->panel_Connection->Name = L"panel_Connection";
			this->panel_Connection->Size = System::Drawing::Size(411, 166);
			this->panel_Connection->TabIndex = 1;
			// 
			// lbl_Server
			// 
			this->lbl_Server->AutoSize = true;
			this->lbl_Server->Location = System::Drawing::Point(10, 52);
			this->lbl_Server->Name = L"lbl_Server";
			this->lbl_Server->Size = System::Drawing::Size(41, 13);
			this->lbl_Server->TabIndex = 2;
			this->lbl_Server->Text = L"Server:";
			// 
			// txtBox_Server
			// 
			this->txtBox_Server->Location = System::Drawing::Point(51, 49);
			this->txtBox_Server->Name = L"txtBox_Server";
			this->txtBox_Server->Size = System::Drawing::Size(100, 20);
			this->txtBox_Server->TabIndex = 1;
			this->txtBox_Server->Text = L"127.0.0.1";
			// 
			// lbl_Port
			// 
			this->lbl_Port->AutoSize = true;
			this->lbl_Port->Location = System::Drawing::Point(10, 29);
			this->lbl_Port->Name = L"lbl_Port";
			this->lbl_Port->Size = System::Drawing::Size(29, 13);
			this->lbl_Port->TabIndex = 2;
			this->lbl_Port->Text = L"Port:";
			// 
			// txtBox_Port
			// 
			this->txtBox_Port->Location = System::Drawing::Point(51, 26);
			this->txtBox_Port->Name = L"txtBox_Port";
			this->txtBox_Port->Size = System::Drawing::Size(100, 20);
			this->txtBox_Port->TabIndex = 1;
			this->txtBox_Port->Text = L"5050";
			// 
			// lbl_Name
			// 
			this->lbl_Name->AutoSize = true;
			this->lbl_Name->Location = System::Drawing::Point(10, 7);
			this->lbl_Name->Name = L"lbl_Name";
			this->lbl_Name->Size = System::Drawing::Size(38, 13);
			this->lbl_Name->TabIndex = 2;
			this->lbl_Name->Text = L"Name:";
			// 
			// txtBox_Name
			// 
			this->txtBox_Name->Location = System::Drawing::Point(51, 4);
			this->txtBox_Name->Name = L"txtBox_Name";
			this->txtBox_Name->Size = System::Drawing::Size(100, 20);
			this->txtBox_Name->TabIndex = 1;
			this->txtBox_Name->Text = L"BlackBot";
			// 
			// btn_debugLog
			// 
			this->btn_debugLog->Location = System::Drawing::Point(281, 7);
			this->btn_debugLog->Name = L"btn_debugLog";
			this->btn_debugLog->Size = System::Drawing::Size(108, 23);
			this->btn_debugLog->TabIndex = 0;
			this->btn_debugLog->Text = L"Write Debug Log";
			this->btn_debugLog->UseVisualStyleBackColor = true;
			this->btn_debugLog->Click += gcnew System::EventHandler(this, &Form1::btn_debugLog_Click);
			// 
			// btn_disconnect
			// 
			this->btn_disconnect->Location = System::Drawing::Point(3, 134);
			this->btn_disconnect->Name = L"btn_disconnect";
			this->btn_disconnect->Size = System::Drawing::Size(75, 23);
			this->btn_disconnect->TabIndex = 0;
			this->btn_disconnect->Text = L"Disconnect";
			this->btn_disconnect->UseVisualStyleBackColor = true;
			this->btn_disconnect->Click += gcnew System::EventHandler(this, &Form1::btn_disconnect_Click);
			// 
			// lbl_RoundTime
			// 
			this->lbl_RoundTime->AutoSize = true;
			this->lbl_RoundTime->Location = System::Drawing::Point(7, 20);
			this->lbl_RoundTime->Name = L"lbl_RoundTime";
			this->lbl_RoundTime->Size = System::Drawing::Size(65, 13);
			this->lbl_RoundTime->TabIndex = 3;
			this->lbl_RoundTime->Text = L"Round Time";
			// 
			// lbl_AITime
			// 
			this->lbl_AITime->AutoSize = true;
			this->lbl_AITime->Location = System::Drawing::Point(7, 2);
			this->lbl_AITime->Name = L"lbl_AITime";
			this->lbl_AITime->Size = System::Drawing::Size(43, 13);
			this->lbl_AITime->TabIndex = 3;
			this->lbl_AITime->Text = L"AI Time";
			// 
			// listViewLog
			// 
			this->listViewLog->Location = System::Drawing::Point(13, 44);
			this->listViewLog->Name = L"listViewLog";
			this->listViewLog->Size = System::Drawing::Size(411, 521);
			this->listViewLog->TabIndex = 2;
			this->listViewLog->UseCompatibleStateImageBehavior = false;
			// 
			// pictureBox_Canvas
			// 
			this->pictureBox_Canvas->Location = System::Drawing::Point(430, 44);
			this->pictureBox_Canvas->Name = L"pictureBox_Canvas";
			this->pictureBox_Canvas->Size = System::Drawing::Size(825, 690);
			this->pictureBox_Canvas->TabIndex = 3;
			this->pictureBox_Canvas->TabStop = false;
			// 
			// panel1
			// 
			this->panel1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->panel1->Controls->Add(this->lbl_RoundTime);
			this->panel1->Controls->Add(this->lbl_ant);
			this->panel1->Controls->Add(this->lbl_AITime);
			this->panel1->Location = System::Drawing::Point(12, 3);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(1243, 38);
			this->panel1->TabIndex = 4;
			// 
			// lbl_ant
			// 
			this->lbl_ant->AutoSize = true;
			this->lbl_ant->Location = System::Drawing::Point(540, 2);
			this->lbl_ant->Name = L"lbl_ant";
			this->lbl_ant->Size = System::Drawing::Size(31, 13);
			this->lbl_ant->TabIndex = 3;
			this->lbl_ant->Text = L"Ants:";
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1260, 737);
			this->Controls->Add(this->panel1);
			this->Controls->Add(this->pictureBox_Canvas);
			this->Controls->Add(this->listViewLog);
			this->Controls->Add(this->panel_Connection);
			this->Name = L"Form1";
			this->Text = L"Form1";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Form1::Form1_FormClosing);
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::Form1_Paint);
			this->panel_Connection->ResumeLayout(false);
			this->panel_Connection->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox_Canvas))->EndInit();
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	_GUIlog ^log;
	TcpNetworkClient ^tcpConnect;
	_Tcp_Logic ^tcpLogic;
	_hlpPaint ^hlpPaint;
	GameStateRenderer ^renderer;
	Version ^version;

	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
			hlpPaint = gcnew _hlpPaint();
			log = _GUIlog::getInstance();
			log->listViewLog = listViewLog;
			log->FileLog = true;
			log->PrepareListViewLog();
			log->EnableCustomPaint();
			log->write("BlackBot Loaded");
			version = gcnew Version(1, 0, 0);
			Text = Text + " version " + version->ToString();
			this->txtBox_Name->Text += "-" + version->ToString();

			renderer = GameStateRenderer::getInstance();
			renderer->canvas = pictureBox_Canvas;
	}

	bool connectToServer(String ^tcpAddress, int port) {
		if (tcpConnect) {
			tcpConnect->disconnectFromServer();
		}

		tcpConnect = gcnew TcpNetworkClient();

		if (!tcpLogic) {
			tcpLogic = gcnew _Tcp_Logic();
			tcpLogic->aiTime = lbl_AITime;
			tcpLogic->roundTime = lbl_RoundTime;
			tcpLogic->antLabel = lbl_ant;
		}

		tcpLogic->AddHandler(tcpConnect);

		if (tcpConnect->connectToServer(tcpAddress, port)) {
			tcpConnect->EnableNologinMode();
			return true;
		}

		return false;
	}

	private: System::Void btn_Connect_Click(System::Object^  sender, System::EventArgs^  e) {
		if (tcpConnect && tcpConnect->connected) {
			log->write("Already connected to server.", Color::Orange);
			return;
		}
		log->write("Connecting..", Color::Green);
		int port = datatypes::String2int(txtBox_Port->Text, 54321);
		if (connectToServer(txtBox_Server->Text, port)) {
			String ^nick = this->txtBox_Name->Text->Replace(" ", "")->Trim();
			tcpConnect->sendMessage("name " + nick);
		}
	}

	private: System::Void Form1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		hlpPaint->FormPaint((Control^)sender, e);
	}

	private: System::Void Form1_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
		if (tcpConnect) {
			tcpConnect->disconnectFromServer();
		}
	}
private: System::Void btn_debugLog_Click(System::Object^  sender, System::EventArgs^  e) {
	if (tcpLogic) tcpLogic->saveDebugData();
}
private: System::Void btn_disconnect_Click(System::Object^  sender, System::EventArgs^  e) {
	if (tcpConnect) {
		tcpLogic->dropHandles();
		tcpConnect->disconnectFromServer();
	}
}
};
}
