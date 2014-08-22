#pragma once

using namespace System::Drawing::Drawing2D;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

public enum class PaintType { GlowAnimated,Border,buttonOverlay };

public ref struct _PaintObject {
	Color glowColor;
	Color ForeColor;
	System::Drawing::Size labelAdjust;
	String ^Text;
	PaintType pType;
	Control ^element;
	Object ^Tag;
	int glowAlpha;
	int AnimationDirection;
	String ^Name;
	int BorderWidth;

	System::Drawing::StringAlignment Alignment;
	System::Drawing::StringAlignment LineAlignment;

private:
	_PaintObject::_PaintObject() {}
public:
	_PaintObject::_PaintObject(PaintType type) {
		this->pType = type;
		AnimationDirection = 1;
		glowAlpha = 20;
		BorderWidth = 1;
		Alignment = StringAlignment::Near;
		LineAlignment = StringAlignment::Center;
	}
};


/**	hlpPain Form gui enhancer.
*	Supports enhancing the GUI forms.
*	Supports alpha.
*/
public ref class _hlpPaint {
protected:
	List<_PaintObject^> ^PaintObjectList;
public:
	array<Color> ^Colors1;		//! Color gradient array.
	float angle;				//! Draw angle.
	InterpolationMode iMode;
	SmoothingMode smooth;
	int borderSize;
	int TitleSize;
	Color CircleColor;
	Brush ^brushTextGlow;
	Color glowColor;
	Control ^animElement;
	Color textColor;
	int glowStrength;
	System::Windows::Forms::Timer ^timer1;
	
	_hlpPaint::_hlpPaint() {
		Colors1 = gcnew array<Color> { Color::FromArgb(45,45,45),Color::FromArgb(60,60,60),Color::FromArgb(82,82,82),Color::FromArgb(180,180,180) };
		//Colors1 = gcnew array<Color> { Color::FromArgb(149,177,220),Color::White,Color::Green };
		angle = 90.0f;
		iMode = InterpolationMode::Bilinear;
		smooth = SmoothingMode::None;
		borderSize = 0;
		TitleSize = 0;
		CircleColor = Color::FromArgb(45,45,45);
		brushTextGlow = gcnew SolidBrush(Color::FromArgb(25, Color::White));
		glowColor = Color::Red;
		textColor = Color::Red;
		glowStrength = 7;
		PaintObjectList = gcnew List<_PaintObject^>;
	}


public: _PaintObject ^AddPainter(Control ^element,PaintType pType) {
		// glow
		_PaintObject ^pObj = gcnew _PaintObject(pType);
		pObj->element = element;
		element->Tag = pObj;
		pObj->Name = element->Name;

		if(pType == PaintType::GlowAnimated) {
			pObj->glowColor = element->ForeColor;
			pObj->ForeColor = element->ForeColor;
			pObj->Text = element->Text;
			pObj->BorderWidth = 2;

			Size size1 = TextRenderer::MeasureText( element->Text,element->Font );
			size1.Width = int(size1.Width * 1.1);
			size1.Height = int(size1.Height *1.2);
			element->MinimumSize = size1;

			element->Text = "";
			enableGlowAnim();

		}

		if(pType == PaintType::Border) {
			pObj->ForeColor = element->ForeColor;
		}

		if(pType == PaintType::buttonOverlay) {	}

		element->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &_hlpPaint::element_glow);

		PaintObjectList->Add(pObj);
		return pObj;
	}

private: System::Void element_glow(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
			 Control ^ctrl = safe_cast<Control^>(sender);
			 _PaintObject ^pObj = safe_cast<_PaintObject^>(ctrl->Tag);
			 
			 if(pObj->pType == PaintType::GlowAnimated) {
				TextGlow(pObj,e);
				return;
			 }

			 if(pObj->pType == PaintType::Border) {
				 PaintBorder(pObj,e);
				 return;
			 }
			 if(pObj->pType == PaintType::buttonOverlay) {
				 paintButtonOverlay(pObj,e);
				 return;
			 }
		 }

		 void paintButtonOverlay(_PaintObject ^pObj, PaintEventArgs^ %e) {
			 // Create solid brush.
			 SolidBrush ^redBrush = gcnew SolidBrush(Color::FromArgb(20,255,255,255));
			 /*
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
			 */
		 }

