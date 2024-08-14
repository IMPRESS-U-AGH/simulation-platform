#ifndef TB_StlWorldConstruction_HH
#define TB_StlWorldConstruction_HH

#include "WorldConstruction.hh"
#include "D3DTray.hh"



class StlWorldConstruction: public WorldConstruction {
public:
  static WorldConstruction *GetInstance();

bool Create() override;

void ConstructTrayDetectors(G4VPhysicalVolume *parentPV);

void ConstructSDandField() override;

std::vector<VPatient*> GetCustomDetectors() const override;

private:
    ///
    StlWorldConstruction();

    ///
    ~StlWorldConstruction();

    /// Delete the copy and move constructors
    StlWorldConstruction(const StlWorldConstruction &) = delete;
    StlWorldConstruction &operator=(const StlWorldConstruction &) = delete;
    StlWorldConstruction(StlWorldConstruction &&) = delete;
    StlWorldConstruction &operator=(StlWorldConstruction &&) = delete;

    ///
    std::vector<D3DTray*> m_trays;

    ///
    G4VPhysicalVolume* m_bunker_wall = nullptr;
    G4VPhysicalVolume* m_bunker_inside_pv = nullptr;

};

#endif