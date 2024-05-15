#include "Utils.h"

BOOL isAuthenticated = FALSE;

BOOL sendOperation(HANDLE hPipe, Response* response) {
    DWORD bytesWritten;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    BOOL success = WriteFile(hPipe, response, sizeof(Response), &bytesWritten, &ov);
    if (!success) {
        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEvent, INFINITE);
            success = GetOverlappedResult(hPipe, &ov, &bytesWritten, FALSE);
        }
    }
    CloseHandle(hEvent);
    if (!success || bytesWritten == 0) {
        _tprintf(_T("Erro ao enviar operação: %d\n"), GetLastError());
        return FALSE;
    }
    return TRUE;
}


HANDLE connectToServer() {
    HANDLE hPipe;
    hPipe = CreateFile(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        _tprintf(_T("Erro ao conectar ao pipe: %d\n"), GetLastError());
        return NULL;
    }
    _tprintf(_T("Conectado ao servidor.\n"));
    return hPipe;
}


BOOL authenticate(HANDLE hPipe, TCHAR* credentials) {
    Response response = { 0 };
    _tcscpy_s(response.operacao.msg, TAM, credentials); 

    if (!sendOperation(hPipe, &response)) {
        _tprintf(_T("Falha ao enviar comando de login.\n"));
        return FALSE;
    }

    DWORD bytesRead;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    BOOL readSuccess = ReadFile(hPipe, &response, sizeof(response), &bytesRead, &ov);
    if (!readSuccess && GetLastError() == ERROR_IO_PENDING) {
        WaitForSingleObject(hEvent, INFINITE);
        readSuccess = GetOverlappedResult(hPipe, &ov, &bytesRead, FALSE);
    }
    CloseHandle(hEvent);

    if (!readSuccess || bytesRead == 0) {
        _tprintf(_T("Falha ao receber resposta de login: %d\n"), GetLastError());
        return FALSE;
    }

    _tprintf(_T("Resposta: %s\n"), response.mensagem);
    return _tcscmp(response.mensagem, _T("Login bem-sucedido")) == 0;
}

void userInterface(HANDLE hPipe) {
    Response response = { 0 }; // Inicializa a estrutura a zeros
    TCHAR input[TAM];
    DWORD bytesRead;

    while (TRUE) {
        if (!isAuthenticated) {
            _tprintf(_T("Digite o comando 'login <username> <password>' para autenticar ou 'exit' para encerrar: "));
        }
        else {
            _tprintf(_T("Digite um comando ('buy', 'sell', 'listc', 'balance', 'exit'): "));
        }

        _fgetts(input, TAM, stdin);
        input[_tcslen(input) - 1] = '\0';  // Remove o caractere de nova linha

        if (_tcscmp(input, _T("exit")) == 0) break;

        if (_tcsncmp(input, _T("login "), 6) == 0) {
            if (authenticate(hPipe, input)) {
                isAuthenticated = TRUE;
                _tprintf(_T("Autenticação bem-sucedida. Você agora pode executar outros comandos.\n"));
                continue;
            }
            else {
                _tprintf(_T("Falha na autenticação. Tente novamente.\n"));
                continue;
            }
        }

        if (!isAuthenticated) {
            _tprintf(_T("Por favor, autentique-se usando o comando 'login <username> <password>'.\n"));
            continue;
        }

        _tcscpy_s(response.operacao.msg, TAM, input); 

        if (!sendOperation(hPipe, &response)) {
            _tprintf(_T("Falha ao enviar comando.\n"));
            continue;
        }

        if (!ReadFile(hPipe, &response, sizeof(response), &bytesRead, NULL) || bytesRead == 0) {
            _tprintf(_T("Falha ao receber resposta: %d\n"), GetLastError());
            continue;
        }

        _tprintf(_T("Resposta: %s\n"), response.mensagem);
    }
}


int _tmain(int argc, TCHAR* argv[]) {
#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
    _setmode(_fileno(stderr), _O_WTEXT);
#endif 

    HANDLE hPipe = connectToServer();
    if (hPipe == NULL) {
        _tprintf(_T("Não foi possível conectar ao servidor.\n"));
        return 1;
    }

    userInterface(hPipe);

    CloseHandle(hPipe);
    return 0;
}