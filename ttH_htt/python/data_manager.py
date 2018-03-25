import ROOT
import array

def run_cmd(command):
  print "executing command = '%s'" % command
  p = subprocess.Popen(command, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
  stdout, stderr = p.communicate()
  return stdout

def finMaxMin(histSource) :
    file = TFile(histSource+".root","READ");
    file.cd()
    hSum = TH1F()
    for keyO in file.GetListOfKeys() :
       obj =  keyO.ReadObj()
       if type(obj) is not TH1F : continue
       hSumDumb = obj.Clone()
       if not hSum.Integral()>0 : hSum=hSumDumb
       else : hSum.Add(hSumDumb)
    return [
    [hSum.GetBinLowEdge(1),  hSum.GetBinCenter(hSum.GetNbinsX())+hSum.GetBinWidth(hSum.GetNbinsX())/2.],
    [hSum.GetBinLowEdge(hSum.FindFirstBinAbove(0.0)),  hSum.GetBinCenter(hSum.FindLastBinAbove (0.0))+hSum.GetBinWidth(hSum.FindLastBinAbove (0.0))/2.]]

def getQuantiles(histoP,ntarget,xmax) :
    histoP.Scale(1./histoP.Integral());
    histoP.GetCumulative()#.Draw();
    histoP.GetXaxis().SetRangeUser(0.,1.)
    histoP.GetYaxis().SetRangeUser(0.,1.)
    histoP.SetMinimum(0.0)
    xq= array.array('d', [0.] * (ntarget+1))
    yq= array.array('d', [0.] * (ntarget+1))
    yqbin= array.array('d', [0.] * (ntarget+1)) # +2 if firsrt is not zero
    for  ii in range(0,ntarget) : xq[ii]=(float(ii)/(ntarget))
    xq[ntarget]=0.999999999
    histoP.GetQuantiles(ntarget,yq,xq)
    line = [None for point in range(ntarget)]
    line2 = [None for point in range(ntarget)]
    for  ii in range(1,ntarget+1) : yqbin[ii]=yq[ii]
    yqbin[ntarget]=xmax # +1 if first is not 0
    #print yqbin
    return yqbin

def rebinRegular(local, histSource, nbin, BINtype) :
    minmax = finMaxMin(local+"/"+histSource)
    # to know the real min and max of the distribution
    xmindef=minmax[1][0]
    xmaxdef=minmax[1][1]
    if BINtype=="ranged" :
        xmin=minmax[1][0]
        xmax=minmax[1][1]
    else :
        xmin=minmax[0][0]
        xmax=minmax[0][1]
    file = TFile(local+"/"+histSource+".root","READ");
    file.cd()
    histograms=[]
    histograms2=[]
    h2 = TH1F()
    hFakes = TH1F()
    hSumAll = TH1F()
    for nkey, keyO in enumerate(file.GetListOfKeys()) :
       obj =  keyO.ReadObj()
       if type(obj) is not TH1F : continue
       h2 = obj.Clone();
       factor=1.
       if  not h2.GetSumw2N() : h2.Sumw2()
       histograms.append(h2.Clone())
       if keyO.GetName() == "fakes_data" : hFakes=obj.Clone()
       if keyO.GetName() == "fakes_data" or keyO.GetName() =="TTZ" or keyO.GetName() =="TTW" or keyO.GetName() =="TTWW" or keyO.GetName() == "EWK" or keyO.GetName() == "tH" or keyO.GetName() == "Rares" :
           hSumDumb2 = obj
           if not hSumAll.Integral()>0 : hSumAll=hSumDumb2.Clone()
           else : hSumAll.Add(hSumDumb2)
    name=histSource+"_"+str(nbin)+"bins_"+BINtype
    fileOut  = TFile(local+"/"+name+".root", "recreate");
    histo = TH1F()
    for nn, histogram in enumerate(histograms) :
        histogramCopy=histogram.Clone()
        nameHisto=histogramCopy.GetName()
        histogram.SetName(histogramCopy.GetName()+"_"+str(nn)+BINtype)
        histogramCopy.SetName(histogramCopy.GetName()+"Copy_"+str(nn)+BINtype)
        if BINtype=="ranged" or BINtype=="regular" :
            histo= TH1F( nameHisto, nameHisto , nbin , xmin , xmax)
        elif "quantile" in BINtype :
            if "Fakes" in BINtype : nbinsQuant=getQuantiles(hFakes,nbin,xmax)
            if "All" in BINtype : nbinsQuant=getQuantiles(hSumAll ,nbin,xmax)
            histo=TH1F(nameHisto, nameHisto , nbin , nbinsQuant) # nbins+1 if first is zero
        else :
            print "not valid bin type"
            return
        histo.Sumw2()
        for place in range(0,histogramCopy.GetNbinsX() + 1) :
            content =      histogramCopy.GetBinContent(place)
            binErrorCopy = histogramCopy.GetBinError(place);
            newbin =       histo.GetXaxis().FindBin(histogramCopy.GetXaxis().GetBinCenter(place))
            binError =     histo.GetBinError(newbin);
            contentNew =   histo.GetBinContent(newbin)
            histo.SetBinContent(newbin, content+contentNew)
            histo.SetBinError(newbin, sqrt(binError*binError+binErrorCopy*binErrorCopy))
        histo.Write()
    fileOut.Write()
    print (local+"/"+name+".root"+" created")
    print ("calculated between: ",xmin,xmax)
    print ("there is MC data between: ",xmindef,xmaxdef)
    return name

def ReadLimits(limits_output):
    f = open(limits_output, 'r+')
    lines = f.readlines() # get all lines as a list (array)
    for line in  lines:
      l = []
      tokens = line.split()
      if "Expected  2.5%"  in line : do2=float(tokens[4])
      if "Expected 16.0%:" in line : do1=float(tokens[4])
      if "Expected 50.0%:" in line : central=float(tokens[4])
      if "Expected 84.0%:" in line : up1=float(tokens[4])
      if "Expected 97.5%:" in line : up2=float(tokens[4])
    return [do2,do1,central,up1,up2]

calculateInt=[
    "tth_htt",
    "tth_hww",
    "tth_hzz",
    "fakes_data",
    "TTW",
    "TTZ",
    "EWK",
    "Rares",
    "data_obs"
  ]
