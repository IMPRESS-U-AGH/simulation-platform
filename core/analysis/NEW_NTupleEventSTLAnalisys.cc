//
// Created by brachwal on 30.05.2020.
//

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


std::vector<NTupleEventSTLAnalisys::TTreeCollection> NTupleEventSTLAnalisys::m_ttree_collection = std::vector<TTreeCollection>();

////////////////////////////////////////////////////////////////////////////////
///
NTupleEventSTLAnalisys *NTupleEventSTLAnalisys::GetInstance() {
  static NTupleEventSTLAnalisys instance = NTupleEventSTLAnalisys();
  return &instance;
}

////////////////////////////////////////////////////////////////////////////////
///
void NTupleEventSTLAnalisys::DefineTTree(const G4String& treeName, bool cellVoxelisation, const G4String& hcName, const G4String& treeDescription){
  if (Service<ConfigSvc>()->GetValue<bool>("RunSvc", "NTupleAnalysis") == false)
    return;

  if(hcName.empty()){
    LOGSVC_INFO("Defining TTree: {}",treeName);
    m_ttree_collection.emplace_back(NTupleEventSTLAnalisys::TTreeCollection());
    m_ttree_collection.back().m_name = treeName;
    m_ttree_collection.back().m_hc_names.emplace_back(treeName);
    m_ttree_collection.back().m_description = treeDescription;
  }
  else {
    G4int treeIdx = -1;
    G4bool treeExists = false;
    for(const auto& tree : m_ttree_collection){
      if (tree.m_name==treeName){
        treeExists = true;
        ++treeIdx;
        break;
      }
      else 
        ++treeIdx;
    }
    if (!treeExists){
      LOGSVC_INFO("Defining TTree: {}",treeName);
      m_ttree_collection.emplace_back(NTupleEventSTLAnalisys::TTreeCollection());
      m_ttree_collection.back().m_name = treeName;
      m_ttree_collection.back().m_description = treeDescription;
      m_ttree_collection.back().m_hc_names.emplace_back(hcName);
    } else {
      m_ttree_collection.at(treeIdx).m_hc_names.emplace_back(hcName);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void NTupleEventSTLAnalisys::BeginOfRun(const G4Run* runPtr, G4bool isMaster){
  if (Service<ConfigSvc>()->GetValue<bool>("RunSvc", "NTupleAnalysis") == false)
    return;
  m_runId = runPtr->GetRunID();
  auto runSvc = Service<RunSvc>();
  auto control_point = runSvc->CurrentControlPoint();
  m_degree_rotation = control_point->GetDegreeRotation();
  for(const auto& tree : NTupleEventSTLAnalisys::m_ttree_collection){
    if(GetNTupleId(tree.m_name+m_treeNamePostfix) == -1){ 
      CreateNTuple(tree);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
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

////////////////////////////////////////////////////////////////////////////////
///
G4int NTupleEventSTLAnalisys::GetNTupleId(const G4String& treeName){
  auto exists = m_ntuple_collection.Has(treeName);
  if(exists){
    auto evtCollection = m_ntuple_collection.Get(treeName);
    return evtCollection.m_ntupleId;
  }
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
///
void NTupleEventSTLAnalisys::FillEventCollection(const G4String& treeName, const G4Event *evt, VoxelHitsCollection* hitsColl){
  auto isoToSim = Service<ConfigSvc>()->GetValue<G4ThreeVector>("WorldConstruction", "IsoToSimTransformation");
  auto analysisManager = G4AnalysisManager::Instance();
  auto ntplId = GetNTupleId(treeName);
  if (ntplId==-1){
    G4cout << " [WARNING]::FillEventCollection:: given TTree ("<< treeName <<") collection doesn't exists! " << G4endl;
    return;
  }
  auto& evtColl = m_ntuple_collection.Get(treeName); 
  int nHits = hitsColl->entries();
  if(nHits==0){
    return; 
  }
  evtColl.m_evtId = evt->GetEventID();
  G4double totalEnergyDeposit = 0;
  G4double totalDose = 0;

  std::vector<G4double> hits_evt_global_time;
  for (int i=0;i<nHits;i++){ 

    auto hit = dynamic_cast<VoxelHit*>(hitsColl->GetHit(i));
    auto hitEDeposit = hit->GetEnergyDeposit() / keV;
    auto hitMeanEDeposit = hit->GetMeanEnergyDeposit() / keV;



    auto volumeDose = hit->GetDose();
    evtColl.m_CellIDose.emplace_back( cellDose );




    totalEnergyDeposit+=hitEDeposit;
    if(!evtColl.m_minimalistic_ttree){
      evtColl.m_VolumeHitEDeposit.emplace_back(hitEDeposit);
      evtColl.m_VolumeHitMeanEDeposit.emplace_back(hitMeanEDeposit);
    }
    evtColl.m_VolumeHitDose.emplace_back( volumeDose );

    if(!evtColl.m_minimalistic_ttree){
      auto evtPE = hit->GetEvtPrimariesEnergy();
      evtColl.m_G4EvtPrimaryEnergy.assign(evtPE.begin(),evtPE.end());
      evtColl.m_EvtPrimariesN = hit->GetEvtNPrimaries();
      hits_evt_global_time.emplace_back( hit->GetGlobalTime() );
    }


    if (evtColl.m_tracks_analysis){
      std::vector<G4int> trkType;
      std::vector<G4int> trkTypeId;
      for(const auto& iTrkType : hit->GetTrkType()){
        trkType.emplace_back(iTrkType.first);
        trkTypeId.emplace_back(iTrkType.second);
      }
      evtColl.m_HitsTrkId.emplace_back(trkType);
      evtColl.m_HitsTrkTypeId.emplace_back(trkTypeId);

      std::vector<G4double> trkEnergy;
      for(const auto& iTrkE : hit->GetTrkEnergy()){
        trkEnergy.emplace_back(iTrkE.second / MeV);
      }
      evtColl.m_HitsTrkEnergy.emplace_back(trkEnergy);

      std::vector<G4double> trkTheta;
      for(const auto& iTrkTheta : hit->GetTrkTheta()){
        trkTheta.emplace_back(iTrkTheta.second);
      }
      evtColl.m_HitsTrkTheta.emplace_back(trkTheta);

      std::vector<G4double> trkPosX;
      std::vector<G4double> trkPosY;
      std::vector<G4double> trkPosZ;
      for(const auto& iTrkPos : hit->GetTrkPosition()){
        trkPosX.emplace_back(iTrkPos.second.getX()-isoToSim.x());
        trkPosY.emplace_back(iTrkPos.second.getY()-isoToSim.y());
        trkPosZ.emplace_back(iTrkPos.second.getZ()-isoToSim.z());
      }
      evtColl.m_HitsTrkPosX.emplace_back(trkPosX);
      evtColl.m_HitsTrkPosY.emplace_back(trkPosY);
      evtColl.m_HitsTrkPosZ.emplace_back(trkPosZ);

      std::vector<G4double> trkLength;
      for(const auto& iTrkLength : hit->GetTrkLength()){
        trkLength.emplace_back(iTrkLength.second);
      }
      evtColl.m_HitsTrkLength.emplace_back(trkLength);
    }

  }
  if(!evtColl.m_minimalistic_ttree){
    evtColl.m_global_time = *std::max_element(hits_evt_global_time.begin(), hits_evt_global_time.end());
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void NTupleEventSTLAnalisys::FillNTupleEvent(){
  auto analysisManager = G4AnalysisManager::Instance();
  
  for(auto coll = m_ntuple_collection.Begin(); coll != m_ntuple_collection.End(); ++coll){
    auto minimalistic_ttree = coll->second.m_minimalistic_ttree;
    auto nHits = coll->second.m_CellIdX.size();
    if(nHits > 0){
      auto& treeEvtColl = coll->second;
      auto ntupleId = treeEvtColl.m_ntupleId;

      auto fillNtupleIColumn = [&](const char* name, G4int val){
          analysisManager->FillNtupleIColumn(ntupleId,treeEvtColl.m_colId[name], val);
      };

      auto fillNtupleDColumn = [&](const char* name, G4double val){
        analysisManager->FillNtupleDColumn(ntupleId,treeEvtColl.m_colId[name], val);
      };

      for (int i=0;i<nHits;++i){

          fillNtupleIColumn("ThreadId",G4Threading::G4GetThreadId());
          fillNtupleIColumn("G4EvtId",treeEvtColl.m_evtId);
          fillNtupleDColumn("G4EvtGlobalTime",treeEvtColl.m_global_time);
          fillNtupleIColumn("G4EvtPrimaryN",treeEvtColl.m_EvtPrimariesN);
          fillNtupleIColumn("G4RunId",m_runId);
          fillNtupleDColumn("PositionX",treeEvtColl.m_VoxelPositionX.at(i));
          fillNtupleDColumn("PositionY",treeEvtColl.m_VoxelPositionY.at(i));
          fillNtupleDColumn("PositionZ",treeEvtColl.m_VoxelPositionZ.at(i));
          fillNtupleDColumn("VolumeEDeposit",treeEvtColl.m_VoxelHitEDeposit.at(i));
          fillNtupleDColumn("VolumeMeanEDeposit",treeEvtColl.m_VoxelHitMeanEDeposit.at(i));
          fillNtupleDColumn("VolumeDose",treeEvtColl.m_VoxelHitDose.at(i));
          treeEvtColl.m_TrkId.clear();
          treeEvtColl.m_TrkId = treeEvtColl.m_VoxelHitsTrkId.at(i);
          treeEvtColl.m_TrkTypeId.clear();
          treeEvtColl.m_TrkTypeId = treeEvtColl.m_VoxelHitsTrkTypeId.at(i);
          treeEvtColl.m_TrkEnergy.clear();
          treeEvtColl.m_TrkEnergy = treeEvtColl.m_VoxelHitsTrkEnergy.at(i);
          treeEvtColl.m_TrkTheta.clear();
          treeEvtColl.m_TrkTheta = treeEvtColl.m_VoxelHitsTrkTheta.at(i);
          treeEvtColl.m_TrkLength.clear();
          treeEvtColl.m_TrkLength = treeEvtColl.m_VoxelHitsTrkLength.at(i);
          treeEvtColl.m_TrkPositionX.clear();
          treeEvtColl.m_TrkPositionX = treeEvtColl.m_VoxelHitsTrkPosX.at(i);
          treeEvtColl.m_TrkPositionY.clear();
          treeEvtColl.m_TrkPositionY = treeEvtColl.m_VoxelHitsTrkPosY.at(i);
          treeEvtColl.m_TrkPositionZ.clear();
          treeEvtColl.m_TrkPositionZ = treeEvtColl.m_VoxelHitsTrkPosZ.at(i);
          ///////////////////////////////////////////////////////////////////////
          analysisManager->AddNtupleRow(ntupleId);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void NTupleEventSTLAnalisys::EndOfEventAction(const G4Event *evt){
  auto hCofThisEvent = evt->GetHCofThisEvent();
  if(hCofThisEvent){
    auto nColl = hCofThisEvent->GetNumberOfCollections();
    ClearEventCollections();
  for(const auto& tree : NTupleEventSTLAnalisys::m_ttree_collection){
    for(const auto& hc : tree.m_hc_names){
      auto collection_id = G4SDManager::GetSDMpointer()->GetCollectionID(hc);
      if(collection_id<0)
          G4cout<< "[ERROR]:: NTupleEventSTLAnalisys::EndOfEventAction TTree: " << tree.m_name+m_treeNamePostfix << "G4SDManager err: " << collection_id  << G4endl;
      else{
        auto hitsColl = dynamic_cast<VoxelHitsCollection*>(hCofThisEvent->GetHC(collection_id));
        FillEventCollection(tree.m_name+m_treeNamePostfix,evt,hitsColl);
      }
    }
  }
  FillNTupleEvent();
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void NTupleEventSTLAnalisys::ClearEventCollections(){
  for(auto coll = m_ntuple_collection.Begin(); coll != m_ntuple_collection.End(); ++coll){
    coll->second.m_evtId = -1;
    coll->second.m_global_time= 0.;
    coll->second.m_G4EvtPrimaryEnergy.clear();
    coll->second.m_EvtPrimariesN = 0;
    coll->second.m_VolumeHitEDeposit.clear();
    coll->second.m_VolumeHitMeanEDeposit.clear();
    coll->second.m_VolumeHitDose.clear();
    coll->second.m_HitsTrkId.clear();
    coll->second.m_HitsTrkTypeId.clear();
    coll->second.m_HitsTrkEnergy.clear();
    coll->second.m_HitsTrkTheta.clear();
    coll->second.m_HitsTrkLength.clear();
    coll->second.m_HitsTrkPosX.clear();
    coll->second.m_HitsTrkPosY.clear();
    coll->second.m_HitsTrkPosZ.clear();

  }
}