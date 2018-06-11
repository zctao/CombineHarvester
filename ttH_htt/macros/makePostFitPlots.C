
#include <TFile.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1.h>
#include <THStack.h>
#include <TAxis.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TPaveText.h>
#include <TMath.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TList.h>
#include <TF1.h>
#include <TColor.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <assert.h>

/*
$ root -l
root [0] .L makePostFitPlots_1l_2tau.C+
makePostFitPlots_1l_2tau()
*/
// root -l -b -n -q /home/acaan/VHbbNtuples_8_0_x/CMSSW_7_4_7/src/CombineHarvester/ttH_htt/macros/makePostFitPlots.C++(\"noHTT_1B_noHTT_nbin_7\",\"/1l_2tau_1l_2tau_2018Feb08_VHbb_TightTau/noHTT/\",\"1l_2tau\",\"/home/acaan/CMSSW_9_4_0_pre1/src/tth-bdt-training-test/treatDatacards/\",uselog,minY,maxY,label)

TH1* getRebinnedHistogram1d(const TH1* histoOriginal,
			    unsigned numBins_rebinned)
{
  std::string histoRebinnedName = std::string(histoOriginal->GetName()).append("_rebinned");
  TH1* histoRebinned = new TH1D(
    histoRebinnedName.data(), histoOriginal->GetTitle(),
    5,-1,1);
  histoRebinned->Sumw2();
  const TAxis* axis_original = histoOriginal->GetXaxis();
  int numBins_original = axis_original->GetNbins();
  for ( int idxBin = 1; idxBin <= numBins_original; ++idxBin ) {
    double binContent_original = histoOriginal->GetBinContent(idxBin);
    double binError_original = histoOriginal->GetBinError(idxBin);
    double binCenter_original = axis_original->GetBinCenter(idxBin);
    int binIndex_rebinned = histoRebinned->FindBin(binCenter_original);
    double binContent_rebinned = histoRebinned->GetBinContent(binIndex_rebinned);
    binContent_rebinned += binContent_original;
    histoRebinned->SetBinContent(binIndex_rebinned, binContent_rebinned);
    double binError_rebinned = histoRebinned->GetBinError(binIndex_rebinned);
    binError_rebinned = TMath::Sqrt(binError_rebinned*binError_rebinned + binError_original*binError_original);
    histoRebinned->SetBinError(binIndex_rebinned, binError_rebinned);
  }
  return histoRebinned;
}

TH1* loadHistogram(TFile* inputFile, const std::string& directory, const std::string& histogramName, bool gentau)
{
  TH1* histogram;
	std::string histogramName_full = Form("%s/%s", directory.data(), histogramName.data());
	if ( !gentau ) {
		std::cout << "Loading = " << histogramName_full << " "<< std::endl;
		histogram = dynamic_cast<TH1*>(inputFile->Get(histogramName_full.data()));
	}else {
		std::string histogramName_full = Form("%s/%s_gentau", directory.data(), histogramName.data());
		std::cout << "Loading with gentau " << histogramName_full << " "<< std::endl;
		histogram = dynamic_cast<TH1*>(inputFile->Get(histogramName_full.data()));
		std::string histogramName_full2 = Form("%s/%s_faketau", directory.data(), histogramName.data());
		TH1* histogram2 = dynamic_cast<TH1*>(inputFile->Get(histogramName_full2.data()));
		histogram->Add(histogram2);
	}
  if ( !histogram ) {
    std::cerr << "Failed to load histogram = " << histogramName_full << " from file = " << inputFile->GetName() << " !!" << std::endl;
    assert(0);
  }
  if ( !histogram->GetSumw2N() ) histogram->Sumw2();
  return histogram;
}

double compIntegral(TH1* histogram, bool includeUnderflowBin, bool includeOverflowBin)
{
  double sumBinContent = 0.;
  int numBins = histogram->GetNbinsX();
  int firstBin = ( includeUnderflowBin ) ? 0 : 1;
  int lastBin = ( includeOverflowBin  ) ? (numBins + 1) : numBins;
  for ( int iBin = firstBin; iBin <= lastBin; ++iBin ) {
    sumBinContent += histogram->GetBinContent(iBin);
  }
  return sumBinContent;
}

double square(double x)
{
  return x*x;
}

void makeBinContentsPositive(TH1* histogram, int verbosity = 1)
{
  if ( verbosity ) {
    std::cout << "<makeBinContentsPositive>:" << std::endl;
    std::cout << " integral(" << histogram->GetName() << ") = " << histogram->Integral() << std::endl;
  }
  double integral_original = compIntegral(histogram, true, true);
  if ( integral_original < 0. ) integral_original = 0.;
  if ( verbosity ) {
    std::cout << " integral_original = " << integral_original << std::endl;
  }
  int numBins = histogram->GetNbinsX();
  for ( int iBin = 0; iBin <= (numBins + 1); ++iBin ) {
    double binContent_original = histogram->GetBinContent(iBin);
    double binError2_original = square(histogram->GetBinError(iBin));
    if ( binContent_original < 0. ) {
      double binContent_modified = 0.;
      double binError2_modified = binError2_original + square(binContent_original - binContent_modified);
      assert(binError2_modified >= 0.);
      if ( verbosity ) {
        std::cout << "bin #" << iBin << " (x =  " << histogram->GetBinCenter(iBin) << "): binContent = " << binContent_original << " +/- " << TMath::Sqrt(binError2_original)
                  << " --> setting it to binContent = " << binContent_modified << " +/- " << TMath::Sqrt(binError2_modified) << std::endl;
      }
      histogram->SetBinContent(iBin, binContent_modified);
      histogram->SetBinError(iBin, TMath::Sqrt(binError2_modified));
    } else if ( binContent_original < 0. ) {
			double binContent_modified = 0.;
      double binError2_modified = 0.;
		}
  }
  double integral_modified = compIntegral(histogram, true, true);
  if ( integral_modified < 0. ) integral_modified = 0.;
  if ( verbosity ) {
    std::cout << " integral_modified = " << integral_modified << std::endl;
  }
  if ( integral_modified > 0. ) {
    double sf = integral_original/integral_modified;
    if ( verbosity ) {
      std::cout << "--> scaling histogram by factor = " << sf << std::endl;
    }
    histogram->Scale(sf);
  } else {
    for ( int iBin = 0; iBin <= (numBins + 1); ++iBin ) {
      histogram->SetBinContent(iBin, 0.);
    }
  }
  if ( verbosity ) {
    std::cout << " integral(" << histogram->GetName() << ") = " << histogram->Integral() << std::endl;
  }
}

