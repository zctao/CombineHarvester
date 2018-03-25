#!/usr/bin/env python
import os, subprocess, sys
from array import array
from ROOT import *
from math import sqrt, sin, cos, tan, exp
import numpy as np
execfile("python/data_manager.py")
workingDir = os.getcwd()

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt

# cd /home/acaan/VHbbNtuples_8_0_x/CMSSW_7_4_7/src/ ; cmsenv ; cd -
# python compLimits_2016_optXGB.py --channel "2l_2tau" --user "acaan" --version "XGB_frozen_23Mar2018"
from optparse import OptionParser
parser = OptionParser()
parser.add_option("--channel ", type="string", dest="channel", help="The ones whose are implemented now are:\n - 1l_2tau\n - 2lss_1tau\n", default="2lss_1tau")
parser.add_option("--version ", type="string", dest="version", help="As in tthAnalyse...", default="2lss_1tau")
parser.add_option("--user ", type="string", dest="user", help="The user that ran the datacards", default="acaan")
parser.add_option("--addShapeSyst", action="store_true", dest="addShapeSyst", help="If you call this will consider shape systematics", default=False)
parser.add_option("--doPlots", action="store_true", dest="doPlots", help="If you call this will do prefit plots", default=False)
parser.add_option("--doGoF", action="store_true", dest="doGoF", help="Make GoodnessOfFit, only for the last item of the list of shapes", default=False)
parser.add_option("--doImpact", action="store_true", dest="doImpact", help="Make impact os shape stat, only for the last item of the list of shapes", default=False)
(options, args) = parser.parse_args()

# do GoF
# do Impacts

user=options.user
year="2016"
channel=options.channel
version=options.version
addShapeSyst = options.addShapeSyst

# where the original datacards are
mom="/home/"+user+"/ttHAnalysis/"+year+"/"+version+"/datacards/"+channel
if addShapeSyst : shapeSyst = "true"
else : shapeSyst = "false"

if channel == "2lss_1tau" :
  shapesToDo=[
    "memOutput_LR",
    "mvaOutput_2lss_1tau_plainKin_tt",
    "mvaOutput_2lss_1tau_plainKin_1B_M",
    "mvaOutput_2lss_1tau_plainKin_SUM_M",
    "mvaOutput_2lss_1tau_HTT_SUM_M",
    "mvaOutput_2lss_1tau_HTTMEM_SUM_M"
  ]
  binType=[
    "regular",
    "quantileInFakes",
    "quantileInFakes",
    "quantileInFakes",
    "quantileInFakes",
    "quantileInFakes"
  ]
  Nbin=[10, 15, 11, 11, 11, 15]
  useLogPlot="false"
  hasFlips="true"
  minYPlot=[0.1, 0.1, 0.1, 0.1, 0.1]
  maxYPlot=[100, 100, 100, 100, 100]
  labelX=[
  'MEM', 'BDT-tt ', 'Joint-BDT',
  'BDT','BDT','BDT'
  ]
  labelVar=[
  '', 'plainKin', 'plainKin',
  'plainKin','plainKin + HTT','plainKin + HTT + MEM'
  ]

if channel == "1l_2tau" :
  shapesToDo=[
    "mTauTauVis",
    "mvaOutput_plainKin_tt",
    "mvaOutput_HTT_SUM_VT"
  ]
  binType=["regular", "regular", "regular"]
  Nbin=[5, 6, 7]
  useLogPlot="true"
  hasFlips="false"
  minYPlot=[]
  maxYPlot=[]
  minYPlot=[0.1, 0.1, 0.1, 0.1, 0.1]
  maxYPlot=[1000, 1000, 1000, 1000, 1000]
  labelX=['M(#tau#tau) (GeV)', 'BDT-tt (plainKin)', 'Joint-BDT (plainKin)','BDT (plainKin+HTT)']
  labelVar=['', 'plainKin', 'plainKin','plainKin + HTT']

