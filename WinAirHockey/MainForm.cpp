#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

int ball_pos_x = 0;
int ball_pos_y = 0;
int ball_x_inc = 2;
int ball_y_inc = -2;

[STAThreadAttribute]
void Main(array<String ^> ^args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	WinAirHockey::MainForm form;
	Application::Run(%form);
}
