#pragma once

#include "../bolsa/Utils.h"

#include <windowsx.h>

#define MAX_ACOES_BOARD 10

typedef struct {
    HANDLE hWnd,hEvent, hMutex, hMap;

    RECT dim;
    int nCompanys;
    int scaleMin, scaleMax;
    POINT pos[MAX_ACOES_BOARD];

    MemoryShare* shared;    
    BOOL continua;


} DATA;