if channel == "2l_2tau" :
  shapesToDo= [
    "mTauTauVis",
    "mvaOutput_plainKin_tt",
    "mvaOutput_plainKin_ttV",
    "mvaOutput_plainKin_1B_VT",
    "mvaOutput_plainKin_SUM_VT"
  ]
  binType=[
    "regular",
    "quantileInFakes",
    "quantileInFakes",
    "quantileInFakes",
    "quantileInFakes"
  ]
  Nbin=[4, 4, 4, 4, 4]
  useLogPlot='true'
  hasFlips='false'
  minYPlot=[0.01, 0.01, 0.01, 0.01, 0.01]
  maxYPlot=[100, 100, 100, 100, 100]
  labelX=['M(#tau#tau) (GeV)', 'BDT-tt', 'BDT-ttV','Joint-BDT', 'BDT']
  labelVar=['', '', '', '', '']

if channel == "3l_1tau" :
  shapesToDo= [
    "mvaDiscr_3l",
    "mvaOutput_plainKin_tt",
    "mvaOutput_plainKin_ttV",
    "mvaOutput_plainKin_1B_M",
    "mvaOutput_plainKin_SUM_M"
  ]
  binType=[
    "no",
    "quantileInAll",
    "quantileInAll",
    "quantileInAll",
    "quantileInAll"
  ]
  Nbin=[5, 6, 6, 6, 6]
  useLogPlot='false'
  hasFlips='false'
  maxYPlot=[5., 25., 25., 35., 35. ]
  minYPlot=[0., 0., 0., 0., 0.]
  labelX=['MVA-3l', 'BDT-tt', 'BDT-ttV','Joint-BDT','BDT']
  labelVar=['', '', '', '', '']

# to save rebined datacards and combine results
import shutil,subprocess
proc=subprocess.Popen(["mkdir "+version],shell=True,stdout=subprocess.PIPE)
out = proc.stdout.read()
proc=subprocess.Popen(["mkdir "+version+"/"+channel],shell=True,stdout=subprocess.PIPE)
out = proc.stdout.read()
local = version+"/"+channel

