#include <string>
#include <map>
#include <set>
#include <iostream>
#include <utility>
#include <vector>
#include <cstdlib>
#include "boost/program_options.hpp"
#include <TString.h>
#include "CombineHarvester/CombineTools/interface/CombineHarvester.h"
#include "CombineHarvester/CombineTools/interface/Observation.h"
#include "CombineHarvester/CombineTools/interface/Process.h"
#include "CombineHarvester/CombineTools/interface/Utilities.h"
#include "CombineHarvester/CombineTools/interface/Systematics.h"
#include "CombineHarvester/CombineTools/interface/BinByBin.h"

using namespace std;
using boost::starts_with;
namespace po = boost::program_options;

int main(int argc, char** argv) {
  bool add_tH = true;
  bool add_TTWW = true;
  bool add_th_shape_sys = false;

  std::string input_file, output_file;
  double lumi = -1.;
  bool add_shape_sys = false;
  bool rebinned_hist = false;
  bool float_sig = false;
  po::variables_map vm;
  po::options_description config("configuration");
  config.add_options()
    ("input_file,i", po::value<string>(&input_file)->default_value("Tallinn/ttH_2l_2tau_2016Jul16_Tight.input.root"))
    ("output_file,o", po::value<string>(&output_file)->default_value("ttH_2l_2tau"))
    ("lumi,l", po::value<double>(&lumi)->default_value(lumi))
    ("add_shape_sys,s", po::value<bool>(&add_shape_sys)->default_value(true))
    ("rebinned_hist,r", po::value<bool>(&rebinned_hist)->default_value(false))
    ("float_sig,f", po::value<bool>(&float_sig)->default_value(false));
  po::store(po::command_line_parser(argc, argv).options(config).run(), vm);
  po::notify(vm);

  //! [part1]
  // First define the location of the "auxiliaries" directory where we can
  // source the input files containing the datacard shapes
  //string aux_shapes = string(getenv("CMSSW_BASE")) + "/src/CombineHarvester/HIG15008/shapes/";
  //string aux_shapes = "/afs/cern.ch/user/v/veelken/public/HIG15008_datacards/";
  //string aux_shapes = "/home/veelken/public/HIG15008_datacards/";
  string aux_shapes = "/home/sbhowmik/ttHAnalysis_171018/2016/2017Oct04/datacards/2l_2tau/";
  if ( input_file.find_first_of("/") == 0 ) aux_shapes = ""; // set aux_shapes directory to zero in case full path to input file is given on command line

  // Create an empty CombineHarvester instance that will hold all of the
  // datacard configuration and histograms etc.
  ch::CombineHarvester cb;
  // Uncomment this next line to see a *lot* of debug information
  // cb.SetVerbosity(3);

  // Here we will just define two categories for an 8TeV analysis. Each entry in
  // the vector below specifies a bin name and corresponding bin_id.
  ch::Categories cats = {
      {1, "ttH_2l_2tau"}
    };
  // ch::Categories is just a typedef of vector<pair<int, string>>
  //! [part1]


  //! [part2]
  vector<string> masses = {"*"};
  //! [part2]

  //! [part3]
  cb.AddObservations({"*"}, {"ttHl"}, {"13TeV"}, {"*"}, cats);
  //! [part3]

  //! [part4]
  std::string proc_fakes = "fakes_data";
  vector<string> bkg_procs = {"TTW", "TTZ", "EWK", "Rares", proc_fakes};
  if (add_tH) bkg_procs.push_back("tH");
  if (add_TTWW) bkg_procs.push_back("TTWW");
  cb.AddProcesses({"*"}, {"*"}, {"13TeV"}, {"*"}, bkg_procs, cats, false);

  vector<string> bkg_procs_MConly = {"TTW", "TTZ", "EWK", "Rares","conversions"};
  if (add_tH) bkg_procs_MConly.push_back("tH");
  if (add_TTWW) bkg_procs_MConly.push_back("TTWW");
  //cb.AddProcesses({"*"}, {"*"}, {"13TeV"}, {"*"}, bkg_procs, cats, false);

  vector<string> sig_procs = {"ttH_hww", "ttH_hzz", "ttH_htt"};
  cb.AddProcesses(masses, {"*"}, {"13TeV"}, {"*"}, sig_procs, cats, true);
  //! [part4]

  //Some of the code for this is in a nested namespace, so
  // we'll make some using declarations first to simplify things a bit.
  using ch::syst::SystMap;
  using ch::syst::SystMapAsymm;
  using ch::syst::SystMapFunc;
  using ch::syst::era;
  using ch::syst::bin_id;
  using ch::syst::process;

  //! [part5]
  cb.cp().signals()
      .AddSyst(cb, "lumi_$ERA", "lnN", SystMap<era>::init
	       ({"13TeV_2017"}, 1.023));
  //! [part5]

  if ( float_sig ) {
    // normalizations floating individually (but ttWW correlated with ttW)
    for ( auto s : {"ttH_htt", "TTW", "TTZ"} ) {
      cb.cp().process({s})
          .AddSyst(cb, Form("scale_%s", s), "rateParam", SystMap<>::init(1.0));
    }
    cb.cp().process({"TTWW"})
      .AddSyst(cb, "scale_TTWW", "rateParam", SystMapFunc<>::init("(@0)", "scale_TTW"));
    for ( auto s : {"ttH_hww", "ttH_hzz"} ) {
      cb.cp().process({s})
          .AddSyst(cb, Form("scale_%s", s), "rateParam", SystMapFunc<>::init("(@0)", "scale_ttH_htt"));
    }
  }

  //! [part6]
  cb.cp().process(sig_procs)
      .AddSyst(cb, "pdf_Higgs_ttH", "lnN", SystMap<>::init(1.036));
  cb.cp().process(sig_procs)
    .AddSyst(cb, "QCDscale_ttH", "lnN", SystMapAsymm<>::init(0.915, 1.058));
  cb.cp().process(sig_procs)
      .AddSyst(cb, "BR_hbb", "lnN", SystMap<>::init(1.0126));
  // in this analysis un-splitted ttH sample is TTHnobb
  cb.cp().process({"ttH_hww"})
      .AddSyst(cb, "BR_hvv", "lnN", SystMap<>::init(1.0154));
  cb.cp().process({"ttH_hzz"})
      .AddSyst(cb, "BR_hzz", "lnN", SystMap<>::init(1.0154));
  cb.cp().process({"ttH_htt"})
      .AddSyst(cb, "BR_htt", "lnN", SystMap<>::init(1.0165));
  // Xanda : check if the bellow needs to be renamed https://github.com/peruzzim/cmgtools-lite/blob/94X_dev_ttH/TTHAnalysis/python/plotter/ttH-multilepton/systsUnc.txt#L98-L104
  if ( add_shape_sys && add_th_shape_sys ) {
    cb.cp().process(sig_procs)
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttH_x1", "shape", SystMap<>::init(1.0));
    cb.cp().process(sig_procs)
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttH_y1", "shape", SystMap<>::init(1.0));
  }

  // We do not separate the tH components as here
  // https://github.com/peruzzim/cmgtools-lite/blob/94X_dev_ttH/TTHAnalysis/python/plotter/ttH-multilepton/systsUnc.txt#L83-L84
  // I took the largest syst
  if (add_tH) {
    cb.cp().process({"tH"})
        .AddSyst(cb, "pdf_qg", "lnN", SystMap<>::init(1.027));
    cb.cp().process({"tH"})
      .AddSyst(cb, "QCDscale_tH", "lnN", SystMapAsymm<>::init(0.939, 1.046));
  }

  cb.cp().process({"TTW"})
      .AddSyst(cb, "pdf_qqbar", "lnN", SystMap<>::init(1.04));
  cb.cp().process({"TTW"})
      .AddSyst(cb, "QCDscale_ttW", "lnN", SystMapAsymm<>::init(0.885, 1.129));
  // Xanda : check if the bellow needs to be renamed https://github.com/peruzzim/cmgtools-lite/blob/94X_dev_ttH/TTHAnalysis/python/plotter/ttH-multilepton/systsUnc.txt#L98-L104
  if ( add_shape_sys  && add_th_shape_sys ) {
    cb.cp().process({"TTW"})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttW_x1", "shape", SystMap<>::init(1.0));
    cb.cp().process({"TTW"})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttW_y1", "shape", SystMap<>::init(1.0));
  }

  if (add_TTWW) {
    cb.cp().process({"TTWW"})
        .AddSyst(cb, "pdf_TTWW", "lnN", SystMap<>::init(1.03));
    cb.cp().process({"TTWW"})
        .AddSyst(cb, "QCDscale_ttWW", "lnN", SystMapAsymm<>::init(0.891, 1.081));
    // Xanda : check if this bellow is needed (and renamed)
    //if ( add_shape_sys && add_th_shape_sys ) {
    //  cb.cp().process({"TTWW_gentau","TTWW_faketau"})
    //    .AddSyst(cb, "CMS_ttHl_thu_shape_ttW_x1", "shape", SystMap<>::init(1.0));
    //  cb.cp().process({"TTWW_gentau","TTWW_faketau"})
    //    .AddSyst(cb, "CMS_ttHl_thu_shape_ttW_y1", "shape", SystMap<>::init(1.0));
    //}
  }

  cb.cp().process({"TTZ"})
      .AddSyst(cb, "pdf_gg", "lnN", SystMap<>::init(0.966));
  cb.cp().process({"TTZ"})
      .AddSyst(cb, "QCDscale_ttZ", "lnN", SystMapAsymm<>::init(0.904, 1.112));
  // Xanda : check if the bellow needs to be renamed https://github.com/peruzzim/cmgtools-lite/blob/94X_dev_ttH/TTHAnalysis/python/plotter/ttH-multilepton/systsUnc.txt#L98-L104
  if ( add_shape_sys  && add_th_shape_sys ) {
    cb.cp().process({"TTZ"})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttZ_x1", "shape", SystMap<>::init(1.0));
    cb.cp().process({"TTZ"})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttZ_y1", "shape", SystMap<>::init(1.0));
  }

  cb.cp().process({"conversions"})
      .AddSyst(cb, "CMS_ttHl_Convs", "lnN", SystMap<>::init(1.5));
  // Xanda: on the datacards we do have thu_shape

  cb.cp().process({"EWK"})
      .AddSyst(cb, "CMS_ttHl_EWK", "lnN", SystMap<>::init(1.5));

  cb.cp().process({"Rares"})
      .AddSyst(cb, "CMS_ttHl_Rares", "lnN", SystMap<>::init(1.5));

  cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl17_fakes", "lnN", SystMap<>::init(1.5));
  if ( add_shape_sys ) {
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_FRjt_norm", "shape", SystMap<>::init(1.0));
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_FRjt_shape", "shape", SystMap<>::init(1.0));
  }

  //cb.cp().process({proc_fakes})
  //  .AddSyst(cb, "CMS_ttHl16_FRe_norm", "lnN", SystMap<>::init(1.25));
  //cb.cp().process({proc_fakes})
  //  .AddSyst(cb, "CMS_ttHl16_FRm_norm", "lnN", SystMap<>::init(1.25));
  cb.cp().process({proc_fakes})
    .AddSyst(cb, "CMS_ttHl17_Clos_e_norm", "lnN", SystMap<>::init(0.95));
  cb.cp().process({proc_fakes})
    .AddSyst(cb, "CMS_ttHl17_Clos_m_norm", "lnN", SystMap<>::init(1.1));

  if ( add_shape_sys && add_th_shape_sys ) {
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttH_x1", "shape", SystMap<>::init(1.0));
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttH_y1", "shape", SystMap<>::init(1.0));
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttW_x1", "shape", SystMap<>::init(1.0));
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttW_y1", "shape", SystMap<>::init(1.0));
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttZ_x1", "shape", SystMap<>::init(1.0));
    cb.cp().process({proc_fakes})
      .AddSyst(cb, "CMS_ttHl_thu_shape_ttZ_y1", "shape", SystMap<>::init(1.0));
  }

  //cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
  //    .AddSyst(cb, "CMS_ttHl_trigger_uncorr", "lnN", SystMap<>::init(1.03));
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .AddSyst(cb, "CMS_ttHl_trigger", "shape", SystMap<>::init(1.0));

  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .AddSyst(cb, "CMS_ttHl_lepEff_elloose", "shape", SystMap<>::init(1.0));
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .AddSyst(cb, "CMS_ttHl_lepEff_muloose", "shape", SystMap<>::init(1.0));
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .AddSyst(cb, "CMS_ttHl_lepEff_eltight", "shape", SystMap<>::init(1.0));
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .AddSyst(cb, "CMS_ttHl_lepEff_mutight", "shape", SystMap<>::init(1.0));
  //cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
  //    .AddSyst(cb, "CMS_ttHl_lepEff_tight", "lnN", SystMap<>::init(1.09));
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .AddSyst(cb, "CMS_ttHl17_tauID", "lnN", SystMap<>::init(1.1));
  if ( add_shape_sys ) {
    cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
        .AddSyst(cb, "CMS_ttHl_JES", "shape", SystMap<>::init(1.0));
    cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
        .AddSyst(cb, "CMS_ttHl_tauES", "shape", SystMap<>::init(1.0));
  }

  //cb.cp().process(ch::JoinStr({sig_procs, {"TTW", "TTZ", "Rares"}}))
  //    .AddSyst(cb, "CMS_eff_m", "lnN", SystMap<>::init(1.02));

  if ( add_shape_sys ) {
    for ( auto s : {"HF", "HFStats1", "HFStats2", "LF", "LFStats1", "LFStats2", "cErr1", "cErr2"} ) {
      cb.cp().process(ch::JoinStr({sig_procs, {"TTW", "TTZ", "Rares", "tH"}}))
          .AddSyst(cb, Form("CMS_ttHl_btag_%s", s), "shape", SystMap<>::init(1.0));
    }
  }
  //! [part6]
  if (rebinned_hist){
    cb.cp().backgrounds().ExtractShapes(
	aux_shapes + input_file.data(),
	"ttH_2l_2tau/rebinned/$PROCESS",
	"ttH_2l_2tau/rebinned/$PROCESS_$SYSTEMATIC");
    cb.cp().signals().ExtractShapes(
        aux_shapes + input_file.data(),
	"ttH_2l_2tau/rebinned/$PROCESS",
	"ttH_2l_2tau/rebinned/$PROCESS_$SYSTEMATIC");
  }else{
    cb.cp().backgrounds().ExtractShapes(
	aux_shapes + input_file.data(),
	"x_$PROCESS",
	"x_$PROCESS_$SYSTEMATIC");
    cb.cp().signals().ExtractShapes(
	aux_shapes + input_file.data(),
	"x_$PROCESS",
	"x_$PROCESS_$SYSTEMATIC");
  }
  //! [part7]

  // CV: scale yield of all signal and background processes by lumi/2.3,
  //     with 2.3 corresponding to integrated luminosity of 2015 dataset
  if ( lumi > 0. ) {
    std::cout << "scaling signal and background yields to L=" << lumi << "fb^-1 @ 13 TeV." << std::endl;
    cb.cp().process(ch::JoinStr({sig_procs, {"TTW", "TTZ", "Rares", "tH", proc_fakes}})).ForEachProc([&](ch::Process* proc) {
      proc->set_rate(proc->rate()*lumi/2.3);
    });
  }

  //! [part8]
  cb.cp().SetAutoMCStats(cb, 10);
  //! [part8]

  //! [part9]
  // First we generate a set of bin names:
  set<string> bins = cb.bin_set();
  // This method will produce a set of unique bin names by considering all
  // Observation, Process and Systematic entries in the CombineHarvester
  // instance.

  if ( add_shape_sys ) {
    for ( auto s : {"HFStats1", "HFStats2", "LFStats1", "LFStats2"} ) {
      cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
          .RenameSystematic(cb, Form("CMS_ttHl_btag_%s", s), Form("CMS_ttHl17_btag_%s", s));
    }
    for ( auto s : {"HF", "LF", "cErr1", "cErr2"} ) {
      cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
          .RenameSystematic(cb, Form("CMS_ttHl_btag_%s", s) , Form("CMS_ttHl16_btag_%s", s));
    }

	cb.cp().process({proc_fakes})
		.RenameSystematic(cb, "CMS_ttHl_FRjt_norm", "CMS_ttHl17_FRjt_norm");
	cb.cp().process({proc_fakes})
		.RenameSystematic(cb, "CMS_ttHl_FRjt_shape", "CMS_ttHl17_FRjt_shape");

	cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
		.RenameSystematic(cb, "CMS_ttHl_trigger", "CMS_ttHl17_trigger");
  }
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
      .RenameSystematic(cb, "CMS_ttHl_JES", "CMS_scale_j");
  cb.cp().process(ch::JoinStr({sig_procs, bkg_procs_MConly}))
	  .RenameSystematic(cb, "CMS_ttHl_tauES", "CMS_scale_t");


  // Finally we iterate through bins and write a
  // datacard.
  for (auto b : bins) {
    cout << ">> Writing datacard for bin: " << b
	 << "\n";
      // We need to filter on both the mass and the mass hypothesis,
      // where we must remember to include the "*" mass entry to get
      // all the data and backgrounds.
    //cb.cp().bin({b}).mass({"*"}).WriteDatacard(
    //	b + ".txt", output);
    // it does not work anymore with TString (after update Havester to use SetAutoMCStats)
    cb.cp().bin({b}).mass({"*"}).WriteDatacard(
      output_file + ".txt" , output_file + ".root");
  }
  //! [part9]

}