void dumpHistogram(TH1* histogram)
{
  std::cout << "<dumpHistogram>:" << std::endl;
  std::cout << " histogram: name = " << histogram->GetName() << ", title = " << histogram->GetTitle() << std::endl;
  std::cout << "  fillColor = " << histogram->GetFillColor() << ", fillStyle = " << histogram->GetFillStyle() << ","
	    << " lineColor = " << histogram->GetLineColor() << ", lineStyle = " << histogram->GetLineStyle() << ", lineWidth = " << histogram->GetLineWidth() << ","
	    << " markerColor = " << histogram->GetMarkerColor() << ", markerStyle = " << histogram->GetMarkerStyle() << ", markerSize = " << histogram->GetMarkerSize() << std::endl;
  TAxis* xAxis = histogram->GetXaxis();
  int numBins = xAxis->GetNbins();
  for ( int iBin = 1; iBin <= numBins; ++iBin ) {
    std::cout << "bin #" << iBin << " (x = " << xAxis->GetBinCenter(iBin) << "): " << histogram->GetBinContent(iBin) << " +/- " << histogram->GetBinError(iBin) << std::endl;
  }
  std::cout << "integral = " << compIntegral(histogram, true, true) << std::endl;
}

void checkCompatibleBinning(const TH1* histogram1, const TH1* histogram2)
{
  if ( histogram1 && histogram2 ) {
    if ( !(histogram1->GetNbinsX() == histogram2->GetNbinsX()) ) {
      std::cerr << "Histograms " << histogram1->GetName() << " and " << histogram2->GetName() << " have incompatible binning !!" << std::endl;
      std::cerr << " (NbinsX: histogram1 = " << histogram1->GetNbinsX() << ", histogram2 = " << histogram2->GetNbinsX() << ")" << std::endl;
      assert(0);
    }
    const TAxis* xAxis1 = histogram1->GetXaxis();
    const TAxis* xAxis2 = histogram2->GetXaxis();
    int numBins = xAxis1->GetNbins();
    for ( int iBin = 1; iBin <= numBins; ++iBin ) {
      double binWidth = 0.5*(xAxis1->GetBinWidth(iBin) + xAxis2->GetBinWidth(iBin));
      double dBinLowEdge = xAxis1->GetBinLowEdge(iBin) - xAxis2->GetBinLowEdge(iBin);
      double dBinUpEdge = xAxis1->GetBinUpEdge(iBin) - xAxis2->GetBinUpEdge(iBin);
      //if ( !(dBinLowEdge < (1.e-3*binWidth) && dBinUpEdge < (1.e-3*binWidth)) ) {
      //std::cerr << "Histograms boundtaries" << std::endl;
	//std::cerr << "Histograms " << histogram1->GetName() << " and " << histogram2->GetName() << " have incompatible binning !!" << std::endl;
	//std::cerr << " (bin #" << iBin << ": histogram1 = " << xAxis1->GetBinLowEdge(iBin) << ".." << xAxis1->GetBinUpEdge(iBin) << ","
	//	  << " histogram2 = " << xAxis2->GetBinLowEdge(iBin) << ".." << xAxis2->GetBinUpEdge(iBin) << "" << std::endl;
	//assert(0);
     // }
    }
  }
}

TH1* divideHistogramByBinWidth(TH1* histogram)
{
  std::string histogramDensityName = Form("%s_density", histogram->GetName());
  TH1* histogramDensity = (TH1*)histogram->Clone(histogramDensityName.data());
  TAxis* xAxis = histogram->GetXaxis();
  int numBins = xAxis->GetNbins();
  for ( int iBin = 1; iBin <= numBins; ++iBin ) {
    double binContent = histogram->GetBinContent(iBin);
    if ( binContent < 0. ) binContent = 0.;
    double binError = histogram->GetBinError(iBin);
    double binWidth = xAxis->GetBinWidth(iBin);
    histogramDensity->SetBinContent(iBin, binContent); ///binWidth
    histogramDensity->SetBinError(iBin, binError); ///binWidth
		if ( binContent <= 0. ) {histogramDensity->SetBinError(iBin, 0.);}
  }
  return histogramDensity;
}

//void addLabel_CMS_luminosity(double x0_cms, double y0, double x0_luminosity)
//{
//  TPaveText* label_cms = new TPaveText(x0_cms, y0 + 0.050, x0_cms + 0.1900, y0 + 0.100, "NDC");
//  label_cms->AddText("CMS");
//  label_cms->SetTextFont(61);
//  label_cms->SetTextAlign(23);
//  label_cms->SetTextSize(0.055);
//  label_cms->SetTextColor(1);
//  label_cms->SetFillStyle(0);
//  label_cms->SetBorderSize(0);
//  label_cms->Draw();
//
//  TPaveText* label_luminosity = new TPaveText(x0_lu(min)osity, y0 + 0.050, x0_luminosity + 0.1900, y0 + 0.100, "NDC");
//  label_luminosity->AddText("12.9 fb^{-1} (13 TeV)");
//  label_luminosity->SetTextAlign(13);
//  label_luminosity->SetTextSize(0.050);
//  label_luminosity->SetTextColor(1);
//  label_luminosity->SetFillStyle(0);
//  label_luminosity->SetBorderSize(0);
//  label_luminosity->Draw();
//}

