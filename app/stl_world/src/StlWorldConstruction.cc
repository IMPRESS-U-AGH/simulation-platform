#include "StlWorldConstruction.hh"
#include "Services.hh"
#include "G4Box.hh"
#include "Types.hh"

StlWorldConstruction::StlWorldConstruction(){}

    ///
StlWorldConstruction::~StlWorldConstruction(){}

WorldConstruction* StlWorldConstruction::GetInstance() {
    static auto instance = new StlWorldConstruction(); // It's being released by G4GeometryManager
    return instance;
}

bool StlWorldConstruction::Create() {
    
     // create the world box
    auto worldSize = configSvc()->GetValue<G4ThreeVector>("WorldConstruction", "WorldSize");
    auto Air = configSvc()->GetValue<G4MaterialSPtr>("MaterialsSvc", "Usr_G4AIR20C");
    auto worldB = new G4Box("worldG", worldSize.getX(), worldSize.getY(), worldSize.getZ());
    auto worldLV = new G4LogicalVolume(worldB, Air.get(), "worldLV", 0, 0, 0);
    auto isocentre = thisConfig()->GetValue<G4ThreeVector>("Isocentre");
    SetPhysicalVolume(new G4PVPlacement(0, isocentre, "worldPV", worldLV, 0, false, 0));
    auto world_pv = GetPhysicalVolume();

    ConstructWorldModules(world_pv);
    ConstructAssembledModel(world_pv);
    return true;
}

void StlWorldConstruction::ConstructAssembledModel(G4VPhysicalVolume *parentPV) {


    auto config = toml::parse_file(configFile);


    if (config[configObj].as_table()->find("Element")!= config[configObj].as_table()->end()){
        auto numberOfElements = config[configObj]["Element"].as_array()->size();
        for( int i = 0; i < numberOfElements; i++ ){
            auto name = config[configObj]["Element"][i][0];
            auto path = config[configObj]["Element"][i][1];
            auto material = config[configObj]["Element"][i][2];
            m_elements.push_back(new Element(parentPV, name, path, material));
            }
}
}
void StlWorldConstruction::ConstructSDandField() {
    WorldConstruction::ConstructSDandField();
    for(auto& tray : m_trays){
    tray->DefineSensitiveDetector();
    }
}

std::vector<VPatient*> StlWorldConstruction::GetCustomDetectors() const {
    std::vector<VPatient*> customDetectors;
    for(const auto& tray : m_trays){
        customDetectors.push_back(tray->GetDetector());
    }
    return std::move(customDetectors);
}
