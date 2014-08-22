#pragma once

//#include "windows.h"

#define MaxLogLines 100 // Lines before rotating GUI log.
#define BUF_ProgInfo 5

using namespace System;
using namespace System::IO;
using namespace System::Drawing;
using namespace System::Drawing::Drawing2D;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;

public ref struct _LogElement {
	String ^text;
	String ^time;
	String ^date;
	System::Drawing::Color color;
};

public ref class _GUIlog {
    protected:
		bool enabledCustomPaint;
		static bool instanceFlag;
		static _GUIlog ^single;
		delegate void WriteEvent(String^ textmessage, Color farge, Color bakgrunn);
		delegate void VoidEvent(void);
    private:
		int LogLinesRotate;		// log wrap.
		System::String ^LastLine;
		bool reversed;

		[DllImport( "kernel32.dll",
			EntryPoint = "AttachConsole",  
			SetLastError = true,  
			CharSet = CharSet::Auto,  
			CallingConvention = CallingConvention::StdCall)]
		static bool AttachConsole( int dwProcessId );

		[DllImport("kernel32.dll",  
			EntryPoint = "FreeConsole",  
			SetLastError = true,  
			CharSet = CharSet::Auto,  
			CallingConvention = CallingConvention::StdCall)]
		static bool FreeConsole();

    public:                                 // public part
		String ^LogFile;
		String ^ErrorLogFile;
        bool SilentLog;						// no output..
		bool FileLog;
		bool ErrorFileLog;
		bool DebugPrint;
		bool ConsolePrint;
		System::Drawing::Color DefaultTextColor;
		System::Drawing::Color DefaultBackground;
		System::Windows::Forms::ListView^ listViewLog;	// pointer for form GUI list.
		System::Collections::Generic::List<_LogElement^> ^dump;
		String ^ircFile;		// needed for irc support api call to windrop/egg
		String ^ircPrefix;
		unsigned int lineCounter;
		array<Color> ^ColumnsBackgroundColors;
		Color ColumnForeColor;
		int WriteMaxCallCurrent;		// reset in code on demand.
				
	    _GUIlog::_GUIlog () {               // Constructor.
			dump = gcnew System::Collections::Generic::List<_LogElement^>;    
			SilentLog = false;
			FileLog = false;
			ErrorFileLog = false;
			DebugPrint = true;
			ConsolePrint = false;
			LogLinesRotate = MaxLogLines;
			LogFile = Application::StartupPath + "\\"+Application::ProductName+".log";
			ErrorLogFile = Application::StartupPath + "\\"+Application::ProductName+"_Error.log";

			DefaultTextColor = System::Drawing::Color::Black;
			DefaultBackground = System::Drawing::Color::White;
			reversed = false;
			lineCounter = 0;
			// irc part.
			ircFile = Application::StartupPath + "\\..\\windrop\\socket.log";
			ircPrefix=Application::ProductName;

			ColumnForeColor = Color::White;
			ColumnsBackgroundColors = gcnew array<Color> { Color::FromArgb(82,82,82),Color::FromArgb(60,60,60),Color::FromArgb(82,82,82) };
			enabledCustomPaint = false;
			WriteMaxCallCurrent=0;
        }
		
		// initialize.
		void _GUIlog::begin(ListView ^%listPoint) {
			listViewLog = listPoint;

			PrepareListViewLog();
		}

		void EnableCustomPaint(void) {
			if(!enabledCustomPaint) {
				listViewLog->OwnerDraw = true;
				listViewLog->DrawItem += gcnew DrawListViewItemEventHandler(this, &_GUIlog::listView1_DrawItem);
				//listViewLog->DrawSubItem += gcnew DrawListViewSubItemEventHandler(this, &_GUIlog::View1_DrawSubItem);
				listViewLog->DrawColumnHeader += gcnew DrawListViewColumnHeaderEventHandler(this, &_GUIlog::listView1_DrawColumnHeader);
			}
			enabledCustomPaint = true;
		}

private: void View1_DrawSubItem(Object ^sender, DrawListViewSubItemEventArgs ^e) {
			 e->Graphics->FillRectangle(Brushes::Black, e->Bounds);
			 //e->DrawDefault = true;
		 }
private: void listView1_DrawColumnHeader(Object ^sender, DrawListViewColumnHeaderEventArgs ^e) {
			LinearGradientBrush ^Lbrush = gcnew LinearGradientBrush(e->Bounds,Color::Black,Color::Black,90,false);
			ColorBlend ^cBlend1 = gcnew ColorBlend(ColumnsBackgroundColors->Length);

			for(int i=0;i<=(ColumnsBackgroundColors->Length-1);i++) {
				float f = (float) i/(ColumnsBackgroundColors->Length-1);
				cBlend1->Positions[i]=f;
			}

			cBlend1->Colors = ColumnsBackgroundColors;
			Lbrush->InterpolationColors=cBlend1;
			e->Graphics->SmoothingMode = SmoothingMode::None;
			e->Graphics->InterpolationMode = InterpolationMode::Bilinear;
			e->Graphics->FillRectangle(Lbrush, e->Bounds);

			StringFormat ^strformat = gcnew StringFormat();
			strformat->Alignment = StringAlignment::Near;
			strformat->LineAlignment = StringAlignment::Center;

			SolidBrush ^brush = gcnew SolidBrush(ColumnForeColor);
			System::Drawing::Rectangle textBound = e->Bounds;
			textBound.X += 5;
			e->Graphics->DrawString(e->Header->Text,e->Font,brush,textBound,strformat);

			if(e->Bounds.X > 0) {
				Point p1 = Point(e->Bounds.X,e->Bounds.Y);
				Point p2 = Point(e->Bounds.X,e->Bounds.Height);

				Pen ^pen = gcnew Pen(Color::FromArgb(50,Color::White),3.0f);
				e->Graphics->DrawLine(pen,p1,p2);
				e->Graphics->DrawLine(Pens::White,p1,p2);
			}


			// Create solid brush.
			SolidBrush ^redBrush = gcnew SolidBrush(Color::FromArgb(20,255,255,255));
			
			// Create array of points for curve.
			int factor = 20;
			Point point1 = Point(e->Bounds.Width-factor+e->Bounds.X,0);
			Point point2 = Point(e->Bounds.Width+e->Bounds.X + factor, -factor );
			Point point3 = Point(e->Bounds.Width+e->Bounds.X, e->Bounds.Height );
			Point point4 = Point(e->Bounds.Width-factor+e->Bounds.X, e->Bounds.Height + factor );
			Point point5 = Point(e->Bounds.Width-factor+factor/4+e->Bounds.X, e->Bounds.Height/2 );
			
			array<Point> ^points = {point1, point2, point3, point4, point5 };

			// Set fill mode.
			FillMode newFillMode = FillMode::Winding;

			// Set tension. 
			float tension = -0.4f;

			// Fill curve on screen.
			e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;
			e->Graphics->FillClosedCurve(redBrush, points, newFillMode, tension);

		}
private: void listView1_DrawItem(Object ^sender, DrawListViewItemEventArgs ^e) {
			e->DrawDefault = true;
		}
private: void View1_Resize(Object ^sender, EventArgs ^e) {
			 if(this->listViewLog->Columns->Count>1) {
				this->listViewLog->AutoResizeColumn(this->listViewLog->Columns->Count-1,ColumnHeaderAutoResizeStyle::HeaderSize);
			 }
		 }
private: void View1_ResizeColumn(Object ^sender, ColumnWidthChangedEventArgs ^e) {
			  this->listViewLog->Invalidate();
		 }
private: void View1_ResizeColumn(Object ^sender, ColumnWidthChangingEventArgs ^e) {
			 this->listViewLog->Invalidate();
		 }

public:
		_GUIlog::~_GUIlog() {
			instanceFlag = false;
		}

		void _GUIlog::EnableConsolePrint() {
			#ifndef ATTACH_PARENT_PROCESS
				#define ATTACH_PARENT_PROCESS -1
			#endif
			AttachConsole( ATTACH_PARENT_PROCESS );
			ConsolePrint = true;
		}

		void _GUIlog::DisableConsolePrint() {
			FreeConsole();
			ConsolePrint = false;
		}

		void setReversed(bool reversed){
			this->reversed = reversed;
		}

		property System::String^ GetLastLogLine {
			System::String^ get() { return LastLine; }
		}

		static _GUIlog ^getInstance(void) {
			if(! instanceFlag) {
				single = gcnew _GUIlog();
				instanceFlag = true;
				return single;
			} else {
				return single;
			}
		}
		
		static void msgbox(System::String^ text) {
			#ifndef ServiceCode
				if (text != "") {
					System::Windows::Forms::MessageBox::Show(text);
				}
			#endif
		}

		void configureRightClick(void) {
			static bool loaded = false;

			if(!loaded) {
				System::ComponentModel::IContainer^ components = gcnew System::ComponentModel::Container();
				ContextMenuStrip^ contextMenuStrip1 = gcnew System::Windows::Forms::ContextMenuStrip(components);
				ToolStripMenuItem^ copyToClipboardToolStripMenuItem = gcnew ToolStripMenuItem();
				ToolStripMenuItem^ ClearToolStripMenuItem = gcnew ToolStripMenuItem();

				// Items
				copyToClipboardToolStripMenuItem->Name = L"copyToClipboardToolStripMenuItem";
				copyToClipboardToolStripMenuItem->Size = System::Drawing::Size(169, 22);
				copyToClipboardToolStripMenuItem->Text = L"Copy to clipboard";
				copyToClipboardToolStripMenuItem->Click += gcnew System::EventHandler(this, &_GUIlog::copyToClipboardToolStripMenuItem_Click);	// needed, but why? :) -> to call function!

				ClearToolStripMenuItem->Name = L"ClearToolStripMenuItem";
				ClearToolStripMenuItem->Size = System::Drawing::Size(169, 22);
				ClearToolStripMenuItem->Text = L"Clear Log";
				ClearToolStripMenuItem->Click += gcnew System::EventHandler(this, &_GUIlog::ClearToolStripMenuItem_Click);	// needed, but why? :) -> to call function!

				// Menu
				contextMenuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {copyToClipboardToolStripMenuItem,ClearToolStripMenuItem });
				contextMenuStrip1->Name = L"contextMenuStrip1";
				contextMenuStrip1->Size = System::Drawing::Size(170, 48);
				contextMenuStrip1->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &_GUIlog::contextMenuStrip1_Opening);





				this->listViewLog->ContextMenuStrip = contextMenuStrip1;
				

				loaded = true;
			}
		}

		void PrepareListViewLog() {
			if (listViewLog->InvokeRequired) {
				listViewLog->BeginInvoke(gcnew VoidEvent(this, &_GUIlog::PrepareListViewLog));
				return;
			}
			int sizeTime, sizeEntry;
			sizeTime = 55;
			sizeEntry = listViewLog->Width - sizeTime -4;

			DefaultBackground = listViewLog->BackColor;
			DefaultTextColor = listViewLog->ForeColor;

			// listview config.
			this->listViewLog->Clear();
			this->listViewLog->View = System::Windows::Forms::View::Details;
			this->listViewLog->GridLines = true;
			this->listViewLog->FullRowSelect = true;
						
			configureRightClick();


			this->listViewLog->Columns->Add("Time", sizeTime, System::Windows::Forms::HorizontalAlignment::Left);
			this->listViewLog->Columns->Add("Entry", sizeEntry, System::Windows::Forms::HorizontalAlignment::Left);
			
			
			this->listViewLog->Resize += gcnew EventHandler(this, &_GUIlog::View1_Resize);
			//this->listViewLog->ColumnWidthChanged += gcnew ColumnWidthChangedEventHandler(this, &_GUIlog::View1_ResizeColumn);
			this->listViewLog->ColumnWidthChanging += gcnew ColumnWidthChangingEventHandler(this, &_GUIlog::View1_ResizeColumn);

			// tooltip
			this->listViewLog->ShowItemToolTips = true;
		}

		private: System::Void contextMenuStrip1_Opening(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e) {}

		private: System::Void copyToClipboardToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {

					 if(listViewLog->SelectedItems->Count > 0) {

						 ListView::SelectedListViewItemCollection^ sItem = this->listViewLog->SelectedItems;
						 if(sItem->Count == 0) return;

						 String ^text = "";
						 for each(ListViewItem^ item in sItem) {							 
							 for each(ListViewItem::ListViewSubItem ^sub in item->SubItems) {
								 text += sub->Text + " ";
							 }
							 text = text->Trim();
							 text += "\r\n";
						 }

						 CopytoClipboard(text);
					 }
			}
		private: System::Void ClearToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {

						if(listViewLog->SelectedItems->Count > 0) {
							listViewLog->Items->Clear();
						}
					}
		
		public:

		void CopytoClipboard(String ^text) {
			try {
				if(text->Length > 0) {
					Clipboard::SetDataObject(text,true);
				}
			}
			catch(Exception ^ex) {
				write("Error writing to clipboard: " + ex->Message, Color::Red);
				ex->Message;
			}
		}

		static __int64 GetFileSize(String^ file) {
			FileInfo^ pFileProps = gcnew FileInfo(file);

			return pFileProps->Length;
		}

		void write(int val) {
			write(val, DefaultTextColor);
		}

		void write(int val, System::Drawing::Color Color) {
			System::String ^tmp;
			tmp = val.ToString();
			write(tmp,Color,DefaultBackground);
		}

		void write(System::String^ textmessage) {
			write(textmessage,DefaultTextColor,DefaultBackground);
		}
		
		void write(System::String^ textmessage,System::Drawing::Color farge) {
			write(textmessage,farge, DefaultBackground);
			
		}

		void write(System::String^ textmessage, System::Drawing::Color farge, System::Drawing::Color bakgrunn) {
#ifndef ServiceCode
			if (listViewLog) {
				if (this->listViewLog->InvokeRequired) {
					listViewLog->BeginInvoke(gcnew WriteEvent(this, &_GUIlog::write), textmessage, farge, bakgrunn);
					return;
				}
			} else {
				//No log set.
				return;
			}
//				unsigned static int LogLines=0;
			static int counter = 0;
			System::DateTime Now = System::DateTime::Now;
			System::String ^TXTnow = Now.ToString("HH:mm:ss");
			String ^dato = Now.ToString("dd/MM/yyyy");
			ListViewItem^ listItem1;
			
			if(!SilentLog) {
				if(listViewLog) {
					if(listViewLog->Items->Count > LogLinesRotate) {
						if(reversed) {
							listViewLog->Items->RemoveAt(0);
						} else {
							try {
								listViewLog->Items->RemoveAt(listViewLog->Items->Count - 1);
							}
							catch (Exception ^ex) {
								write("exception" + ex->Message);
								return;
							}

						}
					}

					try {
						listItem1 = gcnew System::Windows::Forms::ListViewItem(TXTnow);
					}
					catch (Exception ^ex) {
						write("exception" + ex->Message);
						return;
					}
					
					if (textmessage->Length > 0) {
						// List view.
						LastLine = textmessage;		// save for debugging purposes.
						listItem1->SubItems->Add( textmessage );
						listItem1->BackColor=bakgrunn;
						listItem1->ForeColor=farge;
						if(reversed){
							listViewLog->Items->Add(listItem1);
						} else {
							listViewLog->Items->Insert(0, listItem1);
						}
					}
				}

				if (counter>=1) {
					counter = 0;

					System::Windows::Forms::Application::DoEvents();        // Refresher GUI
				}
					
				if(DebugPrint) {
					System::Diagnostics::Debug::Print(TXTnow + " :: " + textmessage);
				}

				if(ConsolePrint) {
					Console::WriteLine(TXTnow + " :: " + textmessage);
				}
					
				counter++;
			}
			#endif

			if (FileLog) {
				lineCounter++;
				WriteLogFile(LogFile, dato, TXTnow, textmessage);
			}
			
		}

		bool writeMax(String ^text,int max, System::Drawing::Color farge) {
			
			if(WriteMaxCallCurrent <max) {
				write(text, farge);
				WriteMaxCallCurrent++;
				return true;
			}

			if(WriteMaxCallCurrent == max) {
				write("Max loglines reached....... muting output.");
				WriteMaxCallCurrent++;
			}

			return false;
		}
		
		void errorWrite(String ^textMessage) {
			errorWrite(textMessage,false);
		}
		void errorWrite(String ^textMessage, bool critical) {
			if(critical) {
				write(textMessage, Color::Red, Color::Black);
			} else {
				write(textMessage, Color::Red, DefaultBackground);
			}

			System::DateTime Now = System::DateTime::Now;
			System::String ^TXTnow = Now.ToString("HH:mm:ss");
			String ^dato = Now.ToString("dd/MM/yyyy");
			if (ErrorFileLog) {
				WriteLogFile(ErrorLogFile, dato, TXTnow, textMessage);
			}
		}

		void WriteLogFile(String ^logFilePath, String ^date, String ^txtNow, String ^textMessage) {
			bool NewLog=false;
			const int maxSize=10*1024*1024;	// 10M

			if (File::Exists(logFilePath)) {
				__int64 size=GetFileSize(logFilePath);
				if (size > maxSize) {
					File::Delete(logFilePath);
					NewLog=true;
				}
			} else {
				NewLog=true;
			}

			try {
				StreamWriter^ pwriter = gcnew StreamWriter(logFilePath,true);
				if (NewLog) {
					pwriter->WriteLine("["+date+"::"+txtNow +"] LogFile Created/Rotated, rotate at: " +Convert::ToString(maxSize));
					pwriter->WriteLine("-------------------------------------------------------------------------------------------");
				}
				pwriter->WriteLine(lineCounter + " - ["+date+"::"+txtNow +"] "+textMessage);
				pwriter->Flush();
				pwriter->Close();
			}
			catch (Exception ^Ex) {
				FileLog=false;
				write("Error opening log file: " + Ex->Message);
			}
		}

		System::Windows::Forms::ListView^ getListView() {
			return listViewLog;
		}

};
 