# file to run to get pre-fit shapes
file = open(version+"/execute_plots_"+options.channel+".sh","w")
file.write("#!/bin/bash\n")
tableOfLimits = open(version+"/tableOfLimits_"+options.channel+"_shpeSys"+shapeSyst+".txt","w")
tableOfLimits.write("Expected"+ "\n")
tableOfLimits.write("Variable 2.5\% 16.0\% 50.0\% 84.0\% 97.5\%"+ "\n")
# da table of limits
for ii, shape in enumerate(shapesToDo) :
    run_cmd("cp "+mom+"/prepareDatacards_"+channel+"_"+shape+".root "+local)
    out = proc.stdout.read()
    if binType[ii] != "no" :
        shapeToFit = rebinRegular(
        version+"/"+channel+"/",
        "prepareDatacards_"+channel+"_"+shape,
        Nbin[ii],
        binType[ii]
        )
    else : shapeToFit = "prepareDatacards_"+channel+"_"+shape
    print ("Computing limits ", shapeToFit)
    datacardFile_output = os.path.join(workingDir, local, "ttH_%s.root" % shapeToFit)
    print datacardFile_output
    run_cmd('%s --input_file=%s --output_file=%s --add_shape_sys=%s' % ('WriteDatacards_'+channel, workingDir+"/"+local+"/"+shapeToFit+".root ", datacardFile_output, shapeSyst))
    txtFile = datacardFile_output.replace(".root", ".txt")
    logFile = datacardFile_output.replace(".root", ".log")
    run_cmd('combine -M Asymptotic -m %s -t -1 %s &> %s' % (str(125), txtFile, logFile))
    run_cmd('rv higgsCombineTest.Asymptotic.mH125.root')
    rootFile = os.path.join(workingDir, version+"/"+channel, "ttH_%s_shapes.root" % (shapeToFit))
    run_cmd('PostFitShapes -d %s -o %s -m 125 ' % (txtFile, rootFile))
    ####
    label=shape.replace("mvaOutput_", "")
    makeplots='root -l -b -n -q '+workingDir+'/macros/makePostFitPlots.C++(\\"'+\
        shapeToFit+'\\",\\"'+local+'\\",\\"'+options.channel+'\\",\\"'+workingDir+'/\\",'+\
        useLogPlot+','+ hasFlips+',\\"'+ labelX[ii]+'\\",\\"'+ labelVar[ii]+'\\",'+\
        str(minYPlot[ii])+','+str(maxYPlot[ii])+\
        ')'
    file.write(makeplots+ "\n")
    result_limit=" ".join(str(x) for x in ReadLimits(logFile))
    tableOfLimits.write(shape+" "+str(result_limit)+ "\n")
    ##########################################
    if options.doGoF and ii == len(shapesToDo)-1 :
        proc=subprocess.Popen(["mkdir "+local+"/GoF"],shell=True,stdout=subprocess.PIPE)
        out = proc.stdout.read()
        run_cmd('combineTool.py  -M T2W -i '+local+'/ttH_'+shapeToFit+'.txt')
        run_cmd('combine -M MaxLikelihoodFit -d '+local+'/ttH_'+shapeToFit+'.root -t -1')
        run_cmd('python $CMSSW_BASE/src/HiggsAnalysis/CombinedLimit/test/diffNuisances.py -a mlfit.root -g plots.root')
        run_cmd('combineTool.py -M GoodnessOfFit --algorithm saturated -d '+local+'/ttH_'+shapeToFit+'.root -n .saturated')
        run_cmd('combineTool.py -M GoodnessOfFit --algorithm saturated -d '+local+'/ttH_'+shapeToFit+'.root -n .saturated.toys -t 500 -s 0:4:1 --parallel 5')
        run_cmd('combineTool.py -M CollectGoodnessOfFit --input higgsCombine.saturated.GoodnessOfFit.mH120.root higgsCombine.saturated.toys.GoodnessOfFit.mH120.*.root -o GoF_saturated.json')
        run_cmd('$CMSSW_BASE/src/CombineHarvester/CombineTools/scripts/plotGof.py --statistic saturated --mass 120.0 GoF_saturated.json -o GoF_saturated')
        run_cmd('rm higgsCombine*.root')
        run_cmd('mv mlfit.root '+local+'/GoF')
        run_cmd('mv plots.root '+local+'/GoF')
        run_cmd('mv GoF_saturated* '+local+'/GoF')
    if options.doImpact and ii == len(shapesToDo)-1 :
        if shapeSyst == "false" :
            print "WARNING: you are trying to acces impact of shape systematis without including it on datacard!"
            continue
        if not options.doGoF : run_cmd('combineTool.py  -M T2W -i '+local+'/ttH_'+shapeToFit+'.txt')
        proc=subprocess.Popen(["mkdir "+local+"/impacts"],shell=True,stdout=subprocess.PIPE)
        out = proc.stdout.read()
        run_cmd('combineTool.py -M Impacts -m 125 -d '+local+'/ttH_'+shapeToFit+'.root --expectSignal 1 --allPars --parallel 8 -t -1 --doInitialFit')
        run_cmd('combineTool.py -M Impacts -m 125 -d '+local+'/ttH_'+shapeToFit+'.root --expectSignal 1 --allPars --parallel 8 -t -1 --robustFit 1 --doFits')
        run_cmd('combineTool.py -M Impacts -m 125 -d '+local+'/ttH_'+shapeToFit+'.root -o impacts.json')
        run_cmd('plotImpacts.py -i impacts.json -o  impacts')
        run_cmd('rm higgsCombine*.root')
        run_cmd('mv impacts* '+local+'/GoF')

file.close()
if options.doPlots : run_cmd("bash "+version+"/execute_plots_"+options.channel+".sh")
