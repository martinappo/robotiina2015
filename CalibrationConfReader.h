#pragma once
#include "types.h"

class CalibrationConfReader
{
protected:
	HSVColorRange range/* =  {{0,179},{0,255},{0,255}}*/;
	void LoadConf(const std::string &name);
	void SaveConf(const std::string &name);
public:
	CalibrationConfReader();
	virtual HSVColorRange GetObjectThresholds(int index, const std::string &name);
	~CalibrationConfReader();
};

