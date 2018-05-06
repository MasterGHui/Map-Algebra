#ifndef DEALBMP_H
#define DEALBMP_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <float.h>
static char sht_Offx[13] = {0,-1, 0,1,-2,-1,0,1,2,-1,0,1,0};
static char sht_Offy[13] = {-2,-1,-1,-1,0,0,0,0,0,1,1,1,2};
static float sht_DisTmp[13] = {2,2,1,2,2,1,0,1,2,2,1,2,2};
class CDistanceTemplet
{
public:
  virtual int TmpSize() = 0;
  virtual float GetTmpDis(int i) = 0;
  virtual int GetOffx(int i) = 0;
  virtual int GetOffy(int i) = 0;
};

    class COctTmp : public CDistanceTemplet{
    private:
      char *m_Offx;
      char *m_Offy;
      float *m_TmpDis;
    public:
      COctTmp() {
        m_Offx = sht_Offx;
        m_Offy = sht_Offy;
        m_TmpDis = sht_DisTmp;
      };
      virtual int TmpSize() { return 13; };
      virtual float GetTmpDis(int i) { return m_TmpDis[i]; };
      virtual int GetOffx(int i) { return m_Offx[i]; };
      virtual int GetOffy(int i) { return m_Offy[i]; };
  };

  class Eu5Tmp : public CDistanceTemplet
  {
    private:
      float m_Mtx[25];

    public:
      Eu5Tmp(){
        for (unsigned i = 0; i < 25; i++) {
          float x = GetOffx(i);
          float y = GetOffy(i);
          m_Mtx[i] = sqrt(x = x + y * y);
        }
      };
      virtual int TmpSize() { return 25; };
      virtual float GetTformpDis(int i) { return m_Mtx[i]; };
      virtual int GetOffx(int i) { return i % 5 - 2; };
      virtual int GetOffy(int i) { return i / 5 - 2; };
  };  
class Bmpmaker {
public:
  bool BmpReverse(const char * InputBmpFileName, const char * OutBmpFileName); // 反色
  bool BmpOverlap(const char *InputBmpFileName1, const char *InputBmpFileName2, const char *OutputBmpFilename); //叠置
  bool BmpSmooth(const char *InputBmpFilename, const char *OutputBmpFileName, int ModelWidth); // 平滑
  int distancetransform(char *SrcBmpName, CDistanceTemplet *pTemplet, const char *OutLocname, const char *OutDisname);
};

#endif