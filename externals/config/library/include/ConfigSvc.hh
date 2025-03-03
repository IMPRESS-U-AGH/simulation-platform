//
// Created by brachwal on 23.03.2020.
//

#ifndef CONFIGSVC_CONFIGSVC_HH
#define CONFIGSVC_CONFIGSVC_HH

#include <cstdio>
#include <memory>
#include "Configurable.hh"
#include "toml.hh"

////////////////////////////////////////////////////////////////////////////////
///
///\class ConfigSvc
///\brief The main configuration service to manage the desired system configurable.
/// It is a singleton like pattern
// TODO:: Make std::ostream flexibility
class ConfigSvc {
    private:
        ///
        ConfigSvc() = default;
        
        ///
        ~ConfigSvc() = default;
    
        // Delete the copy and move constructors
        ConfigSvc(const ConfigSvc&) = delete;
        ConfigSvc& operator=(const ConfigSvc&) = delete;
        ConfigSvc(ConfigSvc&&) = delete;
        ConfigSvc& operator=(ConfigSvc&&) = delete;

        ///\brief Module name and pointer to actual module mapping.
        std::map<std::string, std::shared_ptr<Configurable>> m_config_modules;

        ///
        std::string m_toml_file;
        toml::parse_result m_toml_config;

        ///
        bool m_toml = false;

        ///
        static void NOT_REGISTERED_MODULE_ERROR(const std::string& caller,const std::string& module);

        ///
        void ReloadConfiguration() const;
    
    public:
        ///\brief Static method to get instance of this singleton object.
        static ConfigSvc* GetInstance();

        ///
        static void LOGIC_ERROR(const std::string& caller, const std::string& module, const std::string& message);

        ///
        static void ARGUMENT_ERROR(const std::string& caller, const std::string& module, const std::string& message);

        ///
        static void WARNING(const std::string& caller, const std::string& module, const std::string& message);

        ///
        static void WARNING(const std::string& message);

        ///
        static void ERROR(const std::string& message);
        
        ///
        static void INFO(const std::string& message);

        ///\brief The main comunication method for changing the actual value of the particular unit of a given module.
        void SetValue(const std::string& module, const std::string& unit, std::any value, bool is_deafult=true);

        ///\brief The main communication method for getting the actual value of the particular unit of a given module.
        template <typename T> T GetValue(const std::string& module, const std::string& unit, const char* caller = __builtin_FUNCTION()) const;

        ///\brief The wrapper of the Configuration::GetStatus
        bool GetStatus(const std::string& module) const;

        ///\brief The wrapper of the Configuration::GetStatus
        bool GetStatus(const std::string& module, const std::string& unit) const;

        ///\brief Register new module configuraiton in the main service.
        void Register(const std::string& module, std::shared_ptr<Configurable>);

        ///\brief Check if the specified module is registered already in the service.
        bool IsRegistered(const std::string& module) const;

        ///\brief Remove from the service given module configuration.
        void Unregister(const std::string& module);

        ///\brief Function setting to defaults
        ///for a given module and its unit for which the value is defined
        void SetDefault(const std::string& module, const std::string& unit);

        ///
        std::shared_ptr<ConfigModule> GetConfigModule(const std::string& module);

        ///
        toml::parse_result& ParseTomlFile(const std::string& file);

        ///
        void PrintTomlConfig() const;

        ///
        bool IsTomlParsed(const std::string& module=std::string()) const;

        ///
        toml::parse_result* GetTomlConfig() { return &m_toml_config; }
        std::string GetTomlFile() { return m_toml_file; }

        ///
        template <typename T> T GetTomlValue(const std::string& module, const std::string& unit, T&& none) const;

        ///\brief Run across all registered modules and validate their config trough Configurable::ValidateConfig() method
        bool ValidateConfiguration(const std::string& module=std::string()) const;
};

template <typename T> T ConfigSvc::GetValue(const std::string& module, const std::string& unit, const char* caller) const {
    if (!IsRegistered(module)){
        auto callStack = static_cast<std::string>(caller)+"() -> ConfigSvc::GetValue";
        ConfigSvc::ARGUMENT_ERROR(callStack,module,"is not defined.");
    }
    return m_config_modules.at(module)->thisConfig()->GetValue<T>(unit);
}

template <typename T> T ConfigSvc::GetTomlValue(const std::string& module, const std::string& unit, T&& none ) const {
    if(m_toml){
        return m_toml_config[module][unit].value_or<T>(std::move(none));
    }
    return none;
}
#endif //CONFIGSVC_CONFIGSVC_HH
