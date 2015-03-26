#ifndef LINHISTCOMBO_HH
#define LINHISTCOMBO_HH

#include <vector>
#include <string>
#include <TH1.h>
#include <TF1.h>

using std::string;
using std::vector;

/// Class for fitting with a linear combination of histograms
class LinHistCombo {
public:
    /// constructor
    LinHistCombo(): interpolate(true), myFit(NULL) {}
    /// destructor
    ~LinHistCombo() { if(myFit) delete(myFit); }
    /// add a fit term
    void addTerm(TH1* h) { terms.push_back(h); }
    /// get fitter
    TF1* getFitter();
    /// fit histogram with linear combination of terms
    int Fit(TH1* h, double xmin, double xmax, const std::string& fitopt = "QR");
    /// require coefficients to be non-negative
    void forceNonNegative();
    
    vector<double> coeffs;      ///< fit coefficients
    vector<double> dcoeffs;     ///< fit coefficient errors
    bool interpolate;           ///< whether to interpolate between bins
    
    /// fit evaluation
    double Evaluate(double* x, double* p);
    /// fit evaluation with current coefficients
    double eval(double x) { return Evaluate(&x, coeffs.data()); }

protected:
    TF1* myFit;                         ///< fit function
    vector<TH1*> terms;                 ///< fit terms
    static unsigned int nFitters;       ///< naming counter
};

#endif
