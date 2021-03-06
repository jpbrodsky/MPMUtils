/// \file OutputManager.cc
/* 
 * OutputManager.cc, part of the MPMUtils package.
 * Copyright (c) 2014 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "OutputManager.hh"
#include "PathUtils.hh"
#include <TH1.h>
#include <TStyle.h>

bool OutputManager::squelchAllPrinting = false;

OutputManager::OutputManager(string nm, string bp):
rootOut(NULL), defaultCanvas(new TCanvas()),
parent(NULL), name(nm) {
    TH1::AddDirectory(kFALSE);
    // set up output canvas
    defaultCanvas->SetCanvasSize(200,200);
#ifdef PUBLICATION_PLOTS
    defaultCanvas->SetGrayscale(true);
#endif    
    basePath = plotPath = dataPath = rootPath = bp;
}

OutputManager::OutputManager(string nm, OutputManager* pnt):
rootOut(NULL), defaultCanvas(NULL), parent(pnt), name(nm) {
    TH1::AddDirectory(kFALSE);
    if(parent) {
        defaultCanvas = parent->defaultCanvas;
        plotPath = dataPath = basePath = rootPath = parent->basePath+"/"+name+"/";
    }
}

void OutputManager::openOutfile() {
    if(rootOut) { rootOut->Close(); }
    makePath(rootPath);
    rootOut = new TFile((rootPath+"/"+name+".root").c_str(),"RECREATE");
    rootOut->cd();
}

void OutputManager::writeROOT() {
    printf("\n--------- Building output .root file... ----------\n");
    if(!rootOut) openOutfile();
    rootOut->cd();
    writeItems();
    clearItems();
    rootOut->Close();
    rootOut = NULL;
    printf("---------          Done.          ----------\n");
}


TH1F* OutputManager::registeredTH1F(string hname, string htitle, unsigned int nbins, float x0, float x1) {
    if(rootOut) rootOut->cd();
    TH1F* h = new TH1F(hname.c_str(),htitle.c_str(),nbins,x0,x1);
    h->GetYaxis()->SetTitleOffset(1.4);
    return (TH1F*)addObject(h);
}

TH2F* OutputManager::registeredTH2F(string hname, string htitle, unsigned int nbinsx, float x0, float x1, unsigned int nbinsy, float y0, float y1) {
    if(rootOut) rootOut->cd();
    return (TH2F*)addObject(new TH2F(hname.c_str(),htitle.c_str(),nbinsx,x0,x1,nbinsy,y0,y1));
}

void OutputManager::printCanvas(string fname, string suffix) const {
    printf("Printing canvas '%s' in '%s'\n",(fname+suffix).c_str(), plotPath.c_str());
    if(squelchAllPrinting) { printf("Printing squelched!\n"); return; }
    makePath(plotPath+"/"+fname+suffix,true);
    defaultCanvas->Print((plotPath+"/"+fname+suffix).c_str());
}

