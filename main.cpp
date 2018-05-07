#ifndef MAIN_H
#define MAIN_H
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "DealBmp.h"
int  main() {
    // char *filepath = "F:/ssstudy/Map Algebra/mmmm.bmp";
    // char *filepath1 = "F:/ssstudy/Map Algebra/mmmm1.bmp";
    // char *outpath = "F:/ssstudy/Map Algebra/nnnn.bmp";
    // char *outpath1 = "F:/ssstudy/Map Algebra/kkk.bmp";//
    // char *outpath2 = "F:/ssstudy/Map Algebra/kkk111.bmp";//平滑
    // char *outpath3 = "F:/ssstudy/Map Algebra/out1.bmp";//分配场 
    // char *outpath4 = "F:/ssstudy/Map Algebra/out2.bmp";//距离场
    // char *middleline = "F:/ssstudy/Map Algebra/middle.bmp";
    // char *buffer = "F:/ssstudy/Map Algebra/buffer.bmp";
    // char *point_test = "F:/ssstudy/Map Algebra/point_test.bmp";
    // char *point_test_fenpei = "F:/ssstudy/Map Algebra/point_test_fenpei.bmp";//test分配场
    // char *point_test_juli = "F:/ssstudy/Map Algebra/point_test_juli.bmp";//test距离场
    //char *outpath5 = "F:/ssstudy/Map Algebra/out3.bmp"
    Bmpmaker *Openfile = new Bmpmaker();
    COctTmp *Ptemplet = new COctTmp();
    // Openfile->BmpReverse("mmmm.bmp", "nnnn.bmp");
    // Openfile->BmpOverlap("mmmm.bmp","nnnn.bmp","kkk.bmp");
    // Openfile->BmpSmooth("nnnn.bmp","kkk111.bmp",3);
    // Openfile->distancetransform("mmmm.bmp", Ptemplet, "out1.bmp", "out2.bmp");
    // Openfile->BMP_MiddleLine("point_test_fenpei.bmp", "point_test_middle.bmp");
    // Openfile->BMP_Buffer("point_test_juli.bmp", "buffer.bmp", 10);
    // Openfile->distancetransform("point_test.bmp", Ptemplet, "point_test_fenpei.bmp", "point_test_juli.bmp");
    // Openfile->BMP_Voronoi("point_test_fenpei.bmp", "point_test_voronoi.bmp");
    Openfile->ScanSrcPtCoors("point_test.bmp", "point_test_pt.txt");
    Openfile->GetTinPtPairs("point_test_fenpei.bmp", "point_test_ptPairs.txt");
    Openfile->LinkPts("point_test.bmp", "point_test_ptPairs.txt", "point_test_pt.txt");
    return 0;
}


#endif // !MAIN_H
