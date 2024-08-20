
#include "Element.hh"
#include "Services.hh"


////////////////////////////////////////////////////////////////////////////////
///
Element::Element(G4VPhysicalVolume *parentPV, const std::string& name, const std::string& path, const std::string& material )
:IPhysicalVolume(name), TomlConfigModule(name), m_element_name(name), m_element_path(path), m_element_material(material) {

    LoadConfiguration();
    Construct(parentPV);
} 

////////////////////////////////////////////////////////////////////////////////
///
void Element::Construct(G4VPhysicalVolume *parentPV) {

  ParseTomlConfig() ;
}

////////////////////////////////////////////////////////////////////////////////
///
void Element::DefineSensitiveDetector() {

}

////////////////////////////////////////////////////////////////////////////////
///
void Element::LoadConfiguration(){

    // Deafult configuration
    m_rot = G4RotationMatrix(); //.rotateY(180.*deg);
    
    m_global_centre = G4ThreeVector(0.0,0.0,0.0);

    // Any config value is being replaced by the one existing in TOML config 
    ParseTomlConfig();

}

////////////////////////////////////////////////////////////////////////////////
///
void Element::ParseTomlConfig(){
    SetTomlConfigFile(); // it set the job main file for searching this configuration
    auto configFile = GetTomlConfigFile();
    if (!svc::checkIfFileExist(configFile)) {
        LOGSVC_CRITICAL("Element::TConfigurarable::ParseTomlConfig::File {} not fount.", configFile);
        exit(1);
    }
    auto config = toml::parse_file(configFile);
    auto configPrefix = GetTomlConfigPrefix();
    LOGSVC_INFO("Element::Importing configuration from: {}:{}",configFile,configPrefix);

    m_global_centre.setX(config[configPrefix]["Position"][0].value_or(0.0));
    m_global_centre.setY(config[configPrefix]["Position"][1].value_or(0.0));
    m_global_centre.setZ(config[configPrefix]["Position"][2].value_or(0.0));


}

