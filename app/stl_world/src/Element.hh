#ifndef Dose3D_STLCONSTRUCTION_HH
#define Dose3D_STLCONSTRUCTION_HH

#include "IPhysicalVolume.hh"
#include "TomlConfigurable.hh"
#include "Services.hh"
///\class PatientGeometry
///\brief The liniac Phantom volume construction.
class Element : public IPhysicalVolume, public TomlConfigModule {
    private:
        ///
        void ParseTomlConfig() override;

        ///
        void LoadConfiguration();

    public:
    ///
    Element(G4VPhysicalVolume *parentPV, const std::string& name, const std::string& path, const std::string& material);

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


    VPatient* GetDetector() const { return m_detector; }

    G4ThreeVector m_global_centre;
    std::string m_element_name;
    std::string m_element_path;
    std::string m_element_material;
    std::string m_elemen_config_file;
    G4RotationMatrix m_rot;

    ///
    VPatient* m_detector;

};


#endif //Dose3D_STLCONSTRUCTION_HH