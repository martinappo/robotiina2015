#include "ManualControl.h"


ManualControl::ManualControl() :ConfigurableModule("ManualControl")
{

	AddSetting("Move Left", []{return "a"; }, [this] {this->wheels->Drive(40, 90, 0); });
	AddSetting("Move Right", []{return "d"; }, [this]{this->wheels->Drive(40, -90, 0); });
	AddSetting("Move Forward", []{return "w"; }, [this]{this->wheels->Drive(190, 0, 0); });
	AddSetting("Move Back", []{return "s"; }, [this]{this->wheels->Drive(-40, 0, 0); });
//	AddSetting("Rotate Right", []{return ""; }, [this]{this->wheels->Rotate(0, 20); });
//	AddSetting("Rotate Left", []{return ""; }, [this]{this->wheels->Rotate(1, 20); });
	AddSetting("Kick", []{return " "; }, [this] {this->coilBoard->Kick(); });
	AddSetting("Start tribbler", []{return "z"; }, [this]{this->coilBoard->ToggleTribbler(true); });
	AddSetting("Stop tribbler", []{return "x"; }, [this]{this->coilBoard->ToggleTribbler(false); });

}


ManualControl::~ManualControl()
{
}
