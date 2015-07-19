#pragma once
#include "types.h"
class ConfigurableModule :
	public IConfigurableModule
{
public:
	ConfigurableModule(std::string sModuleName);
	virtual ~ConfigurableModule();
	 SettingsList &GetSettings() { return m_settings; };
	 std::string GetDebugInfo() { return std::string(""); }
private:
	SettingsList m_settings;
	std::string m_sModuleName;
protected:
	void AddSetting(const std::string& name, std::function<std::string()> const &, std::function<void()> const &);
	void LoadSettings();
	void SaveSettings();

};

#define ADD_BOOL_SETTING(setting) AddSetting(#setting, [this](){return this->setting ? "yes" : "no"; }, [this](){this->setting = !this->setting; SaveSettings();});

