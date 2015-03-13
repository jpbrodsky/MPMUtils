#include "MultiGaus.hh"
#include "SMExcept.hh"

MultiGaus::~MultiGaus() { 
    delete(iguess);
    delete(myTF1);
}

void MultiGaus::setParameter(unsigned int n, double p) {
    smassert(n<3*npks);
    if(n%3==2)
        p = fabs(p);
    iguess[n] = p;
    myTF1->SetParameter(n,p);
}

double MultiGaus::getParameter(unsigned int n) const {
    smassert(n<3*npks);
    return myTF1->GetParameter(n);
}

double MultiGaus::getParError(unsigned int n) const {
    smassert(n<3*npks);
    return myTF1->GetParError(n);
}

float_err MultiGaus::getPar(unsigned int n) const {
    return float_err(getParameter(n),getParError(n));
}

void MultiGaus::fitEstimate(TH1* h, unsigned int n) {
    if(n>=npks) {
        for(unsigned int i=0; i<npks; i++) fitEstimate(h,i);
        return;
    }
    TF1 fGaus("fGausEst", "gaus", iguess[3*n+1]-iguess[3*n+2], iguess[3*n+1]+iguess[3*n+2]);
    h->Fit(&fGaus,"QNR");
    for(int i=0; i<3; i++) iguess[3*n+i] = fGaus.GetParameter(i);
}

void MultiGaus::setCenterSigma(unsigned int n, double c, double s) {
    if(n>=npks) return;
    setParameter(3*n+1, c);
    setParameter(3*n+2, s);
}

TF1* MultiGaus::getFitter() {
    double xmin,xmax;
    xmin = xmax = iguess[1];
    for(unsigned int p=0; p<npks; p++) {
        if(iguess[3*p+1]-nSigma*iguess[3*p+2] < xmin)
            xmin = iguess[3*p+1]-nSigma*iguess[3*p+2];
        if(iguess[3*p+1]+nSigma*iguess[3*p+2] > xmax)
            xmax = iguess[3*p+1]+nSigma*iguess[3*p+2];
        for(unsigned int i=0; i<3; i++)
            myTF1->SetParameter(3*p+i,iguess[3*p+i]);
    }
    myTF1->SetRange(xmin,xmax);
    return myTF1;
}

void MultiGaus::fit(TH1* h, bool draw) {
    for(unsigned int i=0; i<npks; i++)
        iguess[3*i+0] = h->GetBinContent(h->FindBin(iguess[3*i+1]));
    if(draw)
        h->Fit(getFitter(),"QR");
    else
        h->Fit(getFitter(),"QRN");
    
    for(unsigned int i=0; i<3*npks; i++) {
        if(i%3==2)
            iguess[i] = fabs(myTF1->GetParameter(i));
        else
            iguess[i] = myTF1->GetParameter(i);
    }
}

void MultiGaus::display() const {
    for(unsigned int i=0; i<npks; i++) {
        printf("[%u]\tc,s = %g(%g) +- %g(%g)\th = %g(%g)\n", i,
            getParameter(3*i+1), getParError(3*i+1),
            getParameter(3*i+2), getParError(3*i+2),
            getParameter(3*i+0), getParError(3*i+0));
    }
}

void MultiGaus::addCorrelated(unsigned int n, double relCenter, double relHeight, double relWidth) {
    smassert(n<npks);
    corrPeak p;
    p.mainPeak = n;
    p.relCenter = relCenter;
    p.relWidth = relWidth?relWidth:sqrt(relCenter);
    p.relHeight = relHeight;
    corrPeaks.push_back(p);
}


double MultiGaus::operator() (double* x, double* par) { 
    
    bool reject = true;
    for(unsigned int i=0; i<npks; i++) {
        if( iguess[3*i+1] - 1.01*nSigma*iguess[3*i+2] < x[0] && x[0] < iguess[3*i+1] + 1.01*nSigma*iguess[3*i+2] ) {
            reject = false;
            break;
        }
    }
    if(reject) {
        TF1::RejectPoint();
        return 0;
    }
    
    Double_t s = 0;
    for(unsigned int i=0; i<npks; i++)
        s += par[3*i]*exp( -(x[0]-par[3*i+1])*(x[0]-par[3*i+1])/(2*par[3*i+2]*par[3*i+2]) );
    for(vector<corrPeak>::const_iterator it = corrPeaks.begin(); it != corrPeaks.end(); it++) {
        unsigned int i = it->mainPeak;
        s += par[3*i]*it->relHeight*exp( -(x[0]-par[3*i+1]*it->relCenter)*(x[0]-par[3*i+1]*it->relCenter)/(2*par[3*i+2]*par[3*i+2]*it->relWidth) );
    }
    return s;
}

int iterGaus(TH1* h0, TF1* gf, unsigned int nit, float mu, float sigma, float nsigma, float asym) {
    int err = h0->Fit(gf,"Q","",mu-(nsigma-asym)*sigma,mu+(nsigma+asym)*sigma);
    if(!err && !nit)
        return iterGaus(h0,gf,nit-1,gf->GetParameter(1),gf->GetParameter(2),nsigma,asym);
    return err;
}
