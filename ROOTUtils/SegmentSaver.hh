/* 
 * SegmentSaver.hh, part of the MPMUtils package.
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

#ifndef SEGMENTSAVER_HH
#define SEGMENTSAVER_HH

#include "OutputManager.hh"
#include <TH1.h>
#include <TFile.h>
#include <map>
#include <string>

using std::map;
using std::string;

/// class for saving, retrieving, and summing histograms from file
class SegmentSaver: public OutputManager {
public:
    /// constructor, optionally with input filename
    SegmentSaver(OutputManager* pnt, const string& nm = "SegmentSaver", const string& inflName = "");
    /// destructor
    virtual ~SegmentSaver();
    /// get location of this analyzer's input file
    const string& getInflName() const { return inflname; }
    /// get age of analyzer's input file
    double getInflAge() const { return inflAge; }
    /// check whether correct input files exist at given location
    static bool inflExists(const string& inflName);
    
    /// generate or restore from file a saved TH1F histogram
    TH1* registerSavedHist(const string& hname, const string& title,unsigned int nbins, float xmin, float xmax);
    /// generate or restore from file a saved histogram from a template
    TH1* registerSavedHist(const string& hname, const TH1& hTemplate);
    
    /// get core histogram by name
    TH1* getSavedHist(const string& hname);
    /// get core histogram by name, const version
    const TH1* getSavedHist(const string& hname) const;
    /// get full histograms listing
    const map<string,TH1*>& getHists() const { return saveHists; }
    /// zero out all saved histograms
    void zeroSavedHists();
    /// scale all saved histograms by a factor
    virtual void scaleData(double s);
    
    /// add histograms from another SegmentSaver of the same type
    virtual void addSegment(const SegmentSaver& S);
    /// check if this is equivalent layout to another SegmentSaver
    virtual bool isEquivalent(const SegmentSaver& S) const;
    
    bool ignoreMissingHistos;   ///< whether to quietly ignore missing histograms in input file
    
    
    // ----- Subclass me! ----- //
    
    /// create a new instance of this object (cloning self settings) for given directory
    virtual SegmentSaver* makeAnalyzer(const string& nm, const string& inflname) = 0;
    /// virtual routine for generating output plots
    virtual void makePlots() {}
    /// virtual routine for generating calculated hists
    virtual void calculateResults() { isCalculated = true; }
    
    TFile* fIn;                 ///< input file to read in histograms from
    string inflname;            ///< where to look for input file
    bool isCalculated;
    
protected:
    
    /// attempt to load histogram from input file
    TH1* tryLoad(const std::string& hname);
    
    map<std::string,TH1*> saveHists;       ///< saved histograms
    double inflAge;                             ///< age of input file [s]; 0 for brand-new files
};

#endif
