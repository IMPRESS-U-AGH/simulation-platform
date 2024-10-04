#include "NTupleEventSTLAnalisys.hh"
#include "VPatient.hh"
#include "G4Event.hh"
#include "G4Run.hh"
#include "G4SDManager.hh"
#include "G4AnalysisManager.hh"
#include "VoxelHit.hh"
#include "G4Threading.hh"
#include "Services.hh"
#include <algorithm>
#include <vector>




void NTupleEventSTLAnalisys::CreateNTuple(const TTreeCollection& treeColl){
  auto treeName = treeColl.m_name+m_treeNamePostfix;
  auto treeDescription = treeColl.m_description;
  auto analysisManager =  G4AnalysisManager::Instance();
  m_ntuple_collection.Insert(treeName,TTreeEventCollection());
  auto& evtNTupleColl = m_ntuple_collection.Get(treeName);
  auto ntupleId = analysisManager->CreateNtuple(treeName,treeDescription);
  evtNTupleColl.m_ntupleId = ntupleId;


  auto createNtupleIColumn = [&](const char* name){
    evtNTupleColl.m_colId[name] = analysisManager->CreateNtupleIColumn(ntupleId, name); 
  };
  auto createNtupleDColumn = [&](const char* name){
    evtNTupleColl.m_colId[name] = analysisManager->CreateNtupleDColumn(ntupleId, name); 
  };

  auto createNtupleVecIColumn = [&](const char* name,std::vector<G4int>& vec){
    evtNTupleColl.m_colId[name] = analysisManager->CreateNtupleIColumn(ntupleId, name, vec); 
  };

  auto createNtupleVecDColumn = [&](const char* name,std::vector<G4double>& vec){
    evtNTupleColl.m_colId[name] = analysisManager->CreateNtupleDColumn(ntupleId, name, vec); 
  };
  
  // General TTree branches
  createNtupleIColumn("ThreadId");
  createNtupleIColumn("G4EvtId");
  createNtupleIColumn("G4RunId");
  createNtupleDColumn("G4EvtGlobalTime");
  createNtupleVecDColumn("G4EvtPrimaryE",evtNTupleColl.m_G4EvtPrimaryEnergy);
  createNtupleIColumn("G4EvtPrimaryN");
  createNtupleDColumn("VolumeEDeposit");
  createNtupleDColumn("VolumeMeanEDeposit");
  createNtupleDColumn("VolumeDose");
  createNtupleVecIColumn("TrkId",evtNTupleColl.m_TrkId);
  createNtupleVecIColumn("TrkTypeId",evtNTupleColl.m_TrkTypeId);
  createNtupleVecDColumn("TrkE",evtNTupleColl.m_TrkEnergy);
  createNtupleVecDColumn("TrkTheta",evtNTupleColl.m_TrkTheta);
  createNtupleVecDColumn("TrkLength",evtNTupleColl.m_TrkLength);
  createNtupleVecDColumn("TrkPositionX",evtNTupleColl.m_TrkPositionX);
  createNtupleVecDColumn("TrkPositionY",evtNTupleColl.m_TrkPositionY);
  createNtupleVecDColumn("TrkPositionZ",evtNTupleColl.m_TrkPositionZ);

  analysisManager->FinishNtuple(ntupleId);
}
