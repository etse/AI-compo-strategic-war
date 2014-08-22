#pragma once

using namespace System;
using namespace System::Text;

public ref class datatypes {
public:
	static char *s2s(String^ instr) {
		return (char*)(void*)System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(instr); // ninja String^ -> char *
	}

	static String^ s2s(char *inchar) {
		String ^act = gcnew String(inchar);
		return act;
	}

	static int String2int(String ^text_tall, int default) {
		int tall;

		try {
			tall = Convert::ToInt32(text_tall);
		}
		catch(Exception ^) {
			/*
			_GUIlog ^log = _GUIlog::getInstance();
			log->write("Warning string to integer converter, default to "+default+", error: " + ex->Message);
			*/
			tall = default;
		}
		return tall;
	}
	
	static unsigned int String2uint(String ^text_tall, int default) {
		unsigned int tall;

		try {
			tall = Convert::ToUInt32(text_tall);
		}
		catch(Exception ^) {
			tall = default;
		}
		return tall;
	}
	static unsigned int String2uint(String ^text_tall) {
		unsigned int tall;

		tall = String2uint(text_tall,0);

		return tall;
	}

	/** String to bool conversion .
		convert a system string 0/1+/true/false to a boolean true/false.
	*/
	static bool s2b(String^ value1) {
		if(value1->Length == 0) return false;

		if (value1->Equals("1")) return true;
		if (value1->Equals("0")) return false;

		value1 = value1->Trim()->ToLowerInvariant();

		if (value1->Equals("true")) return true;
		if (value1->Equals("yes")) return true;
		if (value1->Equals("false")) return false;
		if (value1->Equals("no")) return false;
		
		int tall=String2int(value1,0);
		if(tall>0) return true;

		return false;
	}
	static int String2int(String ^text_tall) {
		int tall;

		tall = String2int(text_tall,0);

		return tall;
	}


	static String ^b2s(bool value) {
		if (value) {
			return "1";
		}
		return "0";
	}


	static String ^int2thosandSeperated(__int64 number) {
		using namespace System::Globalization;
		
		String ^humanReadable = String::Format(CultureInfo::CreateSpecificCulture("da-DK"), "{0:N0}", number);

		return humanReadable;
	}

	static String ^byte2String(array<Byte> ^%bytes1) {
		StringBuilder ^stringBuilder = gcnew StringBuilder();
		for each( Byte ^b in bytes1 ) {
			stringBuilder->AppendFormat("{0:x2}", b);
		}

		return stringBuilder->ToString();
	}

	static Point playerPosition(String ^text) {
		array<String ^> ^items = text->Split(',');
		int x;
		int y;

		if(items->Length == 2) {
			x = datatypes::String2int(items[0]);
			y = datatypes::String2int(items[1]);

			return Point(x,y);
		}

		return Point(-1, -1);
	}

};