public:	void enableGlowAnim(void) {
		if(!timer1) {
			timer1 = gcnew System::Windows::Forms::Timer();
		}

		timer1->Tick += gcnew EventHandler(this, &_hlpPaint::TickGlowFunction);
		timer1->Start();
		timer1->Enabled = true;
		timer1->Interval = 85;
	}

	void TickGlowFunction(Object ^sender, EventArgs ^e) {
		
		for each(_PaintObject ^pObj in PaintObjectList) {

			if(pObj->pType == PaintType::GlowAnimated) {
				
				pObj->glowAlpha+=pObj->AnimationDirection;
				if((pObj->glowAlpha>=30) || (pObj->glowAlpha<=10)) {
					pObj->AnimationDirection *=-1;
				}

				pObj->element->Invalidate();
			}
		}
	}

	void setBorder(Form ^FormElement) {
		borderSize = (FormElement->Width-FormElement->ClientSize.Width);
		TitleSize = (FormElement->Height-FormElement->ClientSize.Height);
	}

	void GradientCircle(Control ^FormElement, PaintEventArgs^ %e) {
		int width = FormElement->Width - borderSize;
		int height = FormElement->Height - TitleSize;

		System::Drawing::Rectangle rec1 = System::Drawing::Rectangle(0, 0, width,height);
		System::Drawing::SolidBrush ^brush = gcnew System::Drawing::SolidBrush(CircleColor);
		e->Graphics->FillRectangle(brush, rec1);

		int factor = 200;
		rec1 = System::Drawing::Rectangle(-factor, -factor, factor+width,factor*2+height);
		GraphicsPath ^path = gcnew GraphicsPath();
		path->AddEllipse(rec1);

		// Use the path to construct a brush.
		PathGradientBrush ^pthGrBrush = gcnew PathGradientBrush(path);

		// Set the color at the center of the path to blue.
		pthGrBrush->CenterColor = Color::FromArgb(200, 255, 255, 255);

		// Set the color along the entire boundary, of the path to aqua.
		array<Color> ^colors = { Color::FromArgb(0, 0, 0, 0) };
		pthGrBrush->SurroundColors = colors;

		e->Graphics->FillEllipse(pthGrBrush, rec1);
	}

	void FormPaint(Control ^FormElement, PaintEventArgs^ %e) {
		FormPaint(FormElement,e,Colors1);
	}

	void FormPaint(Control ^FormElement, PaintEventArgs^ %e,array<Color> ^Colors1) {

		if((FormElement->Width==0) || (FormElement->Height == 0)) {
			return;
		}

		int ColorCount = Colors1->Length;	// counting from 0, Eks 6 = 7 colors!

		System::Drawing::Rectangle rec1 = System::Drawing::Rectangle(0, 0, FormElement->Width, FormElement->Height);
		LinearGradientBrush ^brush = gcnew LinearGradientBrush(rec1,Color::Black,Color::Black,angle,false);

		ColorBlend ^cb = gcnew ColorBlend(ColorCount);

		for(int i=0;i<=(ColorCount-1);i++) {
			
			float f = (float) i/(ColorCount-1);

			cb->Positions[i]=f;
		}

		cb->Colors = Colors1;

		brush->InterpolationColors=cb;
		e->Graphics->SmoothingMode = smooth;
		e->Graphics->InterpolationMode = iMode;
		e->Graphics->FillRectangle(brush, rec1);
	}

	void PaintTest1(Control ^FormElement, PaintEventArgs^ %e) {

		array<Point> ^quadPoints = gcnew array<Point> {  Point(0, 0),
			Point(0, FormElement->Height),
			Point(FormElement->Height, FormElement->Width),
			Point(FormElement->Height, 0)
		};

		System::Drawing::Rectangle rec1 = System::Drawing::Rectangle(0, 0, FormElement->Width, FormElement->Height);
		PathGradientBrush ^quadBrush = gcnew PathGradientBrush(quadPoints);
		quadBrush->CenterColor = Color::WhiteSmoke;
		quadBrush->SurroundColors = gcnew array<Color> {
			Color::Red,				// top left
			Color::AntiqueWhite,	// bottom left
			Color::Blue,			// bottom right
			Color::AntiqueWhite		// top right
		};
		e->Graphics->FillRectangle(quadBrush, rec1);
	}

	/** Paint a border around a element.
	*/
	void PaintBorder(Control ^FormElement, PaintEventArgs^ %e) {

		if((FormElement->Width==0) || (FormElement->Height == 0)) return;

		System::Drawing::Rectangle rec1 = System::Drawing::Rectangle(1, 1, FormElement->Width-2, FormElement->Height-2);
		System::Drawing::Pen ^pen1 = gcnew System::Drawing::Pen(Color::Black,2.0f);

		e->Graphics->DrawRectangle(pen1,rec1);
	}

	void PaintBorder(_PaintObject ^pObj, PaintEventArgs^ %e) {

		if((pObj->element->Width==0) || (pObj->element->Height == 0)) return;

		System::Drawing::Rectangle rec1 = System::Drawing::Rectangle(pObj->BorderWidth/2, pObj->BorderWidth/2, pObj->element->Width-pObj->BorderWidth, pObj->element->Height-pObj->BorderWidth);
		System::Drawing::Pen ^pen1 = gcnew System::Drawing::Pen(pObj->ForeColor,(float)pObj->BorderWidth);

		e->Graphics->DrawRectangle(pen1,rec1);
	}


	/** Custom onPaint text glow.
	*/
	void TextGlow(Control ^FormElement, PaintEventArgs^ %e) {
		// Reference: http://www.codeproject.com/Articles/42529/Outline-Text

		const int BlurAmt = 6;

		e->Graphics->Clear(Color::Transparent);
		
		SolidBrush ^BackColor1 = gcnew SolidBrush(Color::Transparent);
		e->Graphics->FillRectangle(BackColor1, 0, 0, FormElement->Width, FormElement->Height);
		

		e->Graphics->SmoothingMode = SmoothingMode::HighQuality;
		e->Graphics->InterpolationMode = InterpolationMode::HighQualityBilinear;
		e->Graphics->TextRenderingHint = System::Drawing::Text::TextRenderingHint::AntiAliasGridFit;

		for (int x = 0; x <= BlurAmt; x++) {
			for (int y = 0; y <= BlurAmt; y++) {
				e->Graphics->DrawString(FormElement->Text, FormElement->Font, brushTextGlow, Point(x, y));
				/*
				e->Graphics->DrawString(FormElement->Text, FormElement->Font, brushTextGlow, Point(-x, -y));
				e->Graphics->DrawString(FormElement->Text, FormElement->Font, brushTextGlow, Point(x, -y));
				e->Graphics->DrawString(FormElement->Text, FormElement->Font, brushTextGlow, Point(-x, y));
				*/
			}
		}

		Brush ^brushText = gcnew SolidBrush(FormElement->ForeColor);
		//e->Graphics->DrawString(FormElement->Text, FormElement->Font, brushText, Point(BlurAmt /2, BlurAmt/2));
	}

	void TextGlow(_PaintObject ^pObj, PaintEventArgs^ %e) {
		e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;
		//e->Graphics->InterpolationMode = InterpolationMode::HighQualityBicubic;
		e->Graphics->InterpolationMode = InterpolationMode::HighQualityBilinear;
		e->Graphics->TextRenderingHint = System::Drawing::Text::TextRenderingHint::AntiAliasGridFit;

		StringFormat ^strformat = gcnew StringFormat();
		strformat->Alignment = pObj->Alignment;
		strformat->LineAlignment = pObj->LineAlignment;

		float emSize = e->Graphics->DpiY * pObj->element->Font->Size / 72;

		GraphicsPath ^path = gcnew GraphicsPath();
		path->AddString(pObj->Text,
			pObj->element->Font->FontFamily, 
			(int)FontStyle::Regular,
			emSize,
			pObj->element->ClientRectangle,
			strformat);

		for(int i=1; i<glowStrength; ++i) {
			Pen ^pen = gcnew Pen(Color::FromArgb(pObj->glowAlpha,pObj->glowColor),(float) i*2);
			pen->LineJoin = LineJoin::Round;
			e->Graphics->DrawPath(pen, path);
		}

		if(pObj->BorderWidth > 0) {
			Pen ^penBorder = gcnew Pen(Color::Black,(float) pObj->BorderWidth);
			penBorder->LineJoin = LineJoin::Round;
			e->Graphics->DrawPath(penBorder, path);
		}

		SolidBrush ^brush = gcnew SolidBrush(pObj->ForeColor);
		e->Graphics->FillPath(brush, path);
	}

};
