#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include "resource.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK TrataDialog(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Base");

#define MAX 1
typedef struct {
	TCHAR letra;
	int x, y;
} LETRA_POS;

typedef struct {
	HANDLE hMutex, hWnd;
	RECT dim;			//limites da janela
	INT stepX, stepY;	//velocidade (+/-)
	POINT pos;			//posição da letra x y
	TCHAR letra;
	BOOL continua;
} DATA;

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {

	HWND hWnd;
	MSG lpMsg;
	WNDCLASSEX wcApp;

	wcApp.cbSize = sizeof(WNDCLASSEX);
	wcApp.hInstance = hInst;
	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
	//wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	//wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
	wcApp.hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
	wcApp.hIconSm = LoadIcon(NULL, IDI_SHIELD);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcApp.cbClsExtra = 0;
	wcApp.cbWndExtra = sizeof(DATA*);
	//wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcApp.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(255, 255, 255));

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,
		//TEXT("Exemplo de Janela Principal em C"),
		TEXT("Nova Janela em C"),
		WS_OVERLAPPEDWINDOW,
		//CW_USEDEFAULT,
		50,
		//CW_USEDEFAULT,
		100,
		//CW_USEDEFAULT,
		800,
		//CW_USEDEFAULT,
		600,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL,
		(HINSTANCE)hInst,
		0);


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&lpMsg, NULL, 0, 0) > 0) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}

	return (int)lpMsg.wParam;
}

DWORD WINAPI moveLetra(LPVOID data) {

	DATA* pData = (DATA*)data;

	while (pData->continua) {
		WaitForSingleObject(pData->hMutex, INFINITE);

		if (pData->stepX != 0) {
			pData->pos.x += pData->stepX;
			if (pData->pos.x < 0) { //LIMITE ESQUERDO
				pData->pos.x = 0;
				pData->stepX *= -1;
			}
			else if (pData->pos.x > pData->dim.right - 10) { //LIMITE DIREITO
				pData->pos.x = pData->dim.right - 10;
				pData->stepX *= -1;
			}

		}

		if (pData->stepY != 0) {
			pData->pos.y += pData->stepY;
			if (pData->pos.y < 0) { //LIMITE ESQUERDO
				pData->pos.y = 0;
				pData->stepY *= -1;
			}
			else if (pData->pos.y > pData->dim.bottom - 10) { //LIMITE DIREITO
				pData->pos.y = pData->dim.bottom - 10;
				pData->stepY *= -1;
			}

		}


		ReleaseMutex(pData->hMutex);
		InvalidateRect(pData->hWnd, NULL, TRUE);
		Sleep(1000 / 30);
	}



	return 0;
}


