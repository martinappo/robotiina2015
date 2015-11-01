#include "ConfigurableModule.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using boost::property_tree::ptree;


ConfigurableModule::ConfigurableModule(std::string sModuleName) : m_sModuleName(sModuleName)
{
}


ConfigurableModule::~ConfigurableModule()
{
}

void ConfigurableModule::AddSetting(const std::string& name, std::function<std::string()> const &get_func, std::function<void()> const &set_func){
	m_settings.insert(std::make_pair(name, std::make_tuple(get_func, set_func)));
}


void ConfigurableModule::LoadSettings() {
	ptree pt;
	try {
		read_ini("conf/"+m_sModuleName+".ini", pt);
		for (auto setting : m_settings) {
			auto val = pt.get<std::string>(setting.first);
			while (val != std::get<0>(setting.second)()) { 
				std::get<1>(setting.second)();
			}
		}
	}
	catch (...){
		SaveSettings();
	}

}

void ConfigurableModule::SaveSettings() {
	ptree pt;
	for (auto setting : m_settings) {
		pt.put(setting.first, std::get<0>(setting.second)());
	}
	write_ini("conf/" + m_sModuleName + ".ini", pt);

}
