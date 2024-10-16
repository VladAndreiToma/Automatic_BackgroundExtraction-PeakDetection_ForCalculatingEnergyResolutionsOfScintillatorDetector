#include <iostream>
#include <fstream>
#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TSpectrum.h"


using namespace std;

ofstream fout("logs.log");

// Function to read the ROOT file and extract the TTree
TTree* ReadTheFile(const char* fileName) {
    TFile* reader = TFile::Open(fileName);
    if (!reader || reader->IsZombie()) {
        cerr << "Error: Unable to open file " << fileName << endl;
        return nullptr;
    }
    TTree* myTree = dynamic_cast<TTree*>(reader->Get("data"));
    if (!myTree) {
        cerr << "Error: TTree 'data' not found in file " << fileName << endl;
        return nullptr;
    }
    return myTree;
}

// Function to fill a histogram from the TTree based on ChargeLong values
TH1F* formHisto(TTree* tree, UShort_t* cLong) {
    TH1F* histo = new TH1F("longCharge", "LongCharge", 1100, 200, 1300);
    int nEntries = tree->GetEntries();

    // Loop over entries and fill histogram
    for (int index = 0; index < nEntries; ++index) {
        tree->GetEntry(index);  // Load the tree entry
        histo -> Fill( *cLong );
    }
    return histo;
}


void draw_Histograms( TH1F* bgExtracted_Histo , TH1F* normal_Histo ){
    TCanvas* myCanvas = new TCanvas( "myCanvas" , "plot details Co60" );
    myCanvas -> Divide( 2 , 1 );
    myCanvas -> cd( 1 );
    normal_Histo -> SetTitle( "long_Charge" );
    normal_Histo -> SetLineWidth( 2 );
    normal_Histo -> SetLineColor( kRed );
    normal_Histo -> Draw( "HIST" );

    myCanvas -> cd( 2 );
    bgExtracted_Histo -> SetTitle( "bg_Extracted_long_Charge" );
    bgExtracted_Histo -> SetLineWidth( 2 );
    bgExtracted_Histo -> SetLineColor( kBlue );
    bgExtracted_Histo -> Draw( "HIST" );

}

// making analysis with TSpectrum
TH1F* Extract_Background_With_TSpectrum( TH1F* myHistogram , Int_t iterations ){
    // recursive algorhythm most probably behind that performs a number of iterations to ensure good bg extraction
    TSpectrum *spectrum = new TSpectrum();
    TH1 *background = spectrum->Background(myHistogram, iterations);
    TH1F* copyHist = (TH1F*)myHistogram->Clone();  // Use Clone() instead of Copy()
    copyHist->Add(background, -1);
    return copyHist;
}

// searching for peaks
void Search_For_Peaks( TH1F* histo , Double_t DEFAULT_GAUSSIAN_SPREAD ){
    // default values
    // sigmas and 
    Double_t threshold = 0.3;
    Double_t sigma = 30.5;
    TSpectrum *spectrum = new TSpectrum();
    Int_t nPeaks = spectrum -> Search( histo , sigma , "" , threshold );
    Double_t *xPeaks = spectrum->GetPositionX();
    Double_t *yPeaks = spectrum->GetPositionY();
    fout << "Number of peaks found: " << nPeaks << std::endl;
    for (Int_t i = 0; i < nPeaks; i++) {
        fout << "Peak " << i+1 << ": X = " << xPeaks[i] << ", Y = " << yPeaks[i] << std::endl;
    }

    // Loop over detected peaks and fit a Gaussian around each
    for (Int_t i = nPeaks - 1 ; i >= 0; i-- ) {
        Double_t peakPosition = xPeaks[i];  // X position of the peak
        Double_t fitRange = DEFAULT_GAUSSIAN_SPREAD;  // Range around the peak to fit (adjust as needed)
    
        // Fit a Gaussian function around the peak
        TF1 *gaussian = new TF1( "gaus", "gaus", peakPosition - fitRange, peakPosition + fitRange);
        histo -> Fit( gaussian, "R" );  // "R" ensures fit is done in the specified range

        // Extract mean and sigma from the fit
        Double_t mean = gaussian -> GetParameter(1);  // Mean (position of peak)
        Double_t sigma = gaussian -> GetParameter(2);  // Sigma (width of the peak)

        // Output the results
        fout << "Peak " << i+1 << ": Mean = " << mean << ", Sigma = " << sigma << " / Energy resolution: " << ( 2.54 * sigma / mean ) * 100 << "%" << std::endl;
    }
}

void analysis(){

    // 1. Read the ROOT file
    TTree* dataTree = ReadTheFile("run_500_60_8_CFD_SMOOTH_EXP_2_CFD_FRACTLIST_25_0.root");
    if (!dataTree) return;

    // 2. Set branch addresses
    UChar_t Mod, Ch;
    Double_t FineTS;
    UShort_t ChargeLong;
    Short_t ChargeShort;
    vector<Short_t> Signal;  // Assuming this branch stores signal values in a vector of Short_t

    dataTree->SetBranchAddress( "Mod", &Mod );
    dataTree->SetBranchAddress( "Ch", &Ch );
    dataTree->SetBranchAddress( "FineTS" , &FineTS );
    dataTree->SetBranchAddress( "ChargeLong", &ChargeLong );
    dataTree->SetBranchAddress( "ChargeShort", &ChargeShort );
    dataTree->SetBranchAddress( "Signal", &Signal );

    // 3. Create and fill the histogram for ChargeLong
    TH1F* h_longCharge = formHisto(dataTree, &ChargeLong);

    // 4. Draw basic histogram

    // 5. Extract the background
    TH1F* bg_extracted_longCharge = Extract_Background_With_TSpectrum( h_longCharge , 20 );

    draw_Histograms( bg_extracted_longCharge , h_longCharge );

    // 6 . search for peaks
    Double_t Co60_SPREAD = 30;
    Search_For_Peaks( bg_extracted_longCharge , Co60_SPREAD );

}









/*

    fout << "Tree of experiment is object: " << dataTree << endl;

    int APROX_MEAN_PEAK_1 = 910;
    int APROX_MEAN_PEAK_2 = 1030;
    int PEAK1_LEFT = 880;
    int PEAK1_RIGHT = 940;
    int PEAK2_LEFT = 1000;
    int PEAK2_RIGHT = 1070;

    TF1* gaus1 = new TF1( "gauss1" , "gaus" , PEAK1_LEFT , PEAK1_RIGHT );
    TF1* gaus2 = new TF1( "gauss2" , "gaus"  , PEAK2_LEFT , PEAK2_RIGHT );

h_longCharge -> Fit( gaus1 , "R" , "" , PEAK1_LEFT , PEAK1_RIGHT );
    h_longCharge -> Fit( gaus2 , "R" , "" , PEAK2_LEFT , PEAK2_RIGHT );
    

    // 4. Create a canvas and draw the histogram
    TCanvas* c1 = new TCanvas( "c1" , "ChargeLong Histogram" , 800 , 600 );
    h_longCharge -> Draw( "HIST" );
    gaus1 -> Draw( "SAME" );
    gaus2 -> Draw( "SAME" );

    // Save canvas to a file
    c1 -> SaveAs( "ChargeLong_Histogram.png" );
 */