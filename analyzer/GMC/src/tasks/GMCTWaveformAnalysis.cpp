////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// GMCTWaveformAnalysis                                                       //
//                                                                            //
// Begin_Html <!--
/*-->

<!--*/
// --> End_Html
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/* Generated header file containing necessary includes                        */
#include "generated/GMCTWaveformAnalysisGeneratedIncludes.h"

////////////////////////////////////////////////////////////////////////////////
/*  This header was generated by ROMEBuilder. Manual changes above the        *
 * following line will be lost next time ROMEBuilder is executed.             */
/////////////////////////////////////----///////////////////////////////////////

#include "generated/GMCAnalyzer.h"
#include "generated/GMCWaveformData.h"

#include "tasks/GMCTWaveformAnalysis.h"
#include "ROMEiostream.h"

// uncomment if you want to include headers of all folders
//#include "GMCAllFolders.h"


ClassImp(GMCTWaveformAnalysis)
using namespace std;
//______________________________________________________________________________
void GMCTWaveformAnalysis::Init() {

  Int_t runNr = gAnalyzer->GetRunNumberAt(0);

  //  TString filename = gAnalyzer->GetInputFileNamesStringOriginal();
  TString filename(Form("MCHits%05d.root",runNr));

  cout<<"Load manually the file data : "<<filename.Data()<<endl;

  TFile *fIn = TFile::Open(Form("./%s",filename.Data()));

  if (fIn->IsOpen()) printf(" ***** The file is open correctly!!! \n");
  else {
  
     printf(" ***** Error in opening file data !!! \n");
     return;
  }
  
  fDataTree = (TTree*)fIn->Get("MCHits");

  if (fDataTree->GetEntries() == 0) {
  
     printf(" ***** File data is empty !!! \n");
     return;
     
  }



}

//______________________________________________________________________________
void GMCTWaveformAnalysis::BeginOfRun() {

  printf(" ***** Set Waveforms branch address \n");
  fBrDataWave = new TClonesArray("GMCWaveformData",0);
  fDataTree->SetBranchAddress("DCWaveforms",&fBrDataWave);


}

//______________________________________________________________________________
void GMCTWaveformAnalysis::Event() {

  printf("\n ***** Load event %lld ************************ \n",gAnalyzer->GetCurrentEventNumber());
  LoadEvent(gAnalyzer->GetCurrentEventNumber());

  cout<<"nell'array ci sono waveforms: "<<fBrDataWave->GetEntries()<<endl;

  for (int iwave=0;iwave<fBrDataWave->GetEntries();iwave++) {

    GMCWaveformData *wv = (GMCWaveformData *)fBrDataWave->At(iwave);
    Int_t npeaks = FindPeaks(wv->GetfNpoints(), wv->GetfSignalL());

  }

}

//______________________________________________________________________________
void GMCTWaveformAnalysis::LoadEvent(Int_t nev) {

  //return the nev-th event from data tree
  fDataTree->GetEntry(nev-1);

}

//______________________________________________________________________________
Int_t GMCTWaveformAnalysis::FindPeaks(Int_t npt, Double_t *amplitude) {

  Int_t nrise=12;
  Int_t checkUpTo=12;
  
  if(checkUpTo>nrise) { checkUpTo=nrise; }

  Float_t sig=0.5e-3;
  Float_t *sigd = new Float_t[nrise];
  Float_t *sigd_2 = new Float_t[nrise];
  
  TH1F tmpWdist("tmpWdist","",1000,-0.5,0.5);
  for (int i=100; i<1000; ++i) { tmpWdist.Fill(amplitude[i]); }
  sig = tmpWdist.GetRMS();
  Float_t mean = tmpWdist.GetMean();
  
  cout<<"Signal Off "<<mean<<" noise "<<sig<<endl;
  
  for (int ir=checkUpTo; ir<=nrise; ++ir) {
    int irId = ir-1;
    sigd[irId]=2.4495*sig/((float)(2*ir+1));
    sigd_2[irId]=1.414*sigd[irId];//%0
    sigd_2[irId]*=7.0;
    sigd[irId]*=7.0;
  }
  sig*=7.0*1.414;  //stava a 4

  Float_t *wave = new Float_t[npt];
  Float_t **riseMeas = new Float_t*[npt];// (r,nrise);
  Float_t **deri = new Float_t*[npt];// zeros(r,nrise);
  for (int ip=0; ip<npt; ++ip) {
    wave[ip]=-1.0*(amplitude[ip]-mean);
    riseMeas[ip]=new Float_t [nrise];
    deri[ip]=new Float_t [nrise];
    for (int ir=0; ir<nrise; ++ir) {
      riseMeas[ip][ir]=0.0;
      deri[ip][ir]=0.0;
    }
  }

  Int_t  maxNPks = 100;
  Int_t pkPos[maxNPks];
  Float_t pkHgt[maxNPks];
  Int_t nPks=0;
  Int_t last=-nrise;
  for (int i=nrise+1; i<npt-1; ++i) {
    for (int ir=nrise; ir>=checkUpTo; --ir) {
      int irId = ir-1;
      deri[i][irId] = (2.0*wave[i]-wave[i-ir]-wave[i-ir-1])/((float)(2*ir+1));
      riseMeas[i][irId] = (deri[i][irId]-deri[i-ir][irId]);
      
      if ( deri[i][irId]>=sigd[irId] && (deri[i][irId]-deri[i-ir][irId])>sigd_2[irId] && (wave[i]-wave[i-ir])>sig ) {
        //%if globFlag
        if ((i-last)>ir /*%1*/) {
            pkHgt[nPks]=-1.0*(wave[i]-mean);
            pkPos[nPks]=i;
            ++nPks;
            last=i;
        } else {
            pkHgt[nPks-1]=-1.0*(wave[i]-mean);
            pkPos[nPks-1]=i;
            last=i;
        }
      }
      if(nPks==maxNPks) {break;}
    }
    if(nPks==maxNPks) {break;}
  }

  delete [] sigd;
  delete [] sigd_2;
  delete [] wave;
  for (int ip=0; ip<npt; ++ip) {
    delete [] riseMeas[ip];
    delete [] deri[ip];
  }
  delete [] riseMeas;
  delete [] deri;
    
  return nPks;

}

//______________________________________________________________________________
void GMCTWaveformAnalysis::EndOfRun()
{
}

//______________________________________________________________________________
void GMCTWaveformAnalysis::Terminate()
{
}

