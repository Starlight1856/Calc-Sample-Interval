//**********************************************************************************
//**
//** This class is used to determine the mode ( most common value ) from a series
//** of acquisition samples.
//**
//** The Initialise() method is first called to setup testing parameters, test window and number
//** of test samples.
//** The method AutoRate() is then called on each sample. It measures the time interval
//** between each sample. If the specified number of test samples have not been reached
//** it returns FALSE. When the specified number of samples have been reached the mode
//** value is calculated, and the method returns TRUE. The mode value is then available,
//** GetModeResult returns the mode value in milliseconds, GetFreq in frequency Hz.
//** Any further samples are ignored unless the Initialise() method is called again.
//**
//** All measurements are in milliseconds, to change modify code.
//**
//************************************************************************************

#include <iostream>
#include <time.h>
#include <unistd.h>

using namespace std;
#include <vector>
#include <algorithm>
#include <numeric>

#include "calctime.h"



CCalcAutoRate::CCalcAutoRate()
{
  Initialise(10.0, 51);
}

double CCalcAutoRate::GetFreq(void)
{
  return m_AutoAcquisitionRate;
}

double CCalcAutoRate::GetModeResult(void)
{
 return m_ModeResult;
}

//********************************************************************************
//** Initialise; Initialise test values
//**
//** double WindowInterval_mS; Is the sampling window for mode calculation. If
//**                           0.1 Hz accuracy is required then window should be
//**                           at minium 10mS, smaller values improve the accuracy.
//**
//**
//** uint32_t RateSamplesMax; Is the number of samples to use for mode calculation.
//**                          The size of this value depends on the magnitude of
//**                          jitter present. Larger size gives more accuracy.
//**
//********************************************************************************
void CCalcAutoRate::Initialise(double WindowInterval_mS,
                               uint32_t RateSamplesMax)
{

  m_RateSamplesMax = RateSamplesMax;
  m_CalcRateSamples =0;
  m_AutoAcquisitionRate=0;
  m_ModeResult =0;
  m_WindowInterval_mS = WindowInterval_mS;
  m_vAutoAcqStore.clear(); //** stores acquisition times

}

//*************************************************************************
//** bool AutoRate()
//**
//** Called on each sample. It returns FALSE until the required number
//** of samples have been obtained after which the mode value is calculated
//** and the method returns TRUE. Further samples are then ignored unless
//** Initialise method is called.
//**
//*************************************************************************
bool CCalcAutoRate::AutoRate()
{
  bool found = false;
  timespec time1;
  uint32_t i;


  if( m_CalcRateSamples < m_RateSamplesMax )
    {

    clock_gettime(CLOCK_REALTIME, &time1);

    //** record time in milliseconds
    m_vAutoAcqStore.push_back( (time1.tv_sec * 1000)+ (time1.tv_nsec/1000000) );

    m_CalcRateSamples++;

    }
  else if( m_CalcRateSamples == m_RateSamplesMax ) //** limit test to m_RateSamplesMax number of samples
    {
    //** after m_RateSamplesMax messages determine mode value for more accurate timing of auto acquisition rate
    uint32_t count_num =0;
    double uppertest;
    double range;
    double maxval;
    double minval;
    vector<double>::iterator pos;
    vector<double>::iterator start;

    vector <double> vm ( m_RateSamplesMax-1 );

    //** determine time between samples
    for( i =0; i<m_RateSamplesMax-1; i++ )
      {
      //** double should suffice for range
      vm[i]= (double)(  m_vAutoAcqStore[i+1] - m_vAutoAcqStore[i]); //** in milli sec
      cout << i<< "  "<< vm[i] << "   "  << m_vAutoAcqStore[i] << endl;
      }

    pos = max_element( vm.begin(), vm.end());
    maxval = *pos;

    pos = min_element( vm.begin(), vm.end());
    minval = *pos;

    range = maxval - minval; //** range of measured intervals
    cout <<"max= "<< maxval << "mS "<< "  min= "<< minval << "mS "<< "  Range= "<< range <<  "mS "<<endl;

    if( range <= m_WindowInterval_mS ) //** less window than is unlikely, but if so then just average.
      {
      GetAverage(vm);
      }
    else
      {
      //** just require mode value to the nearest WindowInterval_mS
      vector<uint32_t> modeval;
      vector<uint32_t>::iterator modeint;
      double test_val = minval; //** start from minimum time detected
      uint32_t modeindex;

      while( test_val <= maxval+ m_WindowInterval_mS )
        {
        uppertest =  test_val + m_WindowInterval_mS;

        //** Count how many between range test_val and uppertest
        count_num = (uint32_t)count_if ( vm.begin(), vm.end(), InRange( test_val,  uppertest ) );

        modeval.push_back(count_num);
        test_val = test_val + m_WindowInterval_mS; //** increment test value by m_WindowInterval_mS
        }

      //** find index of mode
      cout << "Mode Index Size = "<< modeval.size() << endl;



      if( modeval.size() > 0 )
        {
        //** max_element() determines which time interval was the most common
        //** distance() determines which at index it is located. As each index
        //** represents a step of 1 window time interval from the minimum time,
        //** the mode result is = minval + ( window_time * modeindex)
        //**
        modeindex = (uint32_t)distance( modeval.begin(), max_element( modeval.begin(), modeval.end() ) );

        int maxe = *max_element( modeval.begin(), modeval.end() );

        cout << "Max Element=" <<  maxe << "   Value= " << modeindex<< endl;

        //** determine mode value from index - values were in 0.1mS steps
        m_ModeResult = minval + ( m_WindowInterval_mS * (double)modeindex ); //** in mS

        m_AutoAcquisitionRate = 1.0 / ( m_ModeResult/1000.0) ;

        }
      else //** condition shouldn't occur but trap any way.
        {
        GetAverage(vm);
        }
      found = true;
      }
    }

 return(found);
}

void CCalcAutoRate::GetAverage( vector <double>& vect )
{
  double avg = accumulate( vect.begin(), vect.end(), 0) / vect.size() ;
  m_ModeResult = avg;
  m_AutoAcquisitionRate = 1.0/(avg/1000.0); //**  average instead; Assumes millesec rate.

}


