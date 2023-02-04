#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

int ball_pos_x = 30;
int ball_pos_y = 100;
int ball_x_inc = 10;
int ball_y_inc = -10;

int user1_bat_pos = 100;
int user1_bat_inc = 10;

[STAThreadAttribute]
void Main(array<String ^> ^args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	WinAirHockey::MainForm form;
	Application::Run(%form);
}
