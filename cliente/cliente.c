#include "Utils.h"

BOOL running = TRUE;

DWORD WINAPI recebeMSG(LPVOID data) {
    HANDLE hPipe = (HANDLE)data;
    Response response;
    BOOL ret;
    DWORD n;
    OVERLAPPED ov;
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // Evento para avisar que já leu...

    if (hEvent == NULL) {
        _tprintf(_T("[ERRO] Falha ao criar evento\n"));
        return 1;
    }

    memset(&response, 0, sizeof(Response));

    do {
        ZeroMemory(&ov, sizeof(OVERLAPPED));
        ov.hEvent = hEvent;

        ret = ReadFile(hPipe, &response, sizeof(Response), &n, &ov);
        if (ret == TRUE) {
            _tprintf(_T("Li de imediato...\n"));
        }
        else if (GetLastError() == ERROR_IO_PENDING) {
            _tprintf(_T("Agendei uma leitura\n"));
            WaitForSingleObject(hEvent, INFINITE);
            GetOverlappedResult(hPipe, &ov, &n, FALSE);
        }
        else {
            _tprintf(_T("[ERRO] Leitura\n"));
            CloseHandle(hEvent); 
            return 1;
        }

        // verificação para ignorar mensagens vazias. Confirmar depois se funciona
        if (n == 0 || _tcslen(response.mensagem) == 0) {
            continue;
        }

        _tprintf(_T("\n[Cliente] Recebi %d bytes: '%s'... (ReadFile)\n"), n, response.mensagem);

        if (_tcscmp(response.mensagem, _T("close")) == 0) {
            running = FALSE;
        }
        else if (_tcscmp(response.mensagem, _T("listc")) == 0) {
            _tprintf(_T("\n[Cliente] Informações da carteira:\n"));
            for (int i = 0; i < MAX_EMPRESAS; i++) {
                if (_tcslen(response.listCompany[i].name) > 0) {
                    _tprintf(_T("Empresa: %s\n"), response.listCompany[i].name);
                    _tprintf(_T("Número de Ações: %d\n"), response.listCompany[i].numAcoes);
                    _tprintf(_T("Valor: %.2f\n"), response.listCompany[i].valor);
                }
            }
        }

    } while (running);

    CloseHandle(hEvent);
    return 0;
}

void userInterface(TCHAR* command, Response* response, BOOL* isLoggedIn) {
    TCHAR* context = NULL;
    TCHAR* token = _tcstok_s(command, _T(" "), &context);

    if (_tcscmp(token, _T("login")) == 0) {
        if (*isLoggedIn) {
            _tprintf(_T("[ERRO] Já está logado. Logout primeiro para fazer login novamente.\n"));
            return;
        }
        TCHAR* username = _tcstok_s(NULL, _T(" "), &context);
        TCHAR* password = _tcstok_s(NULL, _T(" "), &context);
        if (username != NULL && password != NULL) {
            _stprintf_s(response->mensagem, TAM, _T("login %s %s"), username, password);
            *isLoggedIn = TRUE;
        }
        else {
            _tprintf(_T("[ERRO] Uso: login <username> <password>\n"));
        }
    }
    else if (!*isLoggedIn) {
        _tprintf(_T("[ERRO] Você deve fazer login antes de executar outros comandos.\n"));
        return;
    }
    else if (_tcscmp(token, _T("listc")) == 0) {
        _stprintf_s(response->mensagem, TAM, _T("listc"));
    }
    else if (_tcscmp(token, _T("buy")) == 0) {
        TCHAR* empresa = _tcstok_s(NULL, _T(" "), &context);
        TCHAR* quantidade = _tcstok_s(NULL, _T(" "), &context);
        if (empresa != NULL && quantidade != NULL) {
            response->operacao.isCompra = TRUE;
            _stprintf_s(response->operacao.nomeEmpresa, TAM, _T("%s"), empresa);
            response->operacao.quantidadeAcoes = _tstoi(quantidade);
            _stprintf_s(response->mensagem, TAM, _T("buy %s %s"), empresa, quantidade);
        }
        else {
            _tprintf(_T("[ERRO] Uso: buy <nome-empresa> <numero-acoes>\n"));
        }
    }
    else if (_tcscmp(token, _T("sell")) == 0) {
        TCHAR* empresa = _tcstok_s(NULL, _T(" "), &context);
        TCHAR* quantidade = _tcstok_s(NULL, _T(" "), &context);
        if (empresa != NULL && quantidade != NULL) {
            response->operacao.isCompra = FALSE;
            _stprintf_s(response->operacao.nomeEmpresa, TAM, _T("%s"), empresa);
            response->operacao.quantidadeAcoes = _tstoi(quantidade);
            _stprintf_s(response->mensagem, TAM, _T("sell %s %s"), empresa, quantidade);
        }
        else {
            _tprintf(_T("[ERRO] Uso: sell <nome-empresa> <numero-acoes>\n"));
        }
    }
    else if (_tcscmp(token, _T("balance")) == 0) {
        _stprintf_s(response->mensagem, TAM, _T("balance"));
    }
    else if (_tcscmp(token, _T("exit")) == 0) {
        _stprintf_s(response->mensagem, TAM, _T("exit"));
        running = FALSE;
    }
    else {
        _tprintf(_T("[Cliente] Comando não reconhecido: %s\n"), command);
    }
}

