add_compile_options(-O2 -flto -Wno-error=sign-compare -Wno-error=unused-variable)

include(BasicPlugin)
include_directories("${dunedetdataformats_DIR}/../../../include")

basic_plugin(triggeralgs module
  BASENAME_ONLY
  LIBRARIES
  OfflineTPCChannelMap_module
  SOURCE
  src/TAWindow.cpp
  src/TPWindow.cpp
  src/TriggerActivityMakerADCSimpleWindow.cpp
  src/TriggerActivityMakerBundleN.cpp
  src/TriggerActivityMakerChannelAdjacency.cpp
  src/TriggerActivityMakerChannelDistance.cpp
  src/TriggerActivityMakerDBSCAN.cpp
  src/TriggerActivityMakerHorizontalMuon.cpp
  src/TriggerActivityMakerMichelElectron.cpp
  src/TriggerActivityMakerPlaneCoincidence.cpp
  src/TriggerActivityMakerPrescale.cpp
  src/TriggerActivityMakerSNCluster.cpp
  src/TriggerActivityMakerSupernova.cpp
  src/TriggerCandidateMakerADCSimpleWindow.cpp
  src/TriggerCandidateMakerBundleN.cpp
  src/TriggerCandidateMakerChannelAdjacency.cpp
  src/TriggerCandidateMakerChannelDistance.cpp
  src/TriggerCandidateMakerDBSCAN.cpp
  src/TriggerCandidateMakerHorizontalMuon.cpp
  src/TriggerCandidateMakerMichelElectron.cpp
  src/TriggerCandidateMakerPlaneCoincidence.cpp
  src/TriggerCandidateMakerPrescale.cpp
  src/TriggerCandidateMakerSNCluster.cpp
  src/TriggerCandidateMakerSupernova.cpp
  src/TriggerDecisionMakerSupernova.cpp
  src/dbscan/dbscan.cpp
  src/dbscan/Hit.cpp
)
