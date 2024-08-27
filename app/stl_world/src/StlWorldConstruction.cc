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
    auto configSvc = Service<ConfigSvc>(); 
    auto configFile = configSvc->GetTomlFile();
    auto config = toml::parse_file(configFile);


    if (config["Parts"].as_table()->find("elements")!= config["Parts"].as_table()->end()){
        auto numberOfElements = config["Parts"]["elements"].as_array()->size();
        for( int i = 0; i < numberOfElements; i++ ){
            auto name = config["Parts"]["elements"][i][0].value_or(std::string());
            auto path = config["Parts"]["elements"][i][4].value_or(std::string());
            auto material = config["Parts"]["elements"][i][3].value_or(std::string());
            double centreX = config["Parts"]["elements"][i][1][0].value_or(0.0);
            double centreY = config["Parts"]["elements"][i][1][1].value_or(0.0);
            double centreZ = config["Parts"]["elements"][i][1][2].value_or(0.0);
            G4ThreeVector centre =  G4ThreeVector(centreX,centreY,centreZ);
            m_elements.push_back(new Element(parentPV, name, path, material, centre, i));
            }
}
}