void addLabel_CMS_preliminary(double x0, double y0, double x0_luminosity)
{
  TPaveText* label_cms = new TPaveText(x0, y0 + 0.0025, x0 + 0.0950, y0 + 0.0600, "NDC");
  label_cms->AddText("CMS");
  label_cms->SetTextFont(61);
  label_cms->SetTextAlign(13);
  label_cms->SetTextSize(0.0575);
  label_cms->SetTextColor(1);
  label_cms->SetFillStyle(0);
  label_cms->SetBorderSize(0);
  label_cms->Draw();

  TPaveText* label_preliminary = new TPaveText(x0 + 0.1050, y0 - 0.0010, x0 + 0.2950, y0 + 0.0500, "NDC");
  label_preliminary->AddText("Preliminary");
  label_preliminary->SetTextFont(52);
  label_preliminary->SetTextAlign(13);
  label_preliminary->SetTextSize(0.050);
  label_preliminary->SetTextColor(1);
  label_preliminary->SetFillStyle(0);
  label_preliminary->SetBorderSize(0);
  label_preliminary->Draw();

  TPaveText* label_luminosity = new TPaveText(x0_luminosity, y0 + 0.0050, x0_luminosity + 0.1900, y0 + 0.0550, "NDC");
  //label_luminosity->AddText("35.9 fb^{-1} (13 TeV)");
  label_luminosity->AddText("41.53 fb^{-1} (13 TeV)");
  label_luminosity->SetTextAlign(13);
  label_luminosity->SetTextSize(0.050);
  label_luminosity->SetTextColor(1);
  label_luminosity->SetFillStyle(0);
  label_luminosity->SetBorderSize(0);
  label_luminosity->Draw();
}


void setStyle_uncertainty(TH1* histogram)
{
  const int color_int = 12;
  const double alpha = 0.40;
  TColor* color = gROOT->GetColor(color_int);
  static int newColor_int = -1;
  static TColor* newColor = 0;
  if ( !newColor ) {
    newColor_int = gROOT->GetListOfColors()->GetSize() + 1;
    newColor = new TColor(newColor_int, color->GetRed(), color->GetGreen(), color->GetBlue(), "", alpha);
  }
  histogram->SetLineColor(newColor_int);
  histogram->SetLineWidth(0);
  histogram->SetFillColor(newColor_int);
  histogram->SetFillStyle(1001);
}

void makePlot(TH1* histogram_data, bool doKeepBlinded,
	      TH1* histogram_ttH,
	      TH1* histogram_ttZ,
	      TH1* histogram_ttW,
	      TH1* histogram_EWK,
	      TH1* histogram_Rares,
	      TH1* histogram_fakes,
	      TH1* histogram_Flips,
	      TH1* histogram_Conv,
	      TH1* histogramSum_mc,
	      TH1* histogramErr_mc,
	      const std::string& xAxisTitle,
	      const std::string& yAxisTitle,
				double yMin, double yMax,
	      bool showLegend,
	      const std::string& label,
				std::string channel,
	      const std::string& outputFileName,
	      bool useLogScale,
				bool hasFlips,
				bool hasConv
			)

