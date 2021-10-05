

#pragma once


class InRange
{
    const double low;
    const double high;
  public:
    InRange (const double l, const double h) : low (l), high (h) { }
    bool operator()(const double t){ return  (t >= low) && t <= high; }
};

class CCalcAutoRate
{

  private:

     uint32_t m_RateSamplesMax;
     uint32_t m_CalcRateSamples;
     double m_AutoAcquisitionRate;
     vector <uint64_t> m_vAutoAcqStore;
     double m_WindowInterval_mS;
     double m_ModeResult;

     void GetAverage( vector <double>& vect );

    public:
      CCalcAutoRate();
      bool AutoRate(void);
      void Initialise(double WindowInterval_mS, uint32_t RateSamplesMax);
      double GetFreq(void);
      double GetModeResult(void);



};

