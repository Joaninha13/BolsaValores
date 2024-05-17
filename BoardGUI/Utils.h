#pragma once

#include "../bolsa/Utils.h"

#include <windowsx.h>


#define MAX 1
typedef struct {
	TCHAR letra;
	int x, y;
} LETRA_POS;

typedef struct {
	HANDLE hMutex, hWnd;
	RECT dim;			//limites da janela
	int nCompanys;
	POINT pos[MAX_ACOES_BOARD]; //posição da letra x y
	TCHAR letra;
	BOOL continua;
} DATA;
