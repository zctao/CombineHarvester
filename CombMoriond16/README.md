# ttH Combination

## Setup

- CMSSW_7_4_7
- HiggsAnalysis/CombinedLimit on branch `74X-root6`
- CombineHarvester on branch `tth-comb-moriond17`
- The `cadi` area of the cmshcg svn should be available somewhere with at least the following up-to-date:
    + HIG-16-038 (Hbb)
    + HIG-16-040 (Hgg)
    + HIG-17-003 (Htautau)
    + HIG-17-004 (multileptons)

## Produce modified datacards

```sh
cd CombineHarvester/CombMoriond16
python scripts/preparettH2017.py -i [PATH TO FOLDER ENCLOSING cadi DIRECTORY]
```

This will produce two sets of cards: `ttH_[CHANNEL].txt` and `ttH_[CHANNEL]_def.txt`, where CHANNEL is one of: comb, tautau, leptons, bb or gg. The first set contain the modifications introduced by the script and second are the result of simply combining the unmodified input cards.

## Make workspaces

```sh
POSTFIX="_def"; for TYPE in comb tautau leptons bb gg; do echo ${TYPE}; text2workspace.py ttH_${TYPE}${POSTFIX}.txt -m 125.0 -o ttH_${TYPE}${POSTFIX}_P.root -P HiggsAnalysis.CombinedLimit.PhysicsModel:floatingXSHiggs --PO modes=ggH,qqH,ZH,WH,ttH; done
```

## Run NLL scans

```sh
POSTFIX="_def"; for TYPE in comb tautau leptons bb gg; do combine -M MultiDimFit ttH_${TYPE}${POSTFIX}_P.root -m 125.0 --algo grid --points 40 --minimizerTolerance 0.1 --minimizerStrategy 0 --redefineSignalPOIs r_ttH --setPhysicsModelParameterRanges r_ttH=-1,4 --setPhysicsModelParameters r_ggH=1,r_qqH=1,r_ZH=1,r_WH=1,r_ttH=1 --freezeNuisances r_ggH,r_qqH,r_ZH,r_WH,MH -n .grid.${TYPE}${POSTFIX}.r_ttH.exp -t -1 | tail -20; done
```

## Make plots
```sh
POSTFIX="_def"; python scripts/plot1DScan.py --no-input-label --logo CMS --logo-sub Internal \
--POI r_ttH --remove-near-min 0.5 -o scan.r_ttH.all --main-label "SM expected" --no-numbers --translate translate.json
-m higgsCombine.grid.comb${POSTFIX}.r_ttH.exp.MultiDimFit.mH125.root --other \
higgsCombine.grid.tautau${POSTFIX}.r_ttH.exp.MultiDimFit.mH125.root:'ttH#rightarrow#tau#tau':2 \
higgsCombine.grid.leptons${POSTFIX}.r_ttH.exp.MultiDimFit.mH125.root:'ttH#rightarrowleptons':4 \
higgsCombine.grid.bb${POSTFIX}.r_ttH.exp.MultiDimFit.mH125.root:'ttH#rightarrowbb':8 \
higgsCombine.grid.gg${POSTFIX}.r_ttH.exp.MultiDimFit.mH125.root:'ttH#rightarrow#gamma#gamma':28
```