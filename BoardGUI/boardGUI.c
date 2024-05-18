#include "Utils.h"
#include "resource.h"

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK TrataDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDialogProc(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("BoardGUI");

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {
    HWND hWnd;
    MSG lpMsg;
    WNDCLASSEX wcApp;

    wcApp.cbSize = sizeof(WNDCLASSEX);
    wcApp.hInstance = hInst;
    wcApp.lpszClassName = szProgName;
    wcApp.lpfnWndProc = TrataEventos;
    wcApp.style = CS_HREDRAW | CS_VREDRAW;
    wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcApp.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wcApp.cbClsExtra = 0;
    wcApp.cbWndExtra = sizeof(DATA*);
    wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    if (!RegisterClassEx(&wcApp))
        return 0;

    hWnd = CreateWindow(
        szProgName,
        TEXT("BoardGUI"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        HWND_DESKTOP,
        NULL,
        hInst,
        NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&lpMsg, NULL, 0, 0) > 0) {
        TranslateMessage(&lpMsg);
        DispatchMessage(&lpMsg);
    }

    return (int)lpMsg.wParam;
}

void DrawBarGraph(HDC hdc, RECT rect, int* valores, int numEmpresas, int escalaMin, int escalaMax) {
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int barWidth = width / numEmpresas;

    HBRUSH hBrushBackground = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &rect, hBrushBackground);
    DeleteObject(hBrushBackground);

    for (int i = 0; i < numEmpresas; i++) {
        int barHeight = (valores[i] - escalaMin) * height / (escalaMax - escalaMin);
        RECT barRect = {
            rect.left + i * barWidth,
            rect.bottom - barHeight,
            rect.left + (i + 1) * barWidth,
            rect.bottom
        };
        FillRect(hdc, &barRect, (HBRUSH)(COLOR_WINDOW + 1));
        FrameRect(hdc, &barRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        TCHAR valorTexto[10];
        _stprintf_s(valorTexto, 10, TEXT("%d"), valores[i]);
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, barRect.left + (barWidth / 4), barRect.top - 20, valorTexto, lstrlen(valorTexto));
    }
}

DWORD WINAPI moveLetra(LPVOID data) {
    DATA* pData = (DATA*)data;
    while (pData->continua) {
        WaitForSingleObject(pData->hMutex, INFINITE);

        pData->pos[0].x += 1;
        if (pData->pos[0].x > pData->dim.right) {
            pData->pos[0].x = pData->dim.left;
        }

        ReleaseMutex(pData->hMutex);
        InvalidateRect(pData->hWnd, NULL, TRUE);
        Sleep(50);
    }
    return 0;
}

INT_PTR CALLBACK TrataDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DATA* pData;
    TCHAR str[10];
    int scaleMin, scaleMax, numCompanies;

    switch (message) {
    case WM_INITDIALOG:
        pData = (DATA*)GetWindowLongPtr(GetParent(hDlg), 0);
        if (pData != NULL) {
            WaitForSingleObject(pData->hMutex, INFINITE);
            _itot_s(pData->scaleMin, str, 10, 10);
            SetDlgItemText(hDlg, IDC_EDIT1, str);
            _itot_s(pData->scaleMax, str, 10, 10);
            SetDlgItemText(hDlg, IDC_EDIT2, str);
            _itot_s(pData->nCompanys, str, 10, 10);
            SetDlgItemText(hDlg, IDC_EDIT3, str);
            ReleaseMutex(pData->hMutex);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemText(hDlg, IDC_EDIT1, str, 10);
            scaleMin = _ttoi(str);
            GetDlgItemText(hDlg, IDC_EDIT2, str, 10);
            scaleMax = _ttoi(str);
            GetDlgItemText(hDlg, IDC_EDIT3, str, 10);
            numCompanies = _ttoi(str);

            pData = (DATA*)GetWindowLongPtr(GetParent(hDlg), 0);
            if (pData != NULL) {
                WaitForSingleObject(pData->hMutex, INFINITE);
                pData->scaleMin = scaleMin;
                pData->scaleMax = scaleMax;
                pData->nCompanys = numCompanies;
                ReleaseMutex(pData->hMutex);
            }

            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    int res;
    HDC hdc;
    PAINTSTRUCT ps;
    DATA* pData = (DATA*)GetWindowLongPtr(hWnd, 0);
    static HANDLE hThread;

    switch (message) {
    case WM_CREATE:
        pData = (DATA*)malloc(sizeof(DATA));
        SetWindowLongPtr(hWnd, 0, (LONG_PTR)pData);

        pData->hMutex = CreateMutex(NULL, FALSE, NULL);
        pData->hWnd = hWnd;
        GetClientRect(hWnd, &pData->dim);
        pData->continua = TRUE;
        pData->scaleMin = 0;
        pData->scaleMax = 100;
        pData->nCompanys = 10;

        hThread = CreateThread(NULL, 0, moveLetra, (LPVOID)pData, 0, NULL);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &pData->dim);
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        int valores[10] = { 30, 50, 80, 60, 70, 90, 40, 20, 50, 10 }; // Exemplos de valores das ações
        DrawBarGraph(hdc, pData->dim, valores, pData->nCompanys, pData->scaleMin, pData->scaleMax);

        TextOut(hdc, 10, 10, TEXT("Empresa mais recente: "), 22);
        //TextOut(hdc, 170, 10, pData->empresaRecente, lstrlen(pData->empresaRecente));

        EndPaint(hWnd, &ps);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_FICHEIRO_CONFIG:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, TrataDialog);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case ID_HELP_ABOUT:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDialogProc);
            break;
        }
        break;

    case WM_SIZE:
        WaitForSingleObject(pData->hMutex, INFINITE);
        GetClientRect(hWnd, &pData->dim);
        ReleaseMutex(pData->hMutex);
        break;

    case WM_CLOSE:
        res = MessageBox(hWnd, TEXT("Pretende mesmo sair?"), TEXT("Confirmação"), MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL);
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
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
