#!/usr/bin/env python
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('input_card', type=str, help="datacard.txt")
parser.add_argument('-o', '--outdir', default='.', help="Output directory")
parser.add_argument('-d', '--dump', action='store_true',
                    help="Print command instead of executing it")
parser.add_argument('--unblind', action='store_true')
parser.add_argument('--do3D', action='store_true',
                    help="Let r_ttH, r_TTW and r_TTZ float")
parser.add_argument('--limit', action='store_true', help="Do AsymptoticLimits")
parser.add_argument('--fitscan', action='store_true', help="Do MultiDimFit")
parser.add_argument('--impact', action='store_true', help="Do impact")
parser.add_argument('--gof', action='store_true', help="Do goodness of fit")
parser.add_argument('--fitdiagnostics', action='store_true')
parser.add_argument('--significance', action='store_true')
parser.add_argument('--channelComp', action='store_true')
parser.add_argument('--crab', action='store_true')
parser.add_argument('--bsub', action='store_true')

args = parser.parse_args()

if (args.crab or args.bsub):
    assert(args.dump) # for now

cardname = args.input_card.split('/')[-1].replace('.txt','')
cardname = cardname.replace('.root','')

args.outdir=args.outdir.rstrip('/')
#print args.outdir

cwd = os.getcwd()

def run_command(cmd):
    if args.dump:
        print cmd
    else:
        os.system(cmd)

#######################
# text2workspace
workspace=args.input_card
if not '.root' in args.input_card:
    assert('.txt' in args.input_card)
    workspace=cwd+'/'+cardname+'.root'
    t2wcmd='text2workspace.py '+args.input_card+' -o '+workspace
    
    if args.do3D:
        t2wcmd += " -P HiggsAnalysis.CombinedLimit.PhysicsModel:multiSignalModel"
        t2wcmd += " --PO verbose"
        t2wcmd += " --PO 'map=.*/ttH.*:r_ttH[1,-2,5]'"
        t2wcmd += " --PO 'map=.*/TTW:r_ttW[1,-2,5]' --PO 'map=.*/TTWW:r_ttW[1,-2,5]'"
        t2wcmd += " --PO 'map=.*/TTZ:r_ttZ[1,-2,5]'"
        t2wcmd += " --PO 'map=.*/tHq.*:1' --PO 'map=.*/tHW.*:1'"
    run_command(t2wcmd)

if not args.unblind:
    cardname += '_blind'
#print cardname
        
#######################
# Impact
if args.impact:
    run_command('mkdir -p '+args.outdir+'/Impact_'+cardname)
    if not args.dump:
        os.chdir(args.outdir+'/Impact_'+cardname)
    else:
        print "cd", args.outdir+'/Impact_'+cardname
        
    impcmd1 = "combineTool.py -M Impacts -d "+workspace+" -m 125 --parallel 8 --doInitialFit" 
    impcmd2 = "combineTool.py -M Impacts -d "+workspace+" -m 125 --parallel 8 --doFits"

    if args.do3D:
        impcmd1 += " --redefineSignalPOI r_ttH --robustFit 1" #
        impcmd2 += " --redefineSignalPOI r_ttH --robustFit 1" #
    else:
        impcmd1 += " --robustFit 1"
        impcmd2 += " --robustFit 1"
    
    if not args.unblind:
        if args.do3D:
            impcmd1 += " -t -1 --setParameters r_ttH=1,r_ttW=1,r_ttZ=1 "
            impcmd2 += " -t -1 --setParameters r_ttH=1,r_ttW=1,r_ttZ=1 "
        else:
            impcmd1 += " -t -1 --expectSignal 1"
            impcmd2 += " -t -1 --expectSignal 1"         

    if args.crab:
        impcmd2 += " --job-mode crab3 --task-name impact_"+cardname+" --custom-crab custom_crab.py --dry-run"
    if args.bsub: #1nd, 8nh
        impcmd2 += " --job-mode lxbatch --task-name impact_"+cardname+" --sub-opts='-q 8nh' --dry-run"
            
    impcmd3 = "combineTool.py -M Impacts -m 125 -d "+workspace+" -o impacts.json"
    if args.do3D:
        impcmd3 += " --redefineSignalPOI r_ttH"
        
    run_command(impcmd1)
    run_command(impcmd2)
    run_command(impcmd3)

    # plot
    run_command("plotImpacts.py -i impacts.json -o impacts_"+cardname)
    
    os.chdir(cwd)

