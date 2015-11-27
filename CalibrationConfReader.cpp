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

		range.hue.low =  pt.get<int>("hue.low");
		range.hue.high = pt.get<int>("hue.high");
		range.sat.low = pt.get<int>("sat.low");
		range.sat.high = pt.get<int>("sat.high");
		range.val.low = pt.get<int>("val.low");
		range.val.high = pt.get<int>("val.high");

		range.hue.flow = (double)range.hue.low / 179;
		range.hue.fhigh = (double)range.hue.high / 179;
		range.sat.flow = (double)range.sat.low / 255;
		range.sat.fhigh = (double)range.sat.high / 255;
		range.val.flow = (double)range.val.low / 255;
		range.val.fhigh = (double)range.val.high / 255;
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
