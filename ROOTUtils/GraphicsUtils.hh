#ifndef GRAPHICSUTILS_HH
#define GRAPHICSUTILS_HH

#include <math.h>
#include <TVirtualPad.h>
#include <TPolyLine.h>
    
#include <vector>
#include <TH1.h>
#include <cfloat>

using std::vector;
using std::string;

/// axis directions
enum AxisDirection {
    X_DIRECTION = 0,
    Y_DIRECTION = 1,
    Z_DIRECTION = 2,
    T_DIRECTION = 3
};
/// iteration to next axis
inline AxisDirection& operator++(AxisDirection& d) { return d = AxisDirection(d+1); }

/// draw several histograms simultaneously; return max histogram height
double drawSimulHistos(vector<TH1*>& hists, const string& opt = "", const string& newTitle = "DEFAULT");
/// draw a pair of histograms (red and blue by default)
void drawHistoPair(TH1* hRed, TH1* hBlue, const string& opt = "", Int_t c1 = 2, Int_t c2 = 4);
/// draw a pair of histograms, with line and diamond markers
void drawDataMCPair(TH1* dat, TH1* mc);

/// draw vertical line marker
void drawVLine(Float_t x, TVirtualPad* C, Int_t color = 4, Int_t style = 1);
/// draw horizontal line
void drawHLine(Float_t y, TVirtualPad* C, Int_t color = 4);
/// draw circle
void drawCircle(float r, Int_t color = 1, Int_t lstyle = 1, float x0=0, float y0=0);
/// draw shaded rectangle marker
void drawExcludedRegion(Float_t x0, Float_t x1, TCanvas* C, Int_t color = 4, Int_t fill = 1001);

/// make ellipse from center and inverse covariance matrix arrays (for Matrix/Clustering.hh)
TPolyLine* makeEllipse(float x0, float y0, const double* iSigma);

/// set up grayscale figures color palette, black-to-white by default, or inverted
void makeGrayscalepalette(bool b2w = true);
/// set up blue-white-red palette
void makeRBpalette();

#endif