#######################
# Goodness-of-fit
if args.gof:
    run_command('mkdir -p '+args.outdir+'/GoF_'+cardname)
    if not args.dump:
        os.chdir(args.outdir+'/GoF_'+cardname)
    else:
        print "cd", args.outdir+'/GoF_'+cardname

    gofcmd1 = "combineTool.py -n Obs -M GoodnessOfFit -m 125 --algorithm saturated -d "+workspace
    gofcmd2 = "combineTool.py -n Toy -M GoodnessOfFit -m 125 --algorithm saturated -t 1000 -s 123456 --saveToys --toysFreq -d "+workspace
    if not args.unblind:
        # FIXME
        gofcmd1 += " --fixedSignalStrength=1"
        gofcmd2 += " --fixedSignalStrength=1"

    gofcmd3 = "combineTool.py -M CollectGoodnessOfFit --input higgsCombineObs.GoodnessOfFit.mH125.root higgsCombineToy.GoodnessOfFit.mH125.123456.root -o GoF_saturated.json"

    if args.crab:
        gofcmd2 = gofcmd2.replace("-s 123456", "-s -1")
        gofcmd2 += " --job-mode crab3 --task-name gof_"+cardname+" --custom-crab custom_crab.py --dry-run"
    if args.bsub: #1nd, 8nh
        gofcmd2 += " --job-mode lxbatch --task-name gof_"+cardname+" --sub-opts='-q 8nh' --dry-run"
    
    run_command(gofcmd1)
    run_command(gofcmd2)
    run_command(gofcmd3)

    # plot
    gofplot = "plotGof.py --statistic saturated --mass 125.0 GoF_saturated.json -o GoF_saturated"
    run_command(gofplot)
        
    os.chdir(cwd)

#######################
# Likelihood scan
if args.fitscan:
    if not args.dump:
        os.chdir(args.outdir)
    else:
        print "cd", args.outdir

    logfile=args.outdir+'/fitscan_'+cardname+'.log'
    if not args.dump:
        os.system('touch '+logfile)
    
    fitcmd_obs = "combine -n Obs -M MultiDimFit -m 125 --algo=grid --autoRange 2.5 --squareDistPoiStep "+workspace
    fitcmd_exp = "combine -n Exp -M MultiDimFit -m 125 -t -1 --algo=grid --autoRange 2.5 --squareDistPoiStep "+workspace

    if args.do3D:
        fitcmd_obs += " -P r_ttH --floatOtherPOIs 1 --saveInactivePOI 1"
        fitcmd_exp += " -P r_ttH --floatOtherPOIs 1 --saveInactivePOI 1 --setParameters r_ttH=1,r_ttW=1,r_ttZ=1"
    else:
        fitcmd_exp += " --expectSignal=1"
    
    fitcmd_obs += ' &>> '+logfile
    fitcmd_exp += ' &>> '+logfile
    
    run_command(fitcmd_exp)
    if args.unblind:
        run_command(fitcmd_obs)

    plotscan = "plot1DScan.py higgsCombineExp.MultiDimFit.mH125.root --main-label 'Expected' --main-color 35 --POI r_ttH -o scan_r_ttH"
    if args.unblind:
        plotscan += " --others higgsCombineObs.MultiDimFit.mH125.root:'Observed':1"
    run_command(plotscan)
        
    os.chdir(cwd)

#######################
if args.significance:
    if not args.dump:
        os.chdir(args.outdir)
    else:
        print 'cd', args.outdir

    logfile=args.outdir+'/signif_'+cardname+'.log'
    if not args.dump:
        os.system('touch '+logfile)
        
    sigcmd_obs = "combine -M Significance -n Obs --signif -m 125 "+workspace
    sigcmd_exp = "combine -M Significance -n Exp --signif -m 125 -t -1 --toysFreq "+workspace

    if args.do3D:
        sigcmd_obs += " --redefineSignalPOI r_ttH"
        sigcmd_exp += " --redefineSignalPOI r_ttH --setParameters r_ttH=1,r_ttW=1,r_ttZ=1"
    else:
        sigcmd_exp += " --expectSignal=1"

    sigcmd_obs += ' &>> '+logfile
    sigcmd_exp += ' &>> '+logfile

    if args.unblind:
        run_command(sigcmd_obs)
    else:
        run_command(sigcmd_exp)

    os.chdir(cwd)

#######################
# AsymptoticLimits
if args.limit:
    if not args.dump:
        os.chdir(args.outdir)
    else:
        print "cd", args.outdir
        
    logfile=args.outdir+'/asymptlim_'+cardname+'.log'
    if not args.dump:
        os.system('touch '+logfile)
    
    cmd='combine -M AsymptoticLimits -m 125 -t -1 --run blind' # blind
    if args.unblind:
        cmd='combine -M AsymptoticLimits -m 125'
        
    if args.do3D:
        cmd += ' --redefineSignalPOI r_ttH'
        
    cmd += ' '+workspace+' &>> '+logfile
        
    run_command(cmd)

    os.chdir(cwd)

#######################
if not '.root' in args.input_card and not args.dump:
    os.system('mv '+workspace+' '+args.outdir)

    
#######################
# TODO 
#######################
# FitDiagnostics
if args.fitdiagnostics:
    fdgcmd = "combine -M FitDiagnostics -m 125 "
    if not args.unblind:
        fdgcmd += "-t -1 --expectSignal 1 "
    fdgcmd += workspace

    run_command(fdgcmd)


#######################
