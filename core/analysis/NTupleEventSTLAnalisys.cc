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
#include "D3DCell.hh"
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
    // Given tree is related to single scoring volume (hits collection)
    m_ttree_collection.emplace_back(NTupleEventSTLAnalisys::TTreeCollection());
    m_ttree_collection.back().m_name = treeName;
    m_ttree_collection.back().m_hc_names.emplace_back(treeName);
    m_ttree_collection.back().m_description = treeDescription;
  }
  else {
    // Given tree is related to many scoring volume (hits collection)
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
void NTupleEventSTLAnalisys::SetTracksAnalysis(const G4String& treeName, bool flag){
  if (Service<ConfigSvc>()->GetValue<bool>("RunSvc", "NTupleAnalysis") == false)
    return;
  for(auto& tree : m_ttree_collection){
    if (tree.m_name==treeName){
      tree.m_tracks_analysis = flag;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void NTupleEventSTLAnalisys::BeginOfRun(const G4Run* runPtr, G4bool isMaster){
  if (Service<ConfigSvc>()->GetValue<bool>("RunSvc", "NTupleAnalysis") == false)
    return;
  // Book Voxel data Ntuple for all HitsColletions
  //------------------------------------------
  m_runId = runPtr->GetRunID();
  auto runSvc = Service<RunSvc>();
  auto control_point = runSvc->CurrentControlPoint();
  m_degree_rotation = control_point->GetDegreeRotation();
  for(const auto& tree : NTupleEventSTLAnalisys::m_ttree_collection){
    if(GetNTupleId(tree.m_name+m_treeNamePostfix) == -1){ // not created yet
      CreateNTuple(tree);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
// void NTupleEventSTLAnalisys::CreateNTuple(const G4String& treeName, const G4String& treeDescription){
void NTupleEventSTLAnalisys::CreateNTuple(const TTreeCollection& treeColl){
  auto treeName = treeColl.m_name+m_treeNamePostfix;
  auto treeDescription = treeColl.m_description;
  // G4cout << "[DEUBG]:: NTupleEventSTLAnalisys::Creating TTree: "<< treeName << ":"  << treeDescription << G4endl;
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
  createNtupleDColumn("GantryAngle");
  createNtupleDColumn("VoxelEDeposit");
  createNtupleDColumn("VoxelMeanEDeposit");
  createNtupleDColumn("VoxelDose");
  createNtupleVecIColumn("VoxelTrkId",evtNTupleColl.m_VoxelTrkId);
  createNtupleVecIColumn("VoxelTrkTypeId",evtNTupleColl.m_VoxelTrkTypeId);
  createNtupleVecDColumn("VoxelTrkE",evtNTupleColl.m_VoxelTrkEnergy);
  createNtupleVecDColumn("VoxelTrkTheta",evtNTupleColl.m_VoxelTrkTheta);
  createNtupleVecDColumn("VoxelTrkLength",evtNTupleColl.m_VoxelTrkLength);
  createNtupleVecDColumn("VoxelTrkPositionX",evtNTupleColl.m_VoxelTrkPositionX);
  createNtupleVecDColumn("VoxelTrkPositionY",evtNTupleColl.m_VoxelTrkPositionY);
  createNtupleVecDColumn("VoxelTrkPositionZ",evtNTupleColl.m_VoxelTrkPositionZ);

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
  auto& evtColl = m_ntuple_collection.Get(treeName); // Note: once the treeName is wrong this gives undefined beheviour!!
  int nHits = hitsColl->entries();
  // LOGSVC_INFO("nHist: {}",nHits);
  if(nHits==0){
   return; // no hits in this event
  }
  evtColl.m_evtId = evt->GetEventID();
  G4double totalEnergyDeposit = 0;
  G4double totalDose = 0;

  // std::cout << "EvtId: " << evt->GetEventID() << std::endl;
  std::vector<G4double> hits_evt_global_time;
  for (int i=0;i<nHits;i++){ // a.k.a. voxel loop

    auto hit = dynamic_cast<VoxelHit*>(hitsColl->GetHit(i));
    auto hitEDeposit = hit->GetEnergyDeposit() / keV;
    auto hitMeanEDeposit = hit->GetMeanEnergyDeposit() / keV;



    auto voxelDose = hit->GetDose(); // note: it's in gray already;
    auto size = D3DCell::SIZE;
    double cellVolume = pow(size,3);
    auto cellDose = voxelDose * hit->GetVolume() / cellVolume;
    evtColl.m_CellIDose.emplace_back( cellDose );

    // G4cout << hitsColl->GetName() << " dose: " << cellDose << G4endl;


    if(!evtColl.m_minimalistic_ttree){
      auto hitCentre = (hit->GetCentre()) + hit->GetGlobalCentre();
      evtColl.m_VoxelPositionX.push_back(hitCentre.x());
      evtColl.m_VoxelPositionY.push_back(hitCentre.y());
      evtColl.m_VoxelPositionZ.push_back(hitCentre.z());
    }

    totalEnergyDeposit+=hitEDeposit;
    if(!evtColl.m_minimalistic_ttree){
      evtColl.m_VoxelHitEDeposit.emplace_back(hitEDeposit);
      evtColl.m_VoxelHitMeanEDeposit.emplace_back(hitMeanEDeposit);
    }
    evtColl.m_VoxelHitDose.emplace_back( voxelDose );

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
      evtColl.m_VoxelHitsTrkId.emplace_back(trkType);
      evtColl.m_VoxelHitsTrkTypeId.emplace_back(trkTypeId);

      std::vector<G4double> trkEnergy;
      for(const auto& iTrkE : hit->GetTrkEnergy()){
        trkEnergy.emplace_back(iTrkE.second / MeV);
      }
      evtColl.m_VoxelHitsTrkEnergy.emplace_back(trkEnergy);

      std::vector<G4double> trkTheta;
      for(const auto& iTrkTheta : hit->GetTrkTheta()){
        trkTheta.emplace_back(iTrkTheta.second);
      }
      evtColl.m_VoxelHitsTrkTheta.emplace_back(trkTheta);

      std::vector<G4double> trkPosX;
      std::vector<G4double> trkPosY;
      std::vector<G4double> trkPosZ;
      for(const auto& iTrkPos : hit->GetTrkPosition()){
        trkPosX.emplace_back(iTrkPos.second.getX()-isoToSim.x());
        trkPosY.emplace_back(iTrkPos.second.getY()-isoToSim.y());
        trkPosZ.emplace_back(iTrkPos.second.getZ()-isoToSim.z());
      }
      evtColl.m_VoxelHitsTrkPosX.emplace_back(trkPosX);
      evtColl.m_VoxelHitsTrkPosY.emplace_back(trkPosY);
      evtColl.m_VoxelHitsTrkPosZ.emplace_back(trkPosZ);

      std::vector<G4double> trkLength;
      for(const auto& iTrkLength : hit->GetTrkLength()){
        trkLength.emplace_back(iTrkLength.second);
      }
      evtColl.m_VoxelHitsTrkLength.emplace_back(trkLength);
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
          fillNtupleDColumn("GantryAngle",m_degree_rotation);

          fillNtupleDColumn("CellEDeposit",treeEvtColl.m_VoxelHitEDeposit.at(i));         // take value from voxel (vox volume==cell)
          fillNtupleDColumn("CellMeanEDeposit",treeEvtColl.m_VoxelHitMeanEDeposit.at(i)); // take value from voxel (vox volume==cell)
          fillNtupleDColumn("CellDose",treeEvtColl.m_CellIDose.at(i));





          fillNtupleDColumn("VoxelPositionX",treeEvtColl.m_VoxelPositionX.at(i));
          fillNtupleDColumn("VoxelPositionY",treeEvtColl.m_VoxelPositionY.at(i));
          fillNtupleDColumn("VoxelPositionZ",treeEvtColl.m_VoxelPositionZ.at(i));
          fillNtupleDColumn("VoxelEDeposit",treeEvtColl.m_VoxelHitEDeposit.at(i));
          fillNtupleDColumn("VoxelMeanEDeposit",treeEvtColl.m_VoxelHitMeanEDeposit.at(i));
          fillNtupleDColumn("VoxelDose",treeEvtColl.m_VoxelHitDose.at(i));


          treeEvtColl.m_VoxelTrkId.clear();
          treeEvtColl.m_VoxelTrkId = treeEvtColl.m_VoxelHitsTrkId.at(i);
          treeEvtColl.m_VoxelTrkTypeId.clear();
          treeEvtColl.m_VoxelTrkTypeId = treeEvtColl.m_VoxelHitsTrkTypeId.at(i);
          treeEvtColl.m_VoxelTrkEnergy.clear();
          treeEvtColl.m_VoxelTrkEnergy = treeEvtColl.m_VoxelHitsTrkEnergy.at(i);
          treeEvtColl.m_VoxelTrkTheta.clear();
          treeEvtColl.m_VoxelTrkTheta = treeEvtColl.m_VoxelHitsTrkTheta.at(i);
          treeEvtColl.m_VoxelTrkLength.clear();
          treeEvtColl.m_VoxelTrkLength = treeEvtColl.m_VoxelHitsTrkLength.at(i);
          treeEvtColl.m_VoxelTrkPositionX.clear();
          treeEvtColl.m_VoxelTrkPositionX = treeEvtColl.m_VoxelHitsTrkPosX.at(i);
          treeEvtColl.m_VoxelTrkPositionY.clear();
          treeEvtColl.m_VoxelTrkPositionY = treeEvtColl.m_VoxelHitsTrkPosY.at(i);
          treeEvtColl.m_VoxelTrkPositionZ.clear();
          treeEvtColl.m_VoxelTrkPositionZ = treeEvtColl.m_VoxelHitsTrkPosZ.at(i);


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


    coll->second.m_VoxelHitEDeposit.clear();
    coll->second.m_VoxelHitMeanEDeposit.clear();
    coll->second.m_VoxelHitDose.clear();

    coll->second.m_VoxelHitsTrkId.clear();
    coll->second.m_VoxelHitsTrkTypeId.clear();
    coll->second.m_VoxelHitsTrkEnergy.clear();
    coll->second.m_VoxelHitsTrkTheta.clear();
    coll->second.m_VoxelHitsTrkLength.clear();
    coll->second.m_VoxelHitsTrkPosX.clear();
    coll->second.m_VoxelHitsTrkPosY.clear();
    coll->second.m_VoxelHitsTrkPosZ.clear();
    
    coll->second.m_CellIDose.clear();

  }
}