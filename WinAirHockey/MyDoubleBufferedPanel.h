#pragma once

using namespace System;
using namespace System::Windows::Forms;

public ref class MyDoubleBufferedPanel : public Panel {
public:
	MyDoubleBufferedPanel() : Panel() {
		this->DoubleBuffered = true;
	};
};
