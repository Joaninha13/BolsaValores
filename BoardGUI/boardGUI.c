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

BOOL InicializaAll(DATA* all) {

    all->continua = TRUE;

    all->hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, FILE_MAPPING_NAME);
    if (all->hMap == NULL) {
        _tprintf(_T("servidor ainda nao se encontra em execução.\n"));
        return FALSE;
    }

    all->shared = (MemoryShare*)MapViewOfFile(all->hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MemoryShare));
    if (all->shared == NULL) {
        _tprintf(_T("Error : MapViewOfFile (%d)\n"), GetLastError());
        return FALSE;
    }

    all->hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
    if (all->hMutex == NULL) {
       _tprintf(_T("Error : CreateMutex (%d)\n"), GetLastError());
        return FALSE;
    }

    //Criar o evento que vai ser usado para dizer ao board que ah updates na memoria partilhada
    all->hEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
    if (all->hEvent == NULL) {
       _tprintf(_T("Error : CreateEvent (%d)\n"), GetLastError());
        return FALSE;
    }

    all->nCompanys = 10;

    return TRUE;
}

void ordenarDecrescente(MemoryShare* s, int tamanho) {
    int i, j, max_index;


    CompanyShares temp;

    for (i = 0; i < tamanho - 1; i++) {
        max_index = i;

        for (j = i + 1; j < tamanho; j++)
            if (s->topAcoes[j].valor > s->topAcoes[max_index].valor)
                max_index = j;



        temp = s->topAcoes[i];
        s->topAcoes[i] = s->topAcoes[max_index];
        s->topAcoes[max_index] = temp;
    }
}

DWORD WINAPI update(LPVOID dados) {

    DATA* td = (DATA*)dados;

    MemoryShare shared;

    ZeroMemory(&shared, sizeof(MemoryShare));;


    do {

        WaitForSingleObject(td->hEvent, INFINITE);

        WaitForSingleObject(td->hMutex, INFINITE);

        CopyMemory(&shared, td->shared, sizeof(MemoryShare));

        ReleaseMutex(td->hMutex);

        ordenarDecrescente(&shared, MAX_EMPRESAS);

        //for (DWORD i = 0; i < td->nCompanys; i++)
        //    //if (_tcscmp(shared.topAcoes[i].name, _T("") != 0))
        //    _tprintf_s(_T("%d. %s - Valor da ação: %.2f\n"), i + 1, shared.topAcoes[i].name, shared.topAcoes[i].valor);


    } while (shared.continua && td->continua);


    return 0;
}

void DrawBarGraph(HDC hdc, DATA *pData) {
    int width = pData->dim.right - pData->dim.left;
    int height = pData->dim.bottom - pData->dim.top;
    int barWidth = width / pData->nCompanys;

    HBRUSH hBrushBackground = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdc, &pData->dim, hBrushBackground);
    DeleteObject(hBrushBackground);

    for (int i = 0; i < pData->nCompanys; i++) {
        int barHeight = (pData->shared->topAcoes[i].valor - pData->scaleMin) * height / (pData->scaleMax - pData->scaleMin);
        RECT barRect = {
            pData->dim.left + i * barWidth,
            pData->dim.bottom - barHeight,
            pData->dim.left + (i + 1) * barWidth,
            pData->dim.bottom
        };
        FillRect(hdc, &barRect, (HBRUSH)(COLOR_WINDOW + 1));
        FrameRect(hdc, &barRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        TCHAR valorTexto[10];
        _stprintf_s(valorTexto, 10, _T("%.2f"), pData->shared->topAcoes[i].valor);
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, barRect.left + (barWidth / 4), barRect.top - 20, valorTexto, lstrlen(valorTexto));
    }
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

    TCHAR buffer[TAM];

    DATA* pData = (DATA*)GetWindowLongPtr(hWnd, 0);
    static HANDLE hThread;

    switch (message) {
    case WM_CREATE:
        pData = (DATA*)malloc(sizeof(DATA));
        SetWindowLongPtr(hWnd, 0, (LONG_PTR)pData);

        if (!InicializaAll(&pData)) {
			free(pData);
			PostQuitMessage(0);
			return -1;
		}

        pData->hWnd = hWnd;
        GetClientRect(hWnd, &pData->dim);
        pData->continua = TRUE;
        pData->scaleMin = 0;
        pData->scaleMax = 100;
        pData->nCompanys = 10;

        hThread = CreateThread(NULL, 0, update, (LPVOID)pData, 0, NULL);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &pData->dim);
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        DrawBarGraph(hdc, &pData);

        TextOut(hdc, 10, 10, _T("Empresa mais recente: "), 22);
        //TextOut(hdc, 170, 10, pData->empresaRecente, lstrlen(pData->empresaRecente));

        if (pData->shared->isCompra)
            TextOut(hdc, 10, 10, _T("Compra"),6);
        else
            TextOut(hdc, 10, 10, _T("Posto a Venda"), 13);

        _stprintf_s(buffer, _T("Empresa - %s"), pData->shared->venda.name);
        TextOut(hdc, 10, 10, buffer, _tcslen(buffer));

        _stprintf_s(buffer, _T("Numero de Ações - %d"), pData->shared->venda.numAcoes);
        TextOut(hdc, 10, 30, buffer, _tcslen(buffer));

        //VER PORQUE ISTO DA ERRO POR SER DOUBLE?
        /*_stprintf_s(buffer, _T("Valor - %.2f"), pData->shared->venda.valor);
        TextOut(hdc, 10, 50, buffer, _tcslen(buffer));*/

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
