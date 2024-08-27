/**
*
* \author B. Rachwal (brachwal [at] agh.edu.pl)
* \date 10.09.2021
*
*/

#ifndef STL_ELEMENT_SD_HH
#define STL_ELEMENT_SD_HH

#include "VPatientSD.hh"

class ElementSD : public VPatientSD {

  public:
    ///
    ElementSD(const G4String& sdName, const G4ThreeVector& centre, G4int id);

    ///
    ~ElementSD() = default;

    ///
    G4bool ProcessHits(G4Step*, G4TouchableHistory*) override;

};

#endif //STL_ELEMENT_SD_HH
