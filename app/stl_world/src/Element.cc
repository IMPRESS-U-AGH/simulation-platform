#include "Element.hh"
#include "G4SystemOfUnits.hh"
#include "GeoSvc.hh"
#include "G4ProductionCuts.hh"
#include "ConfigSvc.hh"
#include "ElementSD.hh"
#include "G4UserLimits.hh"
#include "NTupleEventAnalisys.hh"
#include "RunAnalysis.hh"
#include "colors.hh"
#include <vector>
#include "Services.hh"
#include "CADMesh.hh"

namespace {
    G4Mutex ElementMutex = G4MUTEX_INITIALIZER;
}


Element::Element(G4VPhysicalVolume *parentPV, const std::string& name, const std::string& path, 
const std::string& material, const G4ThreeVector& centre, const int& elementID)
: VPatient(name),  m_element_name(name), m_element_path(path), m_element_material(material), m_elementID(elementID) {
    m_centre = centre;
    Construct(parentPV);
} 



////////////////////////////////////////////////////////////////////////////////
///
void Element::Construct(G4VPhysicalVolume *parentPV) {

    std::string path = m_element_path;
    std::string name = GetName();
    auto nameLV = name + "LV";
    auto namePV = name + "PV";

    std::cout << "Element: " << name << " path: " << path << std::endl;

    auto mesh = CADMesh::TessellatedMesh::FromSTL(path);
    G4VSolid* solid = mesh->GetSolid();
    
    auto Medium = ConfigSvc::GetInstance()->GetValue<G4MaterialSPtr>("MaterialsSvc", m_element_material);
    auto elementLV = new G4LogicalVolume(solid, Medium.get(), nameLV);

    SetPhysicalVolume(new G4PVPlacement(nullptr, m_centre, namePV, elementLV, parentPV, false, 0));

    auto regVol = new G4Region(name+"_G4RegionCuts");
    auto cuts = new G4ProductionCuts;
    cuts->SetProductionCut(0.1 * mm);
    regVol->SetProductionCuts(cuts);
    elementLV->SetRegion(regVol);
    regVol->AddRootLogicalVolume(elementLV);

}

////////////////////////////////////////////////////////////////////////////////
///
void Element::DefineSensitiveDetector(){
  G4AutoLock lock(&ElementMutex);
  if(m_patientSD.Get()==0){
    auto pv = GetPhysicalVolume();
    auto centre = m_centre;

    auto envBox = dynamic_cast<G4VSolid*>(pv->GetLogicalVolume()->GetSolid());
    auto label = GetName();
    m_patientSD.Put(new ElementSD(label+"_SD",centre,m_elementID));
    auto patientSD = m_patientSD.Get();
    patientSD->SetTracksAnalysis(m_tracks_analysis);

    G4String hcName;
    hcName = label+"_HC";

    std::string name = GetName();
    std::string runCollName = name.substr(0, name.find('_', 0));
    // patientSD->AddScoringVolume(runCollName,hcName,*envBox,m_elementID,m_elementID,m_elementID);
    // VPatient::SetSensitiveDetector(label+"LV", patientSD); 

  }
}
