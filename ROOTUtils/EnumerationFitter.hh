#ifndef ENUMERATIONFITTER_HH
#define ENUMERATIONFITTER_HH 1

#include <vector>
#include <string>
#include <TF1.h>
#include <TGraphErrors.h>
using std::vector;
using std::string;

class EnumerationFitter {
public:
    /// constructor
    EnumerationFitter(): fitter(NULL) {}
    /// destructor
    ~EnumerationFitter() { if(!fitter) delete fitter; }
    /// add a fit terms set
    void addTerm(const vector<double>& t);
    /// fit evaluation from sum of terms
    double Evaluate(double *x, double *p);
    /// get number of fit parameters
    unsigned int getNParams() const { return fterms.size(); }
    /// get fitter
    TF1* getFitter();
    /// load fittable data and terms from a file
    TGraphErrors* loadFitFile(const string& fname);
    
protected:
    
    vector< vector<double> > fterms;    ///< fit term sets
    TF1* fitter;                        ///< fitter based on these terms
};


#endif
