#include "GraphicsUtils.hh"
#include "PathUtils.hh"
#include "StringManip.hh"
#include "SMExcept.hh"
#include <algorithm>

#include <TEllipse.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLine.h>
#include <TLatex.h>
#include <TBox.h>
#include <TStyle.h>
#include <TColor.h>

bool compareHistosByMax(TH1* i, TH1* j) {
    smassert(i && j);
    return i->GetMaximum() < j->GetMaximum();
}

double getXmin(const TH1* h) { smassert(h); return h->GetBinLowEdge(1); }
double getXmax(const TH1* h) { smassert(h); return h->GetBinLowEdge(h->GetNbinsX()+1); }

bool compareHistosByXmin(TH1* i, TH1* j) {
    smassert(i && j);
    return getXmin(i) < getXmin(j);
}
bool compareHistosByXmax(TH1* i, TH1* j) {
    smassert(i && j);
    return getXmax(i) < getXmax(j);
}

double drawSimulHistos(vector<TH1*>& hists, const string& opt, const string& newTitle) {
    if(!hists.size())
        return 0;
    printf("Drawing %i histograms together",(int)hists.size()); fflush(stdout);
    double xmin = getXmin(*std::min_element(hists.begin(),hists.end(),compareHistosByXmin));
    printf(" spanning x=(%g,",xmin); fflush(stdout);
    double xmax = getXmax(*std::max_element(hists.begin(),hists.end(),compareHistosByXmax));
    printf("%g), ",xmax); fflush(stdout);
    TH1* maxHist = *std::max_element(hists.begin(),hists.end(),compareHistosByMax);
    smassert(maxHist);
    printf("with ymax = %g...",maxHist->GetMaximum()); fflush(stdout);
    //maxHist->SetAxisRange(xmin,xmax,"X");
    string oldTitle = maxHist->GetTitle();
    if(newTitle != "DEFAULT")
        maxHist->SetTitle(newTitle.c_str());
    maxHist->Draw(opt.c_str());
    for(vector<TH1*>::iterator it = hists.begin(); it != hists.end(); it++) {
        smassert(*it);
        if(*it == maxHist)
            continue;
        if(opt.size())
            (*it)->Draw((opt+" SAME").c_str());
        else
            (*it)->Draw("SAME");
    }
    printf(" Done.\n");
            
            maxHist->SetTitle(oldTitle.c_str());
            return maxHist->GetMaximum();
}

void drawHistoPair(TH1* hRed, TH1* hBlue, const string& opt, Int_t c1, Int_t c2) {
    smassert(hRed && hBlue);
    hRed->SetLineColor(c1);
    hRed->SetMarkerColor(c1);
    hBlue->SetLineColor(c2);
    hBlue->SetMarkerColor(c2);
    vector<TH1*> hToPlot;
    hToPlot.push_back(hRed);
    hToPlot.push_back(hBlue);
    drawSimulHistos(hToPlot,opt);
}

void drawDataMCPair(TH1* dat, TH1* mc) {
    dat->SetLineColor(1);
    mc->SetMarkerStyle(33);
    mc->SetMarkerColor(1);
    if(dat->GetMaximum() > mc->GetMaximum()) {
        dat->Draw("H E0");
        mc->Draw("P SAME");
    } else {
        mc->Draw("P");
        dat->Draw("H E0 SAME");
    }
}

void combo_draw(const vector<TH1*>& hs, const string& outp, const char* opt) {
    string aName;
    string outpath = outp;
    string nbase = split(outpath,"/").back();
    if(split(nbase,".").back()=="pdf") {
        nbase = dropLast(nbase,".");
        outpath = dropLast(outpath,"/");
    } else nbase = "";
    
    vector<string> hNames;
    for(auto it = hs.begin(); it != hs.end(); it++) {
        if(!(*it)) continue;
        (*it)->Draw(opt);
        char file_template[] = "/tmp/plot_XXXXXX";
        int fd = mkstemp(file_template);
        if(fd==-1) continue; // failure to make temp file
        close(fd);
        hNames.push_back(file_template);
        if(!nbase.size()) nbase = dropLast((*it)->GetName(),"_");
        gPad->Print(file_template,"pdf");
    }
    combo_pdf(hNames, outpath + "/"+nbase+".pdf");
}

