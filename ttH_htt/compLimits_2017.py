import os, subprocess, sys

#...... please comment out according to your needs ........

#add_shape_sys = "false"
add_shape_sys = "true"
#rebinned_hist = "false"
rebinned_hist = "true"

channels = [
  #"1l_2tau",
  #"2lss_1tau",
  #"2los_1tau", 
  "2l_2tau", 
  #"2los_2tau", 
  #"2lss_2tau"
  #"3l_1tau",
]
shapeVariables = {
  "EventCounter",
  "mTauTauVis",
  "mvaDiscr_2l_2tau",
  "mvaOutput_noHTT_1B_VT",
  "mvaOutput_noHTT_SUM_VT",
}
workingDir = os.getcwd()
#..... If you don't have files in datacards ......
#datacardDir = "/home/sbhowmik/ttHAnalysis/2016/2017Dec18_VTight/datacards/"
#datacardFiles = {
#  "2l_2tau"   : "2l_2tau/prepareDatacards_2l_2tau_mTauTauVis.root",
#}
def run_cmd(command):
  print "executing command = '%s'" % command
  p = subprocess.Popen(command, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
  stdout, stderr = p.communicate()
  return stdout

for channel in channels:
  for shapeVariable in shapeVariables:
    ##datacardFile_input = os.path.join(datacardDir, datacardFiles[channel])
    datacardFile_input = os.path.join(workingDir, "datacards", "prepareDatacards_%s_%s.root" % (channel, shapeVariable))
    WriteDatacard_executable = os.path.join("WriteDatacards_%s" %channel)
    datacardFile_output = os.path.join(workingDir, "limits", "ttH_%s_%s.root" % (channel, shapeVariable))
    run_cmd('rm %s' % datacardFile_output)
    run_cmd('%s --input_file=%s --output_file=%s --add_shape_sys=%s --rebinned_hist=%s' % (WriteDatacard_executable, datacardFile_input, datacardFile_output, add_shape_sys, rebinned_hist))
    txtFile = datacardFile_output.replace(".root", ".txt")
    logFile = datacardFile_output.replace(".root", "_limits.log")
    run_cmd('rm %s' % logFile)
    run_cmd('combine -M AsymptoticLimits -t -1 -m 125 %s &> %s' % (txtFile, logFile))
    #run_cmd('combine -M AsymptoticLimits -m 125 %s &> %s' % (txtFile, logFile))
    run_cmd('combine -M MaxLikelihoodFit -t -1 -m 125 %s' % txtFile)
    #run_cmd('combine -M MaxLikelihoodFit -m 125 %s' % txtFile) 
    rootFile =datacardFile_output.replace(".root", "_shapes.root")    
    run_cmd('rm %s' % rootFile)
    run_cmd('PostFitShapes -d %s -o %s -m 125 -f fitDiagnostics.root:fit_s --postfit --sampling --print' % (txtFile, rootFile))
    makePostFitPlots_macro = os.path.join(workingDir, "macros", "makePostFitPlots_%s_%s.C" %(channel,shapeVariable))
    run_cmd('root -b -n -q -l %s++' % makePostFitPlots_macro)

