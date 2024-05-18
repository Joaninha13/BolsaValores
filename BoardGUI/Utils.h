#pragma once

#include "../bolsa/Utils.h"

#include <windowsx.h>


//typedef struct {
//	HANDLE hMutex, hWnd;
//	RECT dim;			//limites da janela
//	int nCompanys;
//	POINT pos[MAX_ACOES_BOARD]; //posição da letra x y
//	TCHAR letra;
//	BOOL continua;
//
//	//limite superior e inferior e nclientes
//
//	//MemoryShare* shared;
//	//HANDLE hEvent, hMutex, hMap;
//	//BOOL continua;
//	//DWORD numEmpresas;
//
//} DATA;

#define MAX_ACOES_BOARD 10

typedef struct {
    HANDLE hMutex, hWnd;
    RECT dim;
    int nCompanys;
    int scaleMin, scaleMax;
    POINT pos[MAX_ACOES_BOARD];
    TCHAR letra;
    MemoryShare* shared;    
    BOOL continua;
} DATA;
