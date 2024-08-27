#ifndef STLCONSTRUCTION_HH
#define STLCONSTRUCTION_HH

#include "VPatient.hh"
#include "Services.hh"

class Element : public VPatient {
    public:
    ///
    Element(G4VPhysicalVolume *parentPV, const std::string& name, const std::string& path, const std::string& material, const G4ThreeVector& centre, const int& elementID);

    ///
    ~Element() {};

    ///
    void Construct(G4VPhysicalVolume *parentPV) override;

    ///
    void Destroy() override {}

    ///
    G4bool Update() override { return true;}

    ///
    void Reset() override {}

    ///
    void WriteInfo() override {}
    
    ///
    void DefineSensitiveDetector();

    void ParseTomlConfig() override {}

    G4ThreeVector m_centre;
    std::string m_element_name;
    std::string m_element_path;
    std::string m_element_material;
    int m_elementID;
    G4RotationMatrix m_rot;

};


#endif //STLCONSTRUCTION_HH