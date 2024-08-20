
#include "Element.hh"
#include "Services.hh"
#include "G4Box.hh"

////////////////////////////////////////////////////////////////////////////////
///
Element::Element(G4VPhysicalVolume *parentPV, const std::string& name, const std::string& path, const std::string& material )
:IPhysicalVolume(name), TomlConfigModule(name), m_element_name(name), m_element_path(path), m_element_metrial(material) {

    LoadConfiguration();
    Construct(parentPV);
} 

////////////////////////////////////////////////////////////////////////////////
///
void Element::Construct(G4VPhysicalVolume *parentPV) {
    auto medium = Service<ConfigSvc>()->GetValue<G4MaterialSPtr>("MaterialsSvc", "Usr_G4AIR20C");
    
    auto boxName = m_tray_name + "EnvBox";
    G4Box *patientEnv = new G4Box(m_tray_name, m_tray_world_halfSize.x(), m_tray_world_halfSize.y(), m_tray_world_halfSize.z());
    
    auto LVName = m_tray_name + "EnvLV";
    G4LogicalVolume *patientEnvLV = new G4LogicalVolume(patientEnv, medium.get(), LVName, 0, 0, 0);
    
    auto PVName = m_tray_name + "EnvPV";
    auto pv = new G4PVPlacement(&m_rot, m_global_centre, PVName, patientEnvLV, parentPV, false, 0);


    m_detector->Construct(pv);
    m_detector->WriteInfo();
}

////////////////////////////////////////////////////////////////////////////////
///
void Element::DefineSensitiveDetector() {
    m_detector->DefineSensitiveDetector();
}

////////////////////////////////////////////////////////////////////////////////
///
void Element::LoadConfiguration(){

    // Deafult configuration
    m_rot = G4RotationMatrix(); //.rotateY(180.*deg);
    // m_tray_world_halfSize = G4ThreeVector(102.,111.,9.2);
    
    m_global_centre = G4ThreeVector(0.0,0.0,0.0);

    m_det_config.m_top_position_in_env = G4ThreeVector(0.0,0.0,0.0);


    m_det_config.m_mrow_shift = false;
    m_det_config.m_mlayer_shift = false;

    // m_det_config.m_cell_medium = "RMPS470";
    // m_det_config.m_stl_geometry_file_path = "example_element.stl";

    m_det_config.m_top_position_in_env = G4ThreeVector(0.,0.0,0.0); // "centre" of the trey is not in the geometrical centre

    // Any config value is being replaced by the one existing in TOML config 
    ParseTomlConfig();

    // Finally, inject the configuration to the detector instance
    dynamic_cast<D3DDetector*>(m_detector)->SetConfig(m_det_config);
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
    // G4cout << "global_centre: " << m_global_centre << G4endl;

    auto vox_nX = config[configPrefix]["CellVoxelization"][0].value_or(0);
    if(vox_nX > 0 ) 
        m_det_config.m_cell_nX_voxels = vox_nX;
    auto vox_nY = config[configPrefix]["CellVoxelization"][1].value_or(0);
    if(vox_nY > 0 ) 
        m_det_config.m_cell_nY_voxels = vox_nY;
    auto vox_nZ = config[configPrefix]["CellVoxelization"][2].value_or(0);
    if(vox_nZ > 0 ) 
        m_det_config.m_cell_nZ_voxels = vox_nZ;

    auto rotX = config[configPrefix]["Rotation"][0].value_or(0);
    if(rotX > 0 ) 
        m_rot.rotateX(rotX*deg);
    auto rotY = config[configPrefix]["Rotation"][1].value_or(0);
    if(rotY > 0 ) 
        m_rot.rotateY(rotY*deg);
    auto rotZ = config[configPrefix]["Rotation"][2].value_or(0);
    if(rotZ > 0 )           
        m_rot.rotateZ(rotZ*deg);


}

