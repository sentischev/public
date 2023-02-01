#pragma once

using namespace System;
using namespace System::Windows::Forms;

public ref class MyDoubleBufferedPanel : public System::Windows::Forms::Panel {
public:
	MyDoubleBufferedPanel() : System::Windows::Forms::Panel() {
		this->DoubleBuffered = true;
	};
};