{

  ////////////////
	std::cout << "entered functions "<< std::endl;
  TH1* histogram_data_density = 0;
  if ( histogram_data ) {
    histogram_data_density = divideHistogramByBinWidth(histogram_data);
  }
  histogram_data_density->SetMarkerColor(1);
  histogram_data_density->SetMarkerStyle(20);
  histogram_data_density->SetMarkerSize(2);
  histogram_data_density->SetLineColor(1);
  histogram_data_density->SetLineWidth(1);
  histogram_data_density->SetLineStyle(1);
	std::cout << "read first histogram "<< std::endl;

  TH1* histogram_ttH_density = 0;
  if ( histogram_ttH ) {
    if ( histogram_data ) checkCompatibleBinning(histogram_ttH, histogram_data);
    histogram_ttH_density = divideHistogramByBinWidth(histogram_ttH);
  }
  histogram_ttH_density->SetFillColor(628);
  histogram_ttH_density->SetLineColor(1);
  histogram_ttH_density->SetLineWidth(1);

  TH1* histogram_ttZ_density = 0;
  if ( histogram_ttZ ) {
    if ( histogram_data ) checkCompatibleBinning(histogram_ttZ, histogram_data);
    histogram_ttZ_density = divideHistogramByBinWidth(histogram_ttZ);
  }
  histogram_ttZ_density->SetFillColor(822);
  histogram_ttZ_density->SetLineColor(1);
  histogram_ttZ_density->SetLineWidth(1);

  TH1* histogram_ttW_density = 0;
  if ( histogram_ttW ) {
    if ( histogram_data ) checkCompatibleBinning(histogram_ttW, histogram_data);
    histogram_ttW_density = divideHistogramByBinWidth(histogram_ttW);
  }
  histogram_ttW_density->SetFillColor(823);
  histogram_ttW_density->SetLineColor(1);
  histogram_ttW_density->SetLineWidth(1);

  TH1* histogram_EWK_density = 0;
  if ( histogram_EWK ) {
    if ( histogram_data ) checkCompatibleBinning(histogram_EWK, histogram_data);
    histogram_EWK_density = divideHistogramByBinWidth(histogram_EWK);

  }
  histogram_EWK_density->SetFillColor(610);
  histogram_EWK_density->SetLineColor(1);
  histogram_EWK_density->SetLineWidth(1);

  TH1* histogram_Rares_density = 0;
  if ( histogram_Rares ) {
    if ( histogram_data ) checkCompatibleBinning(histogram_Rares, histogram_data);
    histogram_Rares_density = divideHistogramByBinWidth(histogram_Rares);
  }
  histogram_Rares_density->SetFillColor(851);
  histogram_Rares_density->SetLineColor(1);
  histogram_Rares_density->SetLineWidth(1);

  TH1* histogram_fakes_density = 0;
  if ( histogram_fakes ) {
    if ( histogram_data ) checkCompatibleBinning(histogram_fakes, histogram_data);
    histogram_fakes_density = divideHistogramByBinWidth(histogram_fakes);
  }
  histogram_fakes_density->SetFillColor(1);
  histogram_fakes_density->SetFillStyle(3005);
  histogram_fakes_density->SetLineColor(1);
  histogram_fakes_density->SetLineWidth(1);

  std::cout << "test read flips "<< std::endl;
	TH1* histogram_Flips_density = 0;
	if ( hasFlips ) {
	  if ( histogram_Flips ) {
			std::cout << "test read flips "<< std::endl;
	    if ( histogram_data ) checkCompatibleBinning(histogram_Flips, histogram_data);
	    histogram_Flips_density = divideHistogramByBinWidth(histogram_Flips);
	  }
	  histogram_Flips_density->SetFillColor(1);
	  histogram_Flips_density->SetFillStyle(3004);
	  histogram_Flips_density->SetLineColor(1);
	  histogram_Flips_density->SetLineWidth(1);
  }

  std::cout << "test read conversions "<< std::endl;
	TH1* histogram_Conv_density = 0;
	if ( hasConv ) {
	  if ( histogram_Conv) {
	    if ( histogram_data ) checkCompatibleBinning(histogram_Conv, histogram_data);
	    histogram_Conv_density = divideHistogramByBinWidth(histogram_Conv);
	  }
	  histogram_Conv_density->SetFillColor(5);
	  //histogram_Conv_density->SetFillStyle(3007);
	  histogram_Conv_density->SetLineColor(1);
	  histogram_Conv_density->SetLineWidth(1);
  }

  TH1* histogramSum_mc_density = 0;
  if ( histogramSum_mc ) {
    if ( histogram_data ) checkCompatibleBinning(histogramSum_mc, histogram_data);
    histogramSum_mc_density = divideHistogramByBinWidth(histogramSum_mc);
  }
  std::cout << "histogramSum_mc_density test = " << histogramSum_mc_density << std::endl;
  dumpHistogram(histogramSum_mc_density);

  TH1* histogramErr_mc_density = 0;
  if ( histogramErr_mc ) {
    if ( histogram_data ) checkCompatibleBinning(histogramErr_mc, histogram_data);
    histogramErr_mc_density = divideHistogramByBinWidth(histogramErr_mc);
  }
  setStyle_uncertainty(histogramErr_mc_density);

  TCanvas* canvas = new TCanvas("canvas", "canvas", 950, 1100);
  canvas->SetFillColor(10);
  canvas->SetBorderSize(2);
  canvas->Draw();

  TPad* topPad = new TPad("topPad", "topPad", 0.00, 0.34, 1.00, 0.995);
  topPad->SetFillColor(10);
  topPad->SetTopMargin(0.065);
  topPad->SetLeftMargin(0.20);
  topPad->SetBottomMargin(0.00);
  topPad->SetRightMargin(0.04);
  topPad->SetLogy(useLogScale);

  TPad* bottomPad = new TPad("bottomPad", "bottomPad", 0.00, 0.01, 1.00, 0.335);
  bottomPad->SetFillColor(10);
  bottomPad->SetTopMargin(0.085);
  bottomPad->SetLeftMargin(0.20);
  bottomPad->SetBottomMargin(0.35);
  bottomPad->SetRightMargin(0.04);
  bottomPad->SetLogy(false);

  canvas->cd();
  topPad->Draw();
  topPad->cd();

  THStack* histogramStack_mc = new THStack();
  histogramStack_mc->Add(histogram_fakes_density);
	if (hasFlips) histogramStack_mc->Add(histogram_Flips_density);
	if (hasConv) histogramStack_mc->Add(histogram_Conv_density);
  histogramStack_mc->Add(histogram_Rares_density);
  histogramStack_mc->Add(histogram_EWK_density);
  histogramStack_mc->Add(histogram_ttW_density);
  histogramStack_mc->Add(histogram_ttZ_density);
  histogramStack_mc->Add(histogram_ttH_density);

  TH1* histogram_ref = histogram_data_density;
  histogram_ref->SetTitle("");
  histogram_ref->SetStats(false);
  histogram_ref->SetMaximum(yMax);
  histogram_ref->SetMinimum(yMin);

  TAxis* xAxis_top = histogram_ref->GetXaxis();
  assert(xAxis_top);
  if ( xAxisTitle != "" ) xAxis_top->SetTitle(xAxisTitle.data());
  xAxis_top->SetTitleOffset(1.20);
  xAxis_top->SetLabelColor(10);
  xAxis_top->SetTitleColor(10);

  TAxis* yAxis_top = histogram_ref->GetYaxis();
  assert(yAxis_top);
  if ( yAxisTitle != "" ) yAxis_top->SetTitle(yAxisTitle.data());
  yAxis_top->SetTitleOffset(1.20);
  yAxis_top->SetTitleSize(0.080);
  yAxis_top->SetLabelSize(0.065);
  yAxis_top->SetTickLength(0.04);

  histogram_ref->Draw("axis");
  // CV: calling THStack::Draw() causes segmentation violation ?!
  //histogramStack_mc->Draw("histsame");
	std::string testchannel  = "1l_2tau";
	std::string testchannel2 = "2lss_1tau";
  if ( channel == testchannel ) {
	  // CV: draw fakes background on top, as it is by far dominant
	  //     and would prevent the ttH signal and other backgrounds to be seen otherwise !!
		if ( channel == testchannel ) {
	  histogram_fakes_density->Add(histogram_ttH_density);
	  histogram_fakes_density->Add(histogram_ttZ_density);
	  histogram_fakes_density->Add(histogram_ttW_density);
	  histogram_fakes_density->Add(histogram_EWK_density);
	  histogram_fakes_density->Add(histogram_Rares_density);
		if ( hasConv ) histogram_fakes_density->Add(histogram_Conv_density);
		if ( hasFlips ) histogram_fakes_density->Add(histogram_Flips_density);
	  }
		//if ( channel != testchannel ) histogram_fakes_density->Add(histogram_fakes_density);
	  TH1* histogram_fakes_density_cloned = (TH1*)histogram_fakes_density->Clone();
	  histogram_fakes_density_cloned->SetFillColor(10);
	  histogram_fakes_density_cloned->SetFillStyle(1001);
		histogram_fakes_density_cloned->SetMaximum(yMax);
		histogram_fakes_density_cloned->SetMinimum(yMin);
	  histogram_fakes_density_cloned->Draw("histsame");
	  histogram_fakes_density->Draw("histsame");
  } /*else {
		TH1* histogram_fakes_density_cloned = (TH1*)histogram_fakes_density->Clone();
		histogram_fakes_density_cloned->SetFillColor(10);
		histogram_fakes_density_cloned->SetFillStyle(1001);
		histogram_fakes_density_cloned->SetMaximum(yMax);
		histogram_fakes_density_cloned->SetMinimum(yMin);
		histogram_fakes_density_cloned->Draw("histsame");
		histogram_fakes_density->Draw("histsame");
		histogram_fakes_density->Add(histogram_ttH_density);
		histogram_fakes_density->Add(histogram_ttZ_density);
		histogram_fakes_density->Add(histogram_ttW_density);
		histogram_fakes_density->Add(histogram_EWK_density);
		histogram_fakes_density->Add(histogram_Rares_density);

	}*/

// if (histogram_Conv.Integral() > 0)

  std::cout << "histogram_fakes_density = " << histogram_fakes_density << ":" << std::endl;
  //dumpHistogram(histogram_fakes_density);

  histogram_ttH_density->Add(histogram_ttZ_density);
  histogram_ttH_density->Add(histogram_ttW_density);
  histogram_ttH_density->Add(histogram_EWK_density);
  histogram_ttH_density->Add(histogram_Rares_density);
	if ( hasConv ) histogram_ttH_density->Add(histogram_Conv_density);
	if ( hasFlips ) histogram_ttH_density->Add(histogram_Flips_density);
  if ( channel != testchannel ) histogram_ttH_density->Add(histogram_fakes_density);
  histogram_ttH_density->Draw("histsame");


  histogram_ttZ_density->Add(histogram_ttW_density);
  histogram_ttZ_density->Add(histogram_EWK_density);
  histogram_ttZ_density->Add(histogram_Rares_density);
	if ( hasConv ) histogram_ttZ_density->Add(histogram_Conv_density);
	if ( hasFlips ) histogram_ttZ_density->Add(histogram_Flips_density);
  if ( channel != testchannel ) histogram_ttZ_density->Add(histogram_fakes_density);
  histogram_ttZ_density->Draw("histsame");
std::cout << "here 4 " <<  std::endl;

  histogram_ttW_density->Add(histogram_EWK_density);
  histogram_ttW_density->Add(histogram_Rares_density);
	if ( hasConv ) histogram_ttW_density->Add(histogram_Conv_density);
	if ( hasFlips ) histogram_ttW_density->Add(histogram_Flips_density);
  if ( channel != testchannel ) histogram_ttW_density->Add(histogram_fakes_density);
  histogram_ttW_density->Draw("histsame");

  histogram_EWK_density->Add(histogram_Rares_density);
	if ( hasConv ) histogram_EWK_density->Add(histogram_Conv_density);
	if ( hasFlips ) histogram_EWK_density->Add(histogram_Flips_density);
  if ( channel != testchannel ) histogram_EWK_density->Add(histogram_fakes_density);
  histogram_EWK_density->Draw("histsame");

	if ( hasConv ) histogram_Rares_density->Add(histogram_Conv_density);
	if ( hasFlips ) histogram_Rares_density->Add(histogram_Flips_density);
  if ( channel != testchannel ) histogram_Rares_density->Add(histogram_fakes_density);
  histogram_Rares_density->Draw("histsame");

	if ( hasConv ) {
		//histogram_Conv_density->Add(histogram_Conv_density);
		if ( hasFlips ) histogram_Conv_density->Add(histogram_Flips_density);
		if ( channel != testchannel ) histogram_Conv_density->Add(histogram_fakes_density);
	  histogram_Conv_density->Draw("histsame");
  }

	if ( hasFlips ) {
		if ( channel != testchannel ) histogram_Flips_density->Add(histogram_fakes_density);
		TH1* histogram_Flips_density_cloned = (TH1*) histogram_Flips_density->Clone();
	  histogram_Flips_density_cloned->SetFillColor(10);
	  histogram_Flips_density_cloned->SetFillStyle(1001);
	  histogram_Flips_density_cloned->Draw("histsame");
		histogram_Flips_density->Draw("histsame");
	}

  if ( channel != testchannel ) {
		TH1* histogram_fakes_density_cloned = (TH1*)histogram_fakes_density->Clone();
	  histogram_fakes_density_cloned->SetFillColor(10);
	  histogram_fakes_density_cloned->SetFillStyle(1001);
	  histogram_fakes_density_cloned->Draw("histsame");
	  histogram_fakes_density->Draw("histsame");
	}

  if ( histogramErr_mc_density ) {
    histogramErr_mc_density->Draw("e2same");
  }


  if ( !doKeepBlinded ) {
    histogram_data_density->Draw("e1psame");
  }

  histogram_ref->Draw("axissame");

  double legend_y0 = 0.670;
	double legend_y01 = 0.670;
  if ( channel == testchannel2 ) legend_y01 = 0.55;
  if ( showLegend ) {
    TLegend* legend1 = new TLegend(0.2600, legend_y0, 0.5350, 0.9250, NULL, "brNDC");
    legend1->SetFillStyle(0);
    legend1->SetBorderSize(0);
    legend1->SetFillColor(10);
    legend1->SetTextSize(0.050);
		legend1->SetHeader(channel.c_str());
    TH1* histogram_data_forLegend = (TH1*)histogram_data_density->Clone();
    histogram_data_forLegend->SetMarkerSize(2);
    legend1->AddEntry(histogram_data_forLegend, "Observed", "p");
    legend1->AddEntry(histogram_ttH_density, "t#bar{t}H", "f");
    legend1->AddEntry(histogram_ttZ_density, "t#bar{t}Z", "f");
    legend1->AddEntry(histogram_ttW_density, "t#bar{t}W", "f");
    legend1->Draw();
    TLegend* legend2 = new TLegend(0.6600, legend_y01, 0.9350, 0.9250, NULL, "brNDC");
    legend2->SetFillStyle(0);
    legend2->SetBorderSize(0);
    legend2->SetFillColor(10);
        legend2->SetTextSize(0.050);
    legend2->AddEntry(histogram_EWK_density, "Electroweak", "f");
    legend2->AddEntry(histogram_Rares_density, "Rares", "f");
		if ( hasConv ) legend2->AddEntry(histogram_Conv_density, "Conversions", "f");
    legend2->AddEntry(histogram_fakes_density, "Fakes", "f");
		if ( hasFlips ) legend2->AddEntry(histogram_Flips_density, "Flips", "f");
    if ( histogramErr_mc ) legend2->AddEntry(histogramErr_mc_density, "Uncertainty", "f");
    legend2->Draw();
  }

  //addLabel_CMS_luminosity(0.2100, 0.9700, 0.6350);
  addLabel_CMS_preliminary(0.2100, 0.9700, 0.6350);

  TPaveText* label_category = 0;
  if ( showLegend ) label_category = new TPaveText(0.300, legend_y0 - 0.0550, 0.9350, legend_y0, "NDC");
  else label_category = new TPaveText(0.1350, 0.8500, 0.8150, 0.9100, "NDC");
  label_category->SetTextAlign(13);
  label_category->AddText(label.data());
  label_category->SetTextSize(0.055);
  label_category->SetTextColor(1);
  label_category->SetFillStyle(0);
  label_category->SetBorderSize(0);
  label_category->Draw();

  canvas->cd();
  bottomPad->Draw();
  bottomPad->cd();

  TH1* histogramRatio = (TH1*)histogram_data_density->Clone("histogramRatio");
  if ( !histogramRatio->GetSumw2N() ) histogramRatio->Sumw2();
  histogramRatio->SetTitle("");
  histogramRatio->SetStats(false);
  histogramRatio->SetMinimum(-0.99);
  histogramRatio->SetMaximum(+0.99);
  histogramRatio->SetMarkerColor(histogram_data_density->GetMarkerColor());
  histogramRatio->SetMarkerStyle(histogram_data_density->GetMarkerStyle());
  histogramRatio->SetMarkerSize(histogram_data_density->GetMarkerSize());
  histogramRatio->SetLineColor(histogram_data_density->GetLineColor());

  TH1* histogramRatioUncertainty = (TH1*)histogram_data_density->Clone("histogramRatioUncertainty");
  if ( !histogramRatioUncertainty->GetSumw2N() ) histogramRatioUncertainty->Sumw2();
  histogramRatioUncertainty->SetMarkerColor(10);
  histogramRatioUncertainty->SetMarkerSize(0);
  setStyle_uncertainty(histogramRatioUncertainty);

  int numBins_bottom = histogramRatio->GetNbinsX();
  for ( int iBin = 1; iBin <= numBins_bottom; ++iBin ) {
    double binContent_data = histogram_data_density->GetBinContent(iBin);
    double binError_data = histogram_data_density->GetBinError(iBin);
    double binContent_mc = 0;
    double binError_mc = 0;
    if ( histogramSum_mc && histogramErr_mc ) {
      binContent_mc = histogramSum_mc_density->GetBinContent(iBin);
      binError_mc = histogramErr_mc_density->GetBinError(iBin);
    } else {
      TList* histograms = histogramStack_mc->GetHists();
      TIter nextHistogram(histograms);
      double binError2_mc = 0.;
      while ( TH1* histogram_density = dynamic_cast<TH1*>(nextHistogram()) ) {
        binContent_mc += histogram_density->GetBinContent(iBin);
        binError2_mc += square(histogram_density->GetBinError(iBin));
      }
      binError_mc = TMath::Sqrt(binError2_mc);
    }
    if ( binContent_mc > 0. ) {
      histogramRatio->SetBinContent(iBin, binContent_data/binContent_mc - 1.0);
      histogramRatio->SetBinError(iBin, binError_data/binContent_mc);

      histogramRatioUncertainty->SetBinContent(iBin, 0.);
      histogramRatioUncertainty->SetBinError(iBin, binError_mc/binContent_mc);
    } else {
			histogramRatio->SetBinContent(iBin, 0.);
      histogramRatio->SetBinError(iBin, 0.);

      histogramRatioUncertainty->SetBinContent(iBin, 0.);
      histogramRatioUncertainty->SetBinError(iBin, 0.);
		}
  }
  std::cout << "histogramRatio = " << histogramRatio << std::endl;
  dumpHistogram(histogramRatio);
  std::cout << "histogramRatioUncertainty = " << histogramRatioUncertainty << std::endl;
  dumpHistogram(histogramRatioUncertainty);

  std::cout << "passed histRatio " << std::endl;
  TAxis* xAxis_bottom = histogramRatio->GetXaxis();
  assert(xAxis_bottom);
  xAxis_bottom->SetTitle(xAxis_top->GetTitle());
  xAxis_bottom->SetLabelColor(1);
  xAxis_bottom->SetTitleColor(1);
  xAxis_bottom->SetTitleOffset(1.05);
  xAxis_bottom->SetTitleSize(0.16);
  xAxis_bottom->SetTitleFont(xAxis_top->GetTitleFont());
  xAxis_bottom->SetLabelOffset(0.02);
  xAxis_bottom->SetLabelSize(0.12);
  xAxis_bottom->SetTickLength(0.065);
  xAxis_bottom->SetNdivisions(505);

  std::cout << "setX " << std::endl;
  TAxis* yAxis_bottom = histogramRatio->GetYaxis();
  assert(yAxis_bottom);
  yAxis_bottom->SetTitle("#frac{Data - Expectation}{Expectation}");
  yAxis_bottom->SetLabelColor(1);
  yAxis_bottom->SetTitleColor(1);
  yAxis_bottom->SetTitleOffset(0.95);
  yAxis_bottom->SetTitleFont(yAxis_top->GetTitleFont());
  yAxis_bottom->SetNdivisions(505);
  yAxis_bottom->CenterTitle();
  yAxis_bottom->SetTitleSize(0.095);
  yAxis_bottom->SetLabelSize(0.110);
  yAxis_bottom->SetTickLength(0.04);
    std::cout << "setY " << std::endl;

  histogramRatio->Draw("axis");
    std::cout << "draw axis " << std::endl;

  TF1* line = new TF1("line","0", xAxis_bottom->GetXmin(), xAxis_bottom->GetXmax());
  line->SetLineStyle(3);
  line->SetLineWidth(1.5);
  line->SetLineColor(kBlack);
  line->Draw("same");
    std::cout << "draw line " << std::endl;


  histogramRatioUncertainty->Draw("e2same");
    std::cout << "draw histRatio " << std::endl;

  if ( !doKeepBlinded ) {
    histogramRatio->Draw("epsame");
  }

  histogramRatio->Draw("axissame");

  canvas->Update();

  size_t idx = outputFileName.find_last_of('.');
  std::string outputFileName_plot = std::string(outputFileName, 0, idx);
    std::cout << "update canvas " << std::endl;
  if ( useLogScale ) outputFileName_plot.append("_log");
  else outputFileName_plot.append("_linear");
    std::cout << " do log scale" << std::endl;
  outputFileName_plot.append(label.data());
  canvas->Print(std::string(outputFileName_plot).append(".png").data());
  canvas->Print(std::string(outputFileName_plot).append(".pdf").data());

  std::cout << " do canvas" << std::endl;
  TCanvas* canvas2 = new TCanvas("canvas", "canvas", 5500, 5500);
  canvas2->SetFillColor(10);
  canvas2->SetBorderSize(2);
  canvas2->Draw();
  canvas2->cd();
  TH1F *histErr = (TH1F*) histogram_fakes->Clone();
  histErr->SetTitleSize(0.04);
  histErr->SetLabelSize(0.04,"XY");
  //gStyle->SetPaintTextFormat("3.1f");
  histErr->SetMarkerSize(0.8);
  for (unsigned int nbinsx=0 ; nbinsx< histErr->GetNbinsX(); nbinsx++){
    std::cout<<histogram_fakes->GetBinError(nbinsx+1)<<
          " "<<(histogram_ttH_density->GetBinContent(nbinsx+1))/(histogram_fakes->GetBinContent(nbinsx+1)+histogram_ttZ_density->GetBinContent(nbinsx+1)+histogram_ttW_density->GetBinContent(nbinsx+1))<<
					" "<<histogram_fakes->GetBinContent(nbinsx+1)<<
          " "<<histogram_fakes->GetBinError(nbinsx+1)/histogram_fakes->GetBinContent(nbinsx+1)
          << std::endl;
    histErr->SetBinContent(nbinsx+1, histogram_fakes->GetBinError(nbinsx+1)/histogram_fakes->GetBinContent(nbinsx+1));
  }
  histErr->GetYaxis()->SetLimits(0.,1.5),
  histErr->GetYaxis()->SetRangeUser(0.,1.5);
  histErr->GetYaxis()->SetTitle("Err/Content");
  std::cout << " do err/cont" << std::endl;
  histErr->Draw("hist,text");
  std::cout<<"got out "<<std::endl;
  std::cout<<"In last bin "<<histogram_fakes->GetBinError(histogram_fakes->GetNbinsX())<<
        " "<<histogram_fakes->GetBinContent(histogram_fakes->GetNbinsX())<<
        " "<<histogram_fakes->GetBinError(histogram_fakes->GetNbinsX())/histogram_fakes->GetBinContent(histogram_fakes->GetNbinsX())
        << std::endl;
  canvas2->Print(std::string(outputFileName_plot).append("_binErroContent.pdf").data());
  canvas2->Print(std::string(outputFileName_plot).append("_binErroContent.png").data());
  //delete canvas2;
  //delete histErr;

  //delete label_cms;
  //delete topPad;
  //delete label_category;
  //delete histogramRatio;
  //delete histogramRatioUncertainty;
  //delete line;
  //delete bottomPad;
  std::cout<<"deleted "<<std::endl;
}

