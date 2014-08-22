#pragma once

using namespace System;

public ref class _Version {
	protected:
		int revision;
	public:
		int major;
		int minor;
		int build;

		_Version::_Version(){
			major = 0;
			minor = 0;
			build = 0;
			
			String ^vsVersion = System::Reflection::Assembly::GetExecutingAssembly()->GetName()->Version->ToString();
			revision = Convert::ToUInt32(vsVersion->Split('.')[3]);
		}

		_Version::_Version(String ^txt){
			DecodeString(txt);
		}

		virtual String ^ToString(void) override {
			return text;
		}
		property String^ text {
			String^ get()  { 
				return major +"."+minor+"."+build;
			}
		}

		void _Version::DecodeString(String ^txt) {
			array<String^>^ items;

			if (txt->Contains(".")) {
				items = txt->Split('.');

				if (items->Length > 1) {
					major = Convert::ToUInt32(items[0]);
					if (items->Length > 2) {
						minor = Convert::ToUInt32(items[1]);
						if (items->Length >= 3) {
							build = Convert::ToUInt32(items[2]);
						}
					}
				}
			} else {
				try {
					major = Convert::ToUInt32(txt);
				}
				catch(Exception ^ex) {
					major = 0;
					return;
					ex->ToString();
				}
			}
		}

		bool _Version::GreaterThan(_Version ^%xver) {
			if (major > xver->major) {
				return true;
			} else if ( (major >= xver->major) && (minor > xver->minor)) {
				return true;
			} else if ((major >= xver->major) && (minor >= xver->minor) && (build > xver->build)) {
				return true;
			}

			return false;
		}

		bool _Version::GreaterOrEqual(_Version ^%xver) {
			if(Equal(xver)) {
				return true;
			}
			if(GreaterThan(xver)) {
				return true;
			}

			return false;
		}

		bool _Version::Equal(_Version ^%xver) {
			if ((major == xver->major) && (minor == xver->minor) && (build == xver->build)) {
				return true;
			}

			return false;
		}


};

  