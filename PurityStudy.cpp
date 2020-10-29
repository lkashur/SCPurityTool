//some standard C++ includes
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <numeric>

//some ROOT includes
#include "TInterpreter.h"
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TChain.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TProfile2D.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TMath.h"
#include "TGraph.h"
#include "TPaletteAxis.h"
#include "TLegend.h"
#include "TVector3.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TVirtualFFT.h"
#include "TFitResult.h"
#include "TPaveText.h"

using namespace std;
using namespace std::chrono;

const int numBins = 15;
const double driftVel = 0.155;
const double driftTimeMax = 186.0;
const double driftTimeRange = 6.0;
const double gain = 250.0*3.9;
const double engConv = 0.0000236/0.66;
const double pedestal = 78.0;

int main(int argc, char **argv)
{
  ///////////////////////////////////
  // Set Plot Formatting Options
  ///////////////////////////////////

  gErrorIgnoreLevel = kError;
  double stops[5] = {0.00,0.34,0.61,0.84,1.00};
  double red[5] = {0.00,0.00,0.87,1.00,0.51};
  double green[5] = {0.00,0.81,1.00,0.20,0.00};
  double blue[5] = {0.51,1.00,0.12,0.00,0.00};
  TColor::CreateGradientColorTable(5,stops,red,green,blue,255);
  gStyle->SetNumberContours(255);
  gStyle->SetOptStat(0);

  ///////////////////////////////////
  // Get Input File Name
  ///////////////////////////////////

  Char_t *inputfilename = (Char_t*)"";
  if (argc < 2)
  {
    cout << endl << "No input file name specified!  Aborting." << endl << endl;
    return -1;
  }
  else
  {
    inputfilename = argv[1];
  }

  TFile outfile("results.root","RECREATE");
  outfile.cd();

  ///////////////////////////////////
  // Define Histograms
  ///////////////////////////////////

  TH2F *LifetimeHist2D = new TH2F("LifetimeHist2D","",numBins,0.0,driftTimeMax,50,0.0,160.0);
  TH1F *LifetimeHist2D_ProjX = (TH1F*) LifetimeHist2D->ProjectionX();
  TProfile *LifetimeHist2D_ProfileX;
  
  TH1F *dEdxHist = new TH1F("dEdxHist","",50,0.0,6.0);
  TH2F *dEdxHist2D = new TH2F("dEdxHist2D","",numBins,0.0,driftTimeMax,50,0.0,6.0);
  TH1F *dEdxHist2D_ProjX = (TH1F*) dEdxHist2D->ProjectionX();
    
  ///////////////////////////////////
  // Load Input Data
  ///////////////////////////////////

  TFile* inputfile = new TFile(inputfilename,"READ");
  
  TTreeReader readerTracks("trackTree", inputfile);
  TTreeReaderValue<int> eventNum(readerTracks, "eventNum");
  TTreeReaderValue<int> trackNum(readerTracks, "trackNum");
  TTreeReaderValue<double> minT_X(readerTracks, "minT_X");
  TTreeReaderValue<double> minT_Y(readerTracks, "minT_Y");
  TTreeReaderValue<double> minT_T(readerTracks, "minT_T");
  TTreeReaderValue<double> maxT_X(readerTracks, "maxT_X");
  TTreeReaderValue<double> maxT_Y(readerTracks, "maxT_Y");
  TTreeReaderValue<double> maxT_T(readerTracks, "maxT_T");
  TTreeReaderArray<double> trackHitX(readerTracks, "trackHitX");
  TTreeReaderArray<double> trackHitY(readerTracks, "trackHitY");
  TTreeReaderArray<double> trackHitT(readerTracks, "trackHitT");
  TTreeReaderArray<double> trackHitC(readerTracks, "trackHitC");

  ///////////////////////////////////
  // First Loop Over Data
  ///////////////////////////////////

  while (readerTracks.Next())
  {
    if((*maxT_T - *minT_T < 10.0*(driftTimeMax-driftTimeRange)) || (*maxT_T - *minT_T > 10.0*(driftTimeMax+driftTimeRange)))
    {
      continue;
    }

    double charge[numBins] = {0.0};
	
    int numHits = trackHitT.GetSize();
    for(int k = 0; k < numHits; k++)
    {
      int index = round(((((float) numBins)/driftTimeMax/10.0)*(trackHitT[k]-*minT_T))-0.5);
      if(index < 0)
      {
        index = 0;
      }
      else if(index > numBins-1)
      {
        index = numBins-1;
      }
      charge[index] += trackHitC[k]-pedestal;
    }

    double pitch = 0.1*(10.0*(dEdxHist2D_ProjX->GetBinCenter(2)-dEdxHist2D_ProjX->GetBinCenter(1))/(*maxT_T-*minT_T))*sqrt(pow(*maxT_X-*minT_X,2)+pow(*maxT_Y-*minT_Y,2)+pow(driftVel*(*maxT_T-*minT_T),2));

    for(int i = 1; i < numBins-1; i++)
    {
      LifetimeHist2D->Fill(LifetimeHist2D_ProjX->GetBinCenter(i+1),gain*charge[i]/pitch/1000.0);
    }
  }

  ///////////////////////////////////
  // Extract Electron Lifetime
  ///////////////////////////////////

  outfile.cd();
  LifetimeHist2D_ProfileX = LifetimeHist2D->ProfileX();
  LifetimeHist2D_ProfileX->SetName("LifetimeHist2D_ProfileX");
  TFitResultPtr r = LifetimeHist2D_ProfileX->Fit("expo","QSE");
  double lifetime_val = fabs(1.0/r->Parameter(1))/1000.0;
  double lifetime_uncert = fabs(r->Error(1)/r->Parameter(1))*lifetime_val;

  cout << "Electron Lifetime:  " << lifetime_val << " +- " << lifetime_uncert << " ms" << endl;

  ///////////////////////////////////
  // Second Loop Over Data
  ///////////////////////////////////

  readerTracks.Restart();
  
  while (readerTracks.Next())
  {
    if((*maxT_T - *minT_T < 10.0*(driftTimeMax-driftTimeRange)) || (*maxT_T - *minT_T > 10.0*(driftTimeMax+driftTimeRange)))
    {
      continue;
    }

    double charge[numBins] = {0.0};
	
    int numHits = trackHitT.GetSize();
    for(int k = 0; k < numHits; k++)
    {
      int index = round(((((float) numBins)/driftTimeMax/10.0)*(trackHitT[k]-*minT_T))-0.5);
      if(index < 0)
      {
        index = 0;
      }
      else if(index > numBins-1)
      {
        index = numBins-1;
      }
      charge[index] += trackHitC[k]-pedestal;
    }

    double pitch = 0.1*(10.0*(dEdxHist2D_ProjX->GetBinCenter(2)-dEdxHist2D_ProjX->GetBinCenter(1))/(*maxT_T-*minT_T))*sqrt(pow(*maxT_X-*minT_X,2)+pow(*maxT_Y-*minT_Y,2)+pow(driftVel*(*maxT_T-*minT_T),2));
    
    for(int i = 0; i < numBins; i++)
    {
      double lifetimeCorr = exp(dEdxHist2D_ProjX->GetBinCenter(i+1)/(1000.0*lifetime_val));
      dEdxHist2D->Fill(dEdxHist2D_ProjX->GetBinCenter(i+1),engConv*gain*lifetimeCorr*charge[i]/pitch);
      if((i > 0) && (i < numBins-1))
      {
        dEdxHist->Fill(engConv*gain*lifetimeCorr*charge[i]/pitch);
      }
    }
  }

  inputfile->Close();

  ///////////////////////////////////
  // Make Plots
  ///////////////////////////////////

  TCanvas c1;
  c1.cd();
  LifetimeHist2D->Draw("COLZ");
  LifetimeHist2D->SetTitle("");
  LifetimeHist2D->GetXaxis()->SetTitle("Drift Time [#mus]");
  LifetimeHist2D->GetXaxis()->SetTitleSize(0.045);
  LifetimeHist2D->GetXaxis()->SetTitleOffset(1.05);
  LifetimeHist2D->GetXaxis()->SetLabelSize(0.04);
  LifetimeHist2D->GetYaxis()->SetTitle("dQ/dx [ke#lower[-0.5]{-}/cm]");
  LifetimeHist2D->GetYaxis()->SetTitleSize(0.045);
  LifetimeHist2D->GetYaxis()->SetTitleOffset(1.12);
  LifetimeHist2D->GetYaxis()->SetLabelSize(0.04);
  c1.SaveAs("LifetimeHist2D.png");

  TCanvas c2;
  c2.cd();
  LifetimeHist2D_ProfileX->Draw();
  LifetimeHist2D_ProfileX->SetMarkerColor(kBlack);
  LifetimeHist2D_ProfileX->SetLineColor(kBlack);
  LifetimeHist2D_ProfileX->SetLineWidth(3.0);
  LifetimeHist2D_ProfileX->SetTitle("");
  LifetimeHist2D_ProfileX->GetXaxis()->SetTitle("Drift Time [#mus]");
  LifetimeHist2D_ProfileX->GetXaxis()->SetTitleSize(0.045);
  LifetimeHist2D_ProfileX->GetXaxis()->SetTitleOffset(1.05);
  LifetimeHist2D_ProfileX->GetXaxis()->SetLabelSize(0.04);
  LifetimeHist2D_ProfileX->GetYaxis()->SetTitle("Mean dQ/dx [ke#lower[-0.5]{-}/cm]");
  LifetimeHist2D_ProfileX->GetYaxis()->SetTitleSize(0.045);
  LifetimeHist2D_ProfileX->GetYaxis()->SetTitleOffset(1.12);
  LifetimeHist2D_ProfileX->GetYaxis()->SetLabelSize(0.04);
  LifetimeHist2D_ProfileX->GetYaxis()->SetRangeUser(0.0,160.0);
  TPaveText* text = new TPaveText(0.45,0.7,0.85,0.85,"nbNDC");
  text->AddText(Form("Elec. Lifetime:  %.2f #pm %.2f ms",lifetime_val,lifetime_uncert));
  text->SetFillColor(kWhite);
  text->Draw("SAME");
  c2.SaveAs("LifetimeHist2D_ProfileX.png");

  TCanvas c3;
  c3.cd();
  dEdxHist->Draw("HIST");
  dEdxHist->SetLineColor(kBlue);
  dEdxHist->SetLineWidth(3.0);
  dEdxHist->SetTitle("");
  dEdxHist->GetXaxis()->SetTitle("dE/dx [MeV/cm]");
  dEdxHist->GetXaxis()->SetTitleSize(0.045);
  dEdxHist->GetXaxis()->SetTitleOffset(1.05);
  dEdxHist->GetXaxis()->SetLabelSize(0.04);
  dEdxHist->GetYaxis()->SetTitle("# of Entries");
  dEdxHist->GetYaxis()->SetTitleSize(0.045);
  dEdxHist->GetYaxis()->SetTitleOffset(1.12);
  dEdxHist->GetYaxis()->SetLabelSize(0.04);
  c3.SaveAs("dEdxHist.png");

  TCanvas c4;
  c4.cd();
  dEdxHist2D->Draw("COLZ");
  dEdxHist2D->SetTitle("");
  dEdxHist2D->GetXaxis()->SetTitle("Drift Time [#mus]");
  dEdxHist2D->GetXaxis()->SetTitleSize(0.045);
  dEdxHist2D->GetXaxis()->SetTitleOffset(1.05);
  dEdxHist2D->GetXaxis()->SetLabelSize(0.04);
  dEdxHist2D->GetYaxis()->SetTitle("dE/dx [MeV/cm]");
  dEdxHist2D->GetYaxis()->SetTitleSize(0.045);
  dEdxHist2D->GetYaxis()->SetTitleOffset(1.12);
  dEdxHist2D->GetYaxis()->SetLabelSize(0.04);
  c4.SaveAs("dEdxHist2D.png");
  
  ///////////////////////////////////
  // Write Output File
  ///////////////////////////////////
  
  outfile.cd();

  LifetimeHist2D->Write();
  LifetimeHist2D_ProfileX->Write();
  dEdxHist->Write();
  dEdxHist2D->Write();
  
  outfile.Close();

  return 0;
}
