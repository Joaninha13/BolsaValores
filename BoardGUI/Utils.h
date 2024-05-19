#pragma once

#include "../bolsa/Utils.h"

#include <windowsx.h>

typedef struct {
    HANDLE hWnd,hEvent, hMutex, hMap;

    RECT dim;
    int nCompanys;
    int scaleMin, scaleMax;
    POINT pos[MAX_ACOES_BOARD];

    MemoryShare* shared;
    MemoryShare auxShared;

    BOOL continua, teste;


} DATA;