INT_PTR CALLBACK TrataDialog(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	DATA* pData;
	TCHAR str[10] = _T("_");

	switch (messg) {

	case WM_INITDIALOG:
		//INIT DATA.... (edit box, static box)
		pData = (DATA*)GetWindowLongPtr(GetParent(hWnd), 0);
		if (pData != NULL) {
			WaitForSingleObject(pData->hMutex, INFINITE);
			str[0] = pData->letra;
			ReleaseMutex(pData->hMutex);
		}
		SetDlgItemText(hWnd, IDC_EDIT1, str);
		return(INT_PTR)TRUE;

		break;

	case WM_COMMAND:

		switch (wParam) {

		case IDOK:
			//LER INFO, ACTUALIZAR DADOS, ETC

			GetDlgItemText(hWnd, IDC_EDIT1, str, 10);
			SetDlgItemText(hWnd, IDC_STATIC2, str);

			pData = (DATA*)GetWindowLongPtr(GetParent(hWnd), 0);
			if (pData != NULL) {
				WaitForSingleObject(pData->hMutex, INFINITE);
				pData->letra = str[0];
				ReleaseMutex(pData->hMutex);
			}
			EndDialog(hWnd, (INT_PTR)0);
			break;


		case IDCANCEL:
			EndDialog(hWnd, (INT_PTR)0);
			break;

		}

		return (INT_PTR)TRUE;

		break;
	}


	return (INT_PTR)FALSE;
}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	int res;
	HDC hdc;
	PAINTSTRUCT ps;
	TCHAR letra;
	DATA* pData = (DATA*)GetWindowLongPtr(hWnd, 0);
	static HANDLE hThread;

	switch (messg) {
	case WM_CREATE:

		pData = (DATA*)malloc(sizeof(DATA));

		SetWindowLongPtr(hWnd, 0, (LONG_PTR)pData);


		pData->hMutex = CreateMutex(NULL, FALSE, NULL);
		pData->hWnd = hWnd;
		GetClientRect(hWnd, &pData->dim);		// DIMENSÃO ÁREA DE CLIENTE DA JANELA
		pData->stepX = 2;						//30fps -> 30 * 2 = 60 pixels por segundo
		pData->stepY = 3;
		pData->pos.x = pData->dim.right / 2;
		pData->pos.y = pData->dim.bottom / 2;
		pData->letra = _T('O');
		pData->continua = TRUE;

		hThread = CreateThread(NULL, 0, moveLetra, (LPVOID)pData, 0, NULL);

		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &pData->dim);		// DIMENSÃO ÁREA DE CLIENTE DA JANELA
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);
		//TextOut(hdc, pData->pos.x, pData->pos.y, &pData->letra, 1);
		Rectangle(hdc, pData->pos.x, pData->pos.y, pData->pos.x + 10, pData->pos.y + 20);
		EndPaint(hWnd, &ps);
		break;


	case WM_COMMAND:

		switch (wParam) {

		case ID_FICHEIRO_INIT:
			WaitForSingleObject(pData->hMutex, INFINITE);
			pData->pos.x = pData->dim.right / 2;
			pData->pos.y = pData->dim.bottom / 2;
			ReleaseMutex(pData->hMutex);
			break;

		case ID_FICHEIRO_CONFIG:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, TrataDialog);
			break;

		case ID_FICHEIRO_SAIR:
			DestroyWindow(hWnd);
			break;

		default:
			break;
		}

		break;


	case WM_CHAR:
		letra = LOWORD(wParam);

		switch (letra) {

		case VK_SPACE:
			//INVERTER MOVIMENTO
			WaitForSingleObject(pData->hMutex, INFINITE);

			pData->stepX *= -1;
			pData->stepY *= -1;

			ReleaseMutex(pData->hMutex);
			break;

		case '+':
			//aumenta velocidade
			WaitForSingleObject(pData->hMutex, INFINITE);


			if (pData->stepX > 0)
				pData->stepX++;
			else
				pData->stepX--;


			if (pData->stepY > 0)
				pData->stepY++;
			else
				pData->stepY--;

			ReleaseMutex(pData->hMutex);
			break;

		case '-':
			//diminuir velocidade
			WaitForSingleObject(pData->hMutex, INFINITE);


			if (pData->stepX > 0)
				pData->stepX--;
			else
				pData->stepX++;

			if (pData->stepY > 0)
				pData->stepY--;
			else
				pData->stepY++;

			ReleaseMutex(pData->hMutex);
			break;


		default:
			break;
		}

		if (wParam == VK_SPACE) {
			//INVERTER MOVIMENTO
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else {
			letra = wParam;
		}
		break;


	case WM_SIZE:
		WaitForSingleObject(pData->hMutex, INFINITE);

		GetClientRect(hWnd, &pData->dim);		// DIMENSÃO ÁREA DE CLIENTE DA JANELA
		pData->pos.x = pData->dim.right / 2;
		pData->pos.y = pData->dim.bottom / 2;

		ReleaseMutex(pData->hMutex);
		break;

	case WM_CLOSE:
		res = MessageBox(
			hWnd,
			_T("Pretende mesmo sair?"),
			_T("Confirmação"),
			MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL
		);
		if (res == IDYES) {
			pData->continua = FALSE;
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		CloseHandle(pData->hMutex);
		free(pData);
		PostQuitMessage(0);
		break;

	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}

	return 0;
}