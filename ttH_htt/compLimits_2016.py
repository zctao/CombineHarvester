import os, subprocess, sys

## ---- Add the channel name here as a new string member of the array-- ##
channels = [ "2los_1tau"]

## ---- Add the final observable  name (used in your channel) here ---- ##
shapeVariables = {
  "2los_1tau" : [ "mvaDiscr_2los" ], 
}

## ---- Add the path to your datacard root file name that you have copied to "shapes/" dir. ---- ##
datacardFiles = {
  "2los_1tau" : "KBFI/datacards_2los_1tau.root",
}

## ----- Add the executable name here ---- ####
WriteDatacard_executables = {
  "2los_1tau" : 'WriteDatacards_2los_1tau', 
}

## ------ Add your postfit plot macro here ---- ###
makePostFitPlots_macros = {
  "2los_1tau" : 'makePostFitPlots_2los_1tau.C', 
}    

def run_cmd(command):
  print "executing command = '%s'" % command
  p = subprocess.Popen(command, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
  stdout, stderr = p.communicate()
  return stdout

workingDir = os.getcwd()
datacardDir = "shapes/" ## NEW !

for channel in channels:
  for shapeVariable in shapeVariables[channel]:
    channel_base = None
    for search_string in [ "1l_2tau", "2lss_1tau", "3l_1tau", "2los_1tau", "2l_2tau", "2los_2tau", "2lss_2tau" ]: 
      if channel.find(search_string) != -1:
        channel_base = search_string
        datacard_dir = os.path.join(workingDir, "datacards")
        if not os.path.exists(datacard_dir):
          os.makedirs(datacard_dir)
        if not os.path.exists(os.path.join(datacard_dir, channel)):
          os.makedirs( os.path.join(datacard_dir, channel) )
          os.makedirs( os.path.join(datacard_dir, channel, "logs") )
        postfit_plot_dir = os.path.join(workingDir, "macros", "plots")
        if not os.path.exists(postfit_plot_dir):
          os.makedirs(postfit_plot_dir)
    ##datacardFile_input = os.path.join(datacardDir, channel_base, datacardFiles[channel] % shapeVariable)
    datacardFile_input = os.path.join(datacardDir, datacardFiles[channel])
    datacardFile_path = os.path.join(workingDir, "datacards", channel)
    datacardRootFile_output = os.path.join(datacardFile_path, "ttH_%s.root" % channel)
    datacardTxtFile_output = datacardRootFile_output.replace(".root", ".txt")
    logFile_limits = os.path.join(datacardFile_path, "logs", "ttH_%s_%s_2016_limits.log" % (channel, shapeVariable))
    logFile_mlfit = os.path.join(datacardFile_path, "logs", "ttH_%s_%s_2016_mlfit.log" % (channel, shapeVariable))
    logFile_postfit_shapes = os.path.join(datacardFile_path, "logs", "ttH_%s_%s_2016_postfit_shapes.log" % (channel, shapeVariable))
    run_cmd('rm %s' % datacardRootFile_output)
    run_cmd('rm %s' % datacardTxtFile_output)
    run_cmd('%s --input_file=%s --output_file=%s --add_shape_sys=true' % (WriteDatacard_executables[channel], datacardFile_input, datacardRootFile_output))
    ### run_cmd('cp %s %s' % (datacardRootFile_output, datacardRootFile_output.replace("/datacards/", "/")))
    run_cmd('rm %s' % logFile_mlfit)    
    ##run_cmd('combine -M MaxLikelihoodFit -t -1 -m 125 %s' % txtFile)
    run_cmd('combineTool.py -M MaxLikelihoodFit -m 125 -d %s --there &> %s' % (datacardTxtFile_output, logFile_mlfit))
    run_cmd('rm %s' % logFile_limits)    
    ##run_cmd('combine -M Asymptotic -t -1 -m 125 %s &> %s' % (txtFile, logFile))
    run_cmd('combineTool.py -M Asymptotic -m 125 --there -d %s &> %s' % (datacardTxtFile_output, logFile_limits))
    PostFitShapesRootFile = os.path.join(workingDir, "datacards", channel, "ttH_%s_%s_2016_shapes.root" % (channel_base, shapeVariable))
    run_cmd('rm %s' % logFile_postfit_shapes)    
    run_cmd('rm %s' % PostFitShapesRootFile)
    run_cmd('PostFitShapes -d %s -o %s -m 125 -f %s/mlfit.Test.root:fit_s --postfit --sampling --print &> %s' % (datacardTxtFile_output, PostFitShapesRootFile, datacardFile_path, logFile_postfit_shapes))
    run_cmd('root -b -n -q -l %s++' % os.path.join(workingDir, "macros", makePostFitPlots_macros[channel]))

