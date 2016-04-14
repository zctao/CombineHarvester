Setting up datacards with just the OS and SS high mT regions for et and mt
In contrast to the normal procedure, the OS region will not be converted to a single bin counting experiment, it will stay with full shapes and undergo the normal auto-rebinning procedure.
The signal processes in the signal categories will be transferred into this region
All catgories other than these two will be dropped before the datacards are written

    MorphingMSSMRun2 --output_folder="mssm_os_highmT" -m MH  --postfix="-mtsv" --control_region=1 --auto_rebin=true --validation 1

    combineTool.py -M T2W -o "ggPhi.root" -P CombineHarvester.CombinePdfs.ModelIndependent:floatingMSSMXSHiggs --PO 'modes=ggH' --PO 'ggHRange=0:20' -i output/mssm_os_highmT/* --parallel 4
    combineTool.py -M T2W -o "bbPhi.root" -P CombineHarvester.CombinePdfs.ModelIndependent:floatingMSSMXSHiggs --PO 'modes=bbH' --PO 'bbHRange=0:20' -i output/mssm_os_highmT/* --parallel 4

Now run the limits - we only need to do ggH in the nobtag and bbH in the btag. And for each will do et and mt separately and then combined.

    combineTool.py -m "90,100,110,120,130,140,160,180,200,250,300,350,400,450,500,600,700,800,900,1000,1200,1400,1500,1600,1800,2000,2300,2600,2900,3200" -M Asymptotic --boundlist ../CombinePdfs/scripts/mssm_ggh_boundaries.json  --setPhysicsModelParameters r_ggH=0,r_bbH=0 -d output/mssm_os_highmT/htt*8*/ggPhi.root --there -n ".ggH" --parallel 4
    combineTool.py -m "90,100,110,120,130,140,160,180,200,250,300,350,400,450,500,600,700,800,900,1000,1200,1400,1500,1600,1800,2000,2300,2600,2900,3200" -M Asymptotic --boundlist ../CombinePdfs/scripts/mssm_bbh_boundaries.json  --setPhysicsModelParameters r_ggH=0,r_bbH=0 -d output/mssm_os_highmT/htt*9*/bbPhi.root --there -n ".bbH" --parallel 4

    combineTool.py -M CollectLimits output/mssm_os_highmT/*/higgsCombine.ggH*.root --use-dirs -o "mssm_os_highmT_ggH.json"
    combineTool.py -M CollectLimits output/mssm_os_highmT/*/higgsCombine.bbH*.root --use-dirs -o "mssm_os_highmT_bbH.json"

    for X in mssm_os_highmT_bbH_htt_*.json; do python scripts/plotMSSMLimits.py --logy --logx ${X} --show obs,exp --cms-sub="Internal" -o ${X/.json/} --process "bb#phi"; done
    for X in mssm_os_highmT_ggH_htt_*.json; do python scripts/plotMSSMLimits.py --logy --logx ${X} --show obs,exp --cms-sub="Internal" -o ${X/.json/}; done



    BIN=htt_mt_10_13TeV; DIR=output/mssm_os_highmT/htt_mt_8_13TeV; MASS=700; WSP=ggPhi.root; GGH="1.0"; BBH="1.0"; cd ${DIR}; combine -M MaxLikelihoodFit -d ${WSP} -m ${MASS} --setPhysicsModelParameterRanges r_ggH=-10,100:r_bbH=-10,100 -v 3 --minimizerStrategy 1 --minimizerTolerance 0.01 --skipBOnlyFit; PostFitShapesFromWorkspace -d combined.txt.cmb -w ${WSP} -o shapes.root --print --freeze r_ggH=${GGH},r_bbH=${BBH},MH=${MASS} --postfit --sampling -f mlfit.root:fit_s; cd -; python scripts/postFitPlot.py --file=${DIR}/shapes.root --log_y --log_x --ratio --r_ggH=${GGH} --r_bbH=${BBH} --mPhi=${MASS} --file_dir="${BIN}" --mode postfit --channel_label "#mu#tau_{h}, nobtag, OS high m_{T} region" --ratio_range 0,2; cp ${DIR}/shapes_postfit_logy_logx.pdf postfit_${BIN}.pdf

    BIN=htt_et_10_13TeV; DIR=output/mssm_os_highmT/htt_et_8_13TeV; MASS=700; WSP=ggPhi.root; GGH="1.0"; BBH="1.0"; cd ${DIR}; combine -M MaxLikelihoodFit -d ${WSP} -m ${MASS} --setPhysicsModelParameterRanges r_ggH=-10,100:r_bbH=-10,100 -v 3 --minimizerStrategy 1 --minimizerTolerance 0.01 --skipBOnlyFit; PostFitShapesFromWorkspace -d combined.txt.cmb -w ${WSP} -o shapes.root --print --freeze r_ggH=${GGH},r_bbH=${BBH},MH=${MASS} --postfit --sampling -f mlfit.root:fit_s; cd -; python scripts/postFitPlot.py --file=${DIR}/shapes.root --log_y --log_x --ratio --r_ggH=${GGH} --r_bbH=${BBH} --mPhi=${MASS} --file_dir="${BIN}" --mode postfit --channel_label "e#tau_{h}, nobtag, OS high m_{T} region" --ratio_range 0,2; cp ${DIR}/shapes_postfit_logy_logx.pdf postfit_${BIN}.pdf

    BIN=htt_mt_13_13TeV; DIR=output/mssm_os_highmT/htt_mt_9_13TeV; MASS=200; WSP=bbPhi.root; GGH="1.0"; BBH="1.0"; cd ${DIR}; combine -M MaxLikelihoodFit -d ${WSP} -m ${MASS} --setPhysicsModelParameterRanges r_ggH=-10,100:r_bbH=-10,100 -v 3 --minimizerStrategy 1 --minimizerTolerance 0.01 --skipBOnlyFit; PostFitShapesFromWorkspace -d combined.txt.cmb -w ${WSP} -o shapes.root --print --freeze r_ggH=${GGH},r_bbH=${BBH},MH=${MASS} --postfit --sampling -f mlfit.root:fit_s; cd -; python scripts/postFitPlot.py --file=${DIR}/shapes.root --log_y --log_x --ratio --r_ggH=${GGH} --r_bbH=${BBH} --mPhi=${MASS} --file_dir="${BIN}" --mode postfit --channel_label "#mu#tau_{h}, btag, OS high m_{T} region" --ratio_range 0,2; cp ${DIR}/shapes_postfit_logy_logx.pdf postfit_${BIN}.pdf

    BIN=htt_et_13_13TeV; DIR=output/mssm_os_highmT/htt_et_9_13TeV; MASS=200; WSP=bbPhi.root; GGH="1.0"; BBH="1.0"; cd ${DIR}; combine -M MaxLikelihoodFit -d ${WSP} -m ${MASS} --setPhysicsModelParameterRanges r_ggH=-10,100:r_bbH=-10,100 -v 3 --minimizerStrategy 1 --minimizerTolerance 0.01 --skipBOnlyFit; PostFitShapesFromWorkspace -d combined.txt.cmb -w ${WSP} -o shapes.root --print --freeze r_ggH=${GGH},r_bbH=${BBH},MH=${MASS} --postfit --sampling -f mlfit.root:fit_s; cd -; python scripts/postFitPlot.py --file=${DIR}/shapes.root --log_y --log_x --ratio --r_ggH=${GGH} --r_bbH=${BBH} --mPhi=${MASS} --file_dir="${BIN}" --mode postfit --channel_label "e#tau_{h}, btag, OS high m_{T} region" --ratio_range 0,2; cp ${DIR}/shapes_postfit_logy_logx.pdf postfit_${BIN}.pdf


**Should add pre-fit and post-fit plotting of these control regions and get the G.O.F up and running!**

Other regions that could be attempted:
 - QCD enriched: SS iso not useful if QCD shape was determined there, however in btag shape is from a relaxed b-tagging selection so can be attempted there. Could also try OS anti-iso, with QCD shape from SS anti-iso, and probably W just from MC as the high mT won't be pure enough. For tt would be good to fit SS iso using the data-driven method into the SS anti-iso sideband.
 - TTbar enriched: Would like to get pure ttbar without adjusting the category defintions, but that's only simple in e-mu, just by inverting the pzeta and possibly adding a MET cut? For et and mt one option is just to reverse the n_jets <= 1 cut in the b-tag category. Could then add an nbtag >= 2 if it was necessary
 - Assume there is no need for diboson or Z->ll enriched.