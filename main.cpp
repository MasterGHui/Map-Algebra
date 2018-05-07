#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "DealBmp.h"
int  main() {
    char *filepath = "F:/ssstudy/Map Algebra/mmmm.bmp";
    char *filepath1 = "F:/ssstudy/Map Algebra/mmmm1.bmp";
    char *outpath = "F:/ssstudy/Map Algebra/nnnn.bmp";
    char *outpath1 = "F:/ssstudy/Map Algebra/kkk.bmp";//
    char *outpath2 = "F:/ssstudy/Map Algebra/kkk111.bmp";//平滑
    char *outpath3 = "F:/ssstudy/Map Algebra/out1.bmp";//分配场 
    char *outpath4 = "F:/ssstudy/Map Algebra/out2.bmp";//距离场
    char *middleline = "F:/ssstudy/Map Algebra/middle.bmp";
    //char *outpath5 = "F:/ssstudy/Map Algebra/out3.bmp"
    Bmpmaker *Openfile = new Bmpmaker();
    COctTmp *Ptemplet = new COctTmp();
    //Openfile->BmpReverse(filepath, outpath);
    //Openfile->BmpOverlap(filepath,outpath,outpath1);
    //Openfile->BmpSmooth(outpath,outpath2,3);
    //Openfile->distancetransform(filepath, Ptemplet, outpath3, outpath4);
    Openfile->BMP_MiddleLine(outpath3, middleline);
    return 0;
}


#endif // !MAIN_H