void makePostFitPlots(
	std::string name ,
	std::string dir,
	std::string channel,
	std::string source,
	bool useLogPlot,
	bool hasFlips,
	bool hasConversions,
	std::string labelX,
	std::string labelVar,
	float minYPlot,
	float maxYPlot,
	bool gentau
)
{
  gROOT->SetBatch(true);
	std::cout << "options passed = " << useLogPlot << " "
	<< hasFlips << " "
	<< labelX << " "
	<< minYPlot << " "
	<< maxYPlot << " "
	 << std::endl;

  TH1::AddDirectory(false);

  std::vector<std::string> categories;
  std::string channelC="ttH_";
  channelC.append(channel.data());
  channelC.append("_prefit");

  categories.push_back(channelC); //"ttH_1l_2tau_prefit");
  //categories.push_back("ttH_1l_2tau_postfit");

  std::string inputFilePath =source.data(); // = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/ttH_htt/";
  std::map<std::string, std::string> inputFileNames; // key = category
  std::string fileI=dir.data();
  fileI.append("/ttH_");
  fileI.append(name);
  fileI.append("_shapes.root");
  inputFileNames[channelC]  = fileI;

  bool doKeepBlinded = false; //true;

  for ( std::vector<std::string>::const_iterator category = categories.begin();
	category != categories.end(); ++category ) {
    std::string inputFileName_full = Form("%s%s", inputFilePath.data(), inputFileNames[*category].data());
    TFile* inputFile = new TFile(inputFileName_full.data());
    if ( !inputFile ) {
      std::cerr << "Failed to open input file = " << inputFileName_full << " !!" << std::endl;
      assert(0);
    }

    TH1* histogram_data = loadHistogram(inputFile, *category, "data_obs", false);
    std::cout << "histogram_data = " << histogram_data << ":" << std::endl;
    if ( !doKeepBlinded ) {
      dumpHistogram(histogram_data);
    }

    TH1* histogram_ttH_htt = loadHistogram(inputFile, *category, "ttH_htt", gentau);
    TH1* histogram_ttH_hww = loadHistogram(inputFile, *category, "ttH_hww", gentau);
    TH1* histogram_ttH_hzz = loadHistogram(inputFile, *category, "ttH_hzz", gentau);
    std::cout << "histogram_ttH: htt = " << histogram_ttH_htt << ", hww = " << histogram_ttH_hww << ", hzz = " << histogram_ttH_hzz << std::endl;
    TString histogramName_ttH = TString(histogram_ttH_htt->GetName()).ReplaceAll("_htt", "_sum");
    TH1* histogram_ttH = (TH1*)histogram_ttH_htt->Clone(histogramName_ttH.Data());
    histogram_ttH->Add(histogram_ttH_hww);
    histogram_ttH->Add(histogram_ttH_hzz);
    makeBinContentsPositive(histogram_ttH);
    std::cout << "histogram_ttH = " << histogram_ttH << ":" << std::endl;
    dumpHistogram(histogram_ttH);

    TH1* histogram_ttZ = loadHistogram(inputFile, *category, "TTZ", gentau);
    std::cout << "histogram_ttZ = " << histogram_ttZ << std::endl;
    makeBinContentsPositive(histogram_ttZ);
    dumpHistogram(histogram_ttZ);

    TH1* histogram_ttW = loadHistogram(inputFile, *category, "TTW", gentau);
    std::cout << "histogram_ttW = " << histogram_ttW << std::endl;
    makeBinContentsPositive(histogram_ttW);
    dumpHistogram(histogram_ttW);

    TH1* histogram_EWK = loadHistogram(inputFile, *category, "EWK", gentau);
    std::cout << "histogram_EWK = " << histogram_EWK << std::endl;
    makeBinContentsPositive(histogram_EWK);
    dumpHistogram(histogram_EWK);

    TH1* histogram_Rares = loadHistogram(inputFile, *category, "Rares", gentau);
    std::cout << "histogram_Rares = " << histogram_Rares << " " << hasFlips<< std::endl;
    makeBinContentsPositive(histogram_Rares);
    dumpHistogram(histogram_Rares);

		TH1* histogram_Flips;
		if (hasFlips) {
		 histogram_Flips = loadHistogram(inputFile, *category, "flips_data", false);
    std::cout << "histogram_Flips = " << histogram_Flips << std::endl;
    makeBinContentsPositive(histogram_Flips);
    dumpHistogram(histogram_Flips);
		}

		TH1* histogram_Conv;
		if (hasConversions) {
		 histogram_Conv = loadHistogram(inputFile, *category, "conversions", false);
    std::cout << "histogram_Conv = " << histogram_Conv << std::endl;
    makeBinContentsPositive(histogram_Conv);
    dumpHistogram(histogram_Conv);
		}

    TH1* histogram_fakes = loadHistogram(inputFile, *category, "fakes_data", false);
    std::cout << "histogram_fakes = " << histogram_fakes << std::endl;
    //TH1* testefakes=getRebinnedHistogram1d(histogram_fakes,  5 ); //Xanda
    //std::cout<<"FAKES REBINNED "<<testefakes->GetBinContent(5)<<" "<<testefakes->Integral() << std::endl;
    makeBinContentsPositive(histogram_fakes);
    dumpHistogram(histogram_fakes);

    TH1* histogramSum_mcBgr = loadHistogram(inputFile, *category, "TotalBkg", false);
    std::cout << "histogramSum_mcBgr = " << histogramSum_mcBgr << std::endl;
    makeBinContentsPositive(histogramSum_mcBgr);
    dumpHistogram(histogramSum_mcBgr);
    TH1* histogramSum_mc = (TH1*)histogramSum_mcBgr->Clone("histogramSum_mc");
    histogramSum_mc->Add(histogram_ttH);
    std::cout << "histogramSum_mc test = " << histogramSum_mc << std::endl;
    TH1* histogramErr_mc = (TH1*)histogramSum_mc->Clone("TotalBkgErr");
		std::cout << "cloned = "<< std::endl;

    std::string outputFilePath = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/ttH_htt/";
    std::string outputFileName = Form("%s/%s/%s_%s.pdf", source.data(),dir.data(), category->data(),name.data());
		//std::string labelY = Form("dN/%s", labelX.c_str());
		std::string labelY = Form("%s", "Events");

    makePlot(histogram_data, doKeepBlinded,
	     histogram_ttH,
	     histogram_ttZ,
	     histogram_ttW,
	     histogram_EWK,
	     histogram_Rares,
	     histogram_fakes,
			 histogram_Flips,
			 histogram_Conv,
	     histogramSum_mc,
	     histogramErr_mc,
	     labelX.c_str(), labelY.c_str(),
			 minYPlot, maxYPlot,
	     true,
	     labelVar.c_str(), //name.data(),
			 channel,
	     outputFileName,
	     useLogPlot,
			 hasFlips, hasConversions
		 );
    std::cout<<"got out function"<<std::endl;

    delete histogram_ttH;
    delete histogramErr_mc;
    delete inputFile;
  }
}