void combo_draw(const vector<TH2*>& hs, const string& outpath, const char* opt) {
    vector<TH1*> hs1;
    for(auto it = hs.begin(); it != hs.end(); it++) hs1.push_back(*it);
    combo_draw(hs1, outpath, opt);
}

void drawCircle(float r, Int_t color, Int_t lstyle, float x0, float y0) {
    TEllipse* e = new TEllipse(x0,y0,r,r);
    e->SetFillStyle(0);
    e->SetLineColor(color);
    e->SetLineStyle(lstyle);
    e->Draw();
}

TPolyLine* makeEllipse(float x0, float y0, const double* iSigma) {
    int npts = 50;
    vector<Double_t> xs;
    vector<Double_t> ys;
    for(int i=0; i<npts; i++) {
        double th = i*2*M_PI/(npts-1);
        double c = cos(th);
        double s = sin(th);
        double r = 1./sqrt((iSigma[0]*c+iSigma[2]*s)*c + (iSigma[1]*c+iSigma[3]*s)*s);
        xs.push_back(r*c+x0);
        ys.push_back(r*s+y0);
    }
    return new TPolyLine(xs.size(), &xs[0], &ys[0]);
}

void drawVLine(Float_t x, TVirtualPad* C, Int_t color, Int_t style) {
    Double_t xmin,ymin,xmax,ymax;
    C->Update();
    C->GetRangeAxis(xmin,ymin,xmax,ymax);
    if(C->GetLogy()) {
        ymin = pow(10,ymin);
        ymax = pow(10,ymax);
    }
    TLine* l = new TLine(x,ymin,x,ymax);
    l->SetLineColor(color);
    l->SetLineStyle(style);
    l->Draw();
}

void drawHLine(Float_t y, TVirtualPad* C, Int_t color, Int_t style) {
    Double_t xmin,ymin,xmax,ymax;
    C->Update();
    C->GetRangeAxis(xmin,ymin,xmax,ymax);
    if(C->GetLogx()) {
        xmin = pow(10,xmin);
        xmax = pow(10,xmax);
    }
    TLine* l = new TLine(xmin,y,xmax,y);
    l->SetLineColor(color);
    l->SetLineStyle(style);
    l->Draw();
}

void drawExcludedRegion(Float_t x0, Float_t x1, TCanvas* C, Int_t color, Int_t fill) {
    Double_t xmin,ymin,xmax,ymax;
    C->Update();
    C->GetRangeAxis(xmin,ymin,xmax,ymax);
    if(C->GetLogy()) {
        ymin = pow(10,ymin);
        ymax = pow(10,ymax);
    }
    TBox* r = new TBox(x0,ymin,x1,ymax);
    r->SetFillColor(color);
    r->SetFillStyle(fill);
    r->Draw();
}

void makeGrayscalepalette(bool b2w) {
    const UInt_t nstops = 2;
    const Int_t nb=255;
    const double c0 = !b2w;
    const double c1 = b2w;
    Double_t l[nstops] = { c0, c1 };
    Double_t s[nstops] = { 0., 1. };

    TColor::CreateGradientColorTable(nstops, s, l, l, l, nb);
}

void makeRBpalette() {
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;
    
    Double_t stops[NRGBs] = { 0.00, 0.25, 0.50, 0.75, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 1.00, 0.75, 1.00 };
    Double_t green[NRGBs] = { 0.00, 0.25, 1.00, 0.00, 0.80 };
    Double_t blue[NRGBs]  = { 1.00, 0.50, 1.00, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}
