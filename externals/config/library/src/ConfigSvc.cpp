//
// Created by brachwal on 23.03.2020.
//

#include <iostream>
#include "ConfigSvc.hh"
#include "colors.hh"

///////////////////////////////////////////////////////////////////////////////
///
ConfigSvc* ConfigSvc::GetInstance() {
    static ConfigSvc instance;
    return &instance;
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::LOGIC_ERROR(const std::string& caller, const std::string& module, const std::string& message){
    throw std::logic_error(caller+": Module( \""+module+"\"): "+message);
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::ARGUMENT_ERROR(const std::string& caller, const std::string& module, const std::string& message){
    throw std::invalid_argument(caller+": Module( \""+module+"\"): "+message);
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::NOT_REGISTERED_MODULE_ERROR(const std::string& caller, const std::string& module){
    throw std::invalid_argument(caller+": Module \""+module+"\": is not registered.");
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::WARNING(const std::string& caller, const std::string& module, const std::string& message){
    ConfigSvc::WARNING(caller+": Module \""+module+"\": "+message);
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::WARNING(const std::string& message){
    std::cout << "[ConfigSvc]"<< FYEL("[WARNING]")<<"::"<< message << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::ERROR(const std::string& message){
    std::cout << "[ConfigSvc]"<< FRED("[ERROR]")<<"::"<< message << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::INFO(const std::string& message){
    std::cout << "[ConfigSvc]"<< FGRN("[INFO]")<<":: "<< message << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
///
bool ConfigSvc::IsRegistered(const std::string& module) const {
    return m_config_modules.find(module) != m_config_modules.end() ? true : false;
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::SetValue(const std::string& module, const std::string& unit, std::any value, bool is_deafult) {
    if (!IsRegistered(module))
        ConfigSvc::NOT_REGISTERED_MODULE_ERROR("ConfigSvc::SetValue",module);
    m_config_modules.at(module)->thisConfig()->SetValue(unit, value, is_deafult);
}

////////////////////////////////////////////////////////////////////////////////
///
bool ConfigSvc::GetStatus(const std::string& module) const {
    if (!IsRegistered(module))
        ConfigSvc::NOT_REGISTERED_MODULE_ERROR("ConfigSvc::GetStatus",module);
    return m_config_modules.at(module)->thisConfig()->GetStatus();    
}

////////////////////////////////////////////////////////////////////////////////
///
bool ConfigSvc::GetStatus(const std::string& module, const std::string& unit) const {
    if (!IsRegistered(module))
        ConfigSvc::NOT_REGISTERED_MODULE_ERROR("ConfigSvc::GetStatus",module);
    return m_config_modules.at(module)->thisConfig()->GetStatus(unit);
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::Register(const std::string& module, std::shared_ptr<Configurable> config_volume) {
    if (IsRegistered(module)) {
        // Each module name should be unique
        ConfigSvc::LOGIC_ERROR("ConfigSvc::Register",module," this module is already registered.");
    } else {
        ConfigSvc::INFO("Register new module : \""+module+"\"");
        m_config_modules.insert(make_pair(module, config_volume));
    }
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::Unregister(const std::string& module) {
    // Erasing By Key
    m_config_modules.erase(module);
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::SetDefault(const std::string& module, const std::string& unit) {
    if (!IsRegistered(module))
        ConfigSvc::NOT_REGISTERED_MODULE_ERROR("ConfigSvc::GetStatus",module);
    m_config_modules.at(module)->DefaultConfig(unit);
}

////////////////////////////////////////////////////////////////////////////////
///
std::shared_ptr<ConfigModule> ConfigSvc::GetConfigModule(const std::string& module){
    if (!IsRegistered(module))
        ConfigSvc::NOT_REGISTERED_MODULE_ERROR("ConfigSvc::GetStatus",module);
    return m_config_modules.at(module)->thisConfig();
}

////////////////////////////////////////////////////////////////////////////////
///
toml::parse_result& ConfigSvc::ParseTomlFile(const std::string& file){
    m_toml_file = file;
    m_toml_config = toml::parse_file(file);
    m_toml = true;
    if(!m_config_modules.empty())
        ReloadConfiguration();
    return m_toml_config;
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::PrintTomlConfig() const{
    if(m_toml){
        ConfigSvc::INFO("PrintTomlConfig...");
        //std::cout << m_toml_config << "\n";
        std::cout << toml::json_formatter{ m_toml_config } << "\n"; // re-serialize as JSON
    }
    else {
        std::cout << "[ConfigSvc]"<<FYEL("[WARNING]")<<":: PrintTomlConfig : any file was parsed.."<< std::endl;
    }
}

////////////////////////////////////////////////////////////////////////////////
///
void ConfigSvc::ReloadConfiguration()const{
    // std::cout << " ReloadConfiguration debug... " << std::endl;

    for(auto& module : m_config_modules){
        auto name = module.second->thisConfig()->GetName();
        if(IsTomlParsed(name)){
            ConfigSvc::INFO("Reload configuration for \""+name+"\"...");
            module.second->thisConfig()->SetTomlConfig();
            module.second->DefaultConfig();
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
///
bool ConfigSvc::IsTomlParsed(const std::string& module) const { 
    if (module.empty()) // generic check for any module
        return m_toml; 
    if (m_toml_config.find(module)!=m_toml_config.end() )
        return true;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
///
bool ConfigSvc::ValidateConfiguration(const std::string& module) const{
    std::string module_name = "None";
    bool valid = false;
    auto print_status = [](const std::string module_name, bool is_valid){
        if(is_valid) ConfigSvc::INFO(module_name+":: module validation... SUCCESS");
        else         ConfigSvc::ERROR(module_name+":: module validation... FAILED");
    };

    if(module.empty()){
        for(const auto& module : m_config_modules){
            module_name = module.second->thisConfig()->GetName();
            valid = module.second->ValidateConfig();
            print_status(module_name,valid);

        }
    } else {
        module_name = module;
        valid = m_config_modules.at(module_name)->ValidateConfig();
        print_status(module_name,valid);

    }
    if(!valid){
            std::string msg = "Not valid configuration!";
            ConfigSvc::ARGUMENT_ERROR("ValidateConfig", module_name,msg);
        }
    return valid;
}
