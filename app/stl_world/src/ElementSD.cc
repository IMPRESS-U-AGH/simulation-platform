#include "ElementSD.hh"
#include "Element.hh"
#include "StepAnalysis.hh"
#include <G4VProcess.hh>
#include "Services.hh"
#include "PatientTrackInfo.hh"
#include "PrimaryParticleInfo.hh"

////////////////////////////////////////////////////////////////////////////////
///
ElementSD::ElementSD(const G4String& sdName, const G4ThreeVector& centre, G4int idX)
:VPatientSD(sdName,centre){
  m_id_x = idX;
  m_id_y = idX;
  m_id_z = idX;
}

////////////////////////////////////////////////////////////////////////////////
///
G4bool ElementSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {

  auto theTouchable = dynamic_cast<const G4TouchableHistory *>(aStep->GetPreStepPoint()->GetTouchable());
  auto volumeName = theTouchable->GetVolume()->GetName();  
  auto aTrack = aStep->GetTrack();
  auto trackInfo = aTrack->GetUserInformation();
  if(trackInfo){
    dynamic_cast<PatientTrackInfo*>(trackInfo)->FillInfo(aStep);
  }
  else{
    trackInfo = new PatientTrackInfo(); // it's being cleaned up by kernel when track ends it's life
    dynamic_cast<PatientTrackInfo*>(trackInfo)->FillInfo(aStep);
    aTrack->SetUserInformation(trackInfo);
  }


////////////////////////////////////////////////////////////////////////////////
///
  auto thisSdHCNames = GetScoringVolumeNames(); 
  for(const auto& hcName : ControlPoint::GetHitCollectionNames()){
    if(find(thisSdHCNames.begin(), thisSdHCNames.end(), hcName) != thisSdHCNames.end())
      ProcessHitsCollection(hcName,aStep); 
  }
  return true;
}