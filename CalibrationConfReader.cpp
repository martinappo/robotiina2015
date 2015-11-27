#include "CalibrationConfReader.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>


CalibrationConfReader::CalibrationConfReader()
{
	range = { { 0, 0 }, { 0, 0 }, { 0, 0 } };

}

void CalibrationConfReader::SaveConf(const std::string &name){
	using boost::property_tree::ptree;

	ptree pt;
	pt.put("hue.low", range.hue.low);
	pt.put("hue.high", range.hue.high);
	pt.put("sat.low", range.sat.low);
	pt.put("sat.high", range.sat.high);
	pt.put("val.low", range.val.low);
	pt.put("val.high", range.val.high);


	write_ini(std::string("conf/") + name + ".ini", pt);
}
void CalibrationConfReader::LoadConf(const std::string &name){
	using boost::property_tree::ptree;

	ptree pt;
	try
	{
		read_ini(std::string("conf/") + name + ".ini", pt);

		range.hue.low = (double)pt.get<int>("hue.low") / 179.0;
		range.hue.high = (double)pt.get<int>("hue.high") / 179.0;;
		range.sat.low = (double)pt.get<int>("sat.low") / 255.0;;
		range.sat.high = (double)pt.get<int>("sat.high") / 255.0;;
		range.val.low = (double)pt.get<int>("val.low") / 255.0;;
		range.val.high = (double)pt.get<int>("val.high") / 255.0;;
	}
	catch (...){};

}

HSVColorRange CalibrationConfReader::GetObjectThresholds(int index, const std::string &name)
{
	LoadConf(name);
	return range;
};


CalibrationConfReader::~CalibrationConfReader()
{
}
