/*
 *  $Id: ProcessThisEventPsd.cc, 2014-10-24 15:07:00 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 09/03/2014
*/

#include "DmpDataBuffer.h"
#include "DmpAlgHex2Root.h"
#include "DmpPsdBase.h"

//-------------------------------------------------------------------
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
bool DmpAlgHex2Root::InitializePsd(){
  DmpLogInfo<<"Initialize\tPsd: Setting connector"<<DmpLogEndl;
// *
// *  TODO: setup connector
// *
  //-------------------------------------------------------------------
  fEvtPsd = new DmpEvtPsdRaw();
  gDataBuffer->RegisterObject("Event/Rdc/Psd",fEvtPsd,"DmpEvtPsdRaw");
  return true;
}

//-------------------------------------------------------------------
bool DmpAlgHex2Root::ProcessThisEventPsd(const long &id){
  if(fPsdBuf.find(id) == fPsdBuf.end()){
  std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<") not find "<<id<<std::endl;
    return false;
  }
std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
  fEvtPsd->Reset();
  short nFee = fPsdBuf[id].size();
  for(short i=0;i<nFee;++i){
    fEvtPsd->fFeeNavig.push_back(fPsdBuf[id][i]->Navigator);
    short feeID = fPsdBuf[id][i]->Navigator.GetFeeID();
    DmpERunMode::Type runMode = fPsdBuf[id][i]->Navigator.GetRunMode();
    if(DmpERunMode::kCompress == runMode){
      short nChannel = fPsdBuf[id][i]->Signal.size()/3;
      for(size_t c=0;c<nChannel;++c){
        short channelID = (short)(unsigned char)fPsdBuf[id][i]->Signal[3*c];
        short v = (short)((fPsdBuf[id][i]->Signal[3*c+1]<<8) | (fPsdBuf[id][i]->Signal[3*c+2]&0x00ff));
        //fEvtPsd->fADC.insert(std::make_pair(fMapPsd[feeID*1000+channelID],v));
        fEvtPsd->fGlobalDyID.push_back(fMapPsd[feeID*1000+channelID]);
        fEvtPsd->fADC.push_back(v);
      }
    }else if(DmpERunMode::kOriginal == runMode || DmpERunMode::kCalDAC == runMode){
      short nChannel = fPsdBuf[id][i]->Signal.size()/2;
      for(size_t c=0;c<nChannel;++c){
        short v = (short)((fPsdBuf[id][i]->Signal[2*c]<<8) | (fPsdBuf[id][i]->Signal[2*c+1]&0x00ff));
        //fEvtPsd->fADC.insert(std::make_pair(fMapPsd[feeID*1000+c],v));
        fEvtPsd->fGlobalDyID.push_back(fMapPsd[feeID*1000+c]);
        fEvtPsd->fADC.push_back(v);
      }
    }else{
      DmpLogError<<" Wrong mode... FeeID = "<<std::hex<<feeID<<"\tmode "<<runMode<<std::dec; PrintTime();
    }
    // flag check
    if(fEvtPsd->fFeeNavig[i].CRCError()){
      DmpLogError<<"Fee(Psd)_0x"<<std::hex<<feeID<<std::dec<<"\tCRC error";PrintTime();
      fEvtHeader->SetTag(DmpEDetectorID::kPsd,DmpEvtHeader::tag_CRCError);
    }
    if(fEvtPsd->fFeeNavig[i].ReceivedTriggerCheck_Wrong()){
      DmpLogError<<"Fee(Psd)_0x"<<std::hex<<feeID<<std::dec<<"\treceived trigger check signal from trigger system, check error";PrintTime();
      fEvtHeader->SetTag(DmpEDetectorID::kPsd,DmpEvtHeader::tag_TrigFlagError);
    }
    if(fEvtPsd->fFeeNavig[i].PackageFlagError()){
      DmpLogError<<"Fee(Psd)_0x"<<std::hex<<feeID<<"\tpackage flag error: "<<fEvtPsd->fFeeNavig[i].GetPackageFlag()<<std::dec;PrintTime();
      fEvtHeader->SetTag(DmpEDetectorID::kPsd,DmpEvtHeader::tag_PkgFlagError);
    }
  }
  // inside triggers match
  if(not fEvtPsd->TriggersMatch()){
    fEvtHeader->SetTag(DmpEDetectorID::kPsd,DmpEvtHeader::tag_TrigNotMatchInSubDet);
    DmpLogError<<"Triggers of Psd Fee not match\t"<<DmpLogEndl;
    for(size_t i=0;i<nFee;++i){
      DmpLogCout<<"\tFeeID = "<<std::hex<<fEvtPsd->fFeeNavig[i].GetFeeID()<<"\ttrigger = "<<fEvtPsd->fFeeNavig[i].GetTrigger()<<std::dec;PrintTime();
    }
  }
  // trigger match trigger system
  if(fEvtHeader->GetTrigger() != fEvtPsd->GetTrigger()){
    fEvtHeader->SetTag(DmpEDetectorID::kPsd,DmpEvtHeader::tag_TrigNotMatchTrigSys);
    DmpLogWarning<<"Psd trigger not match trigger system.\t Psd = "<<fEvtPsd->GetTrigger()<<" trigger system = "<<fEvtHeader->GetTrigger();PrintTime();
  }
std::cout<<"DEBUG: "<<__FILE__<<"("<<__LINE__<<")"<<std::endl;
  return true;
}