int _tmain(int argc, LPTSTR argv[]) {
    Response response;
    TCHAR aux[TAM];
    HANDLE hPipe, hThread;
    BOOL ret;
    DWORD n;
    BOOL isLoggedIn = FALSE;

#ifdef UNICODE 
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif 

    memset(&response, 0, sizeof(Response));

    _tprintf(_T("[Cliente] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);

    if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
        _tprintf(_T("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
        exit(-1);
    }

    _tprintf(_T("[Cliente] Ligar ao pipe do escritor... (CreateFile)\n"));

    hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(_T("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
        exit(-1);
    }

    _tprintf(_T("[Cliente] Liguei-me...\n"));

    hThread = CreateThread(NULL, 0, recebeMSG, (LPVOID)hPipe, 0, NULL);
    if (hThread == NULL) {
        _tprintf(_T("[ERRO] Falha ao criar thread\n"));
        CloseHandle(hPipe);
        exit(-1);
    }

    OVERLAPPED ov;
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == NULL) {
        _tprintf(_T("[ERRO] Falha ao criar evento\n"));
        CloseHandle(hPipe);
        CloseHandle(hThread);
        exit(-1);
    }

    while (running) {
        _tprintf(_T("[Cliente] Comando: "));
        _fgetts(response.mensagem, 256, stdin);
        response.mensagem[_tcslen(response.mensagem) - 1] = '\0';

        if (!running) {
            break;
        }

        userInterface(response.mensagem, &response, &isLoggedIn);

        if (!running) {
            break;
        }

        ZeroMemory(&ov, sizeof(OVERLAPPED));
        ov.hEvent = hEvent;

        _tprintf(_T("[Cliente] Vou enviar a mensagem '%s'... (WriteFile)\n"), response.mensagem);

        ret = WriteFile(hPipe, &response, sizeof(Response), &n, &ov);
        if (ret == TRUE) {
            _tprintf(_T("Escrevi...\n"));
        }
        else {
            if (GetLastError() == ERROR_IO_PENDING) {
                _tprintf(_T("Agendei uma escrita\n"));
                WaitForSingleObject(hEvent, INFINITE);
                GetOverlappedResult(hPipe, &ov, &n, FALSE);
            }
            else {
                _tprintf(_T("[ERRO] Escrita\n"));
                CloseHandle(hPipe);
                CloseHandle(hThread);
                CloseHandle(hEvent);
                exit(-1);
            }
        }

        _tprintf(_T("[Cliente] Enviei %d bytes ao leitor... (WriteFile)\n"), n);
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    CloseHandle(hPipe);
    CloseHandle(hEvent);

    return 0;
}
