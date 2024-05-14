#include "Utils.h"

BOOL isAuthenticated = FALSE;

BOOL sendOperation(HANDLE hPipe, Operation* operation) {
    DWORD bytesWritten;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    BOOL success = WriteFile(hPipe, operation, sizeof(Operation), &bytesWritten, &ov);
    if (!success) {
        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(hEvent, INFINITE);
            success = GetOverlappedResult(hPipe, &ov, &bytesWritten, FALSE);
        }
    }
    CloseHandle(hEvent);
    if (!success || bytesWritten == 0) {
        _tprintf(_T("Erro ao enviar opera��o: %d\n"), GetLastError());
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


BOOL authenticate(HANDLE hPipe, Operation* operation) {
    TCHAR response[256];
    DWORD bytesRead;
    OVERLAPPED ov = { 0 };
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ov.hEvent = hEvent;

    if (!sendOperation(hPipe, operation)) {
        _tprintf(_T("Falha ao enviar comando de login.\n"));
        CloseHandle(hEvent);
        return FALSE;
    }

    BOOL readSuccess = ReadFile(hPipe, response, sizeof(response), &bytesRead, &ov);
    if (!readSuccess && GetLastError() == ERROR_IO_PENDING) {
        WaitForSingleObject(hEvent, INFINITE);
        readSuccess = GetOverlappedResult(hPipe, &ov, &bytesRead, FALSE);
    }
    CloseHandle(hEvent);
    if (!readSuccess || bytesRead == 0) {
        _tprintf(_T("Falha ao receber resposta de login: %d\n"), GetLastError());
        return FALSE;
    }

    _tprintf(_T("Resposta: %s\n"), response);
    return _tcscmp(response, _T("Login bem-sucedido")) == 0;
}
void userInterface(HANDLE hPipe) {
    Operation operation = { 0 }; // Inicializa a estrutura a zeros
    TCHAR response[1024];
    DWORD bytesRead;

    while (TRUE) {
        if (!isAuthenticated) {
            _tprintf(_T("Digite o comando 'login <username> <password>' para autenticar ou 'exit' para encerrar: "));
        }
        else {
            _tprintf(_T("Digite um comando ('buy', 'sell', 'listc', 'balance', 'exit'): "));
        }

        _fgetts(operation.msg, TAM, stdin);
        operation.msg[_tcslen(operation.msg) - 1] = '\0';  // Remove o caractere de nova linha

        if (_tcscmp(operation.msg, _T("exit")) == 0) break;

        if (_tcsncmp(operation.msg, _T("login "), 6) == 0) {
            if (authenticate(hPipe, &operation)) {
                isAuthenticated = TRUE;
                _tprintf(_T("Autentica��o bem-sucedida. Voc� agora pode executar outros comandos.\n"));
                continue;
            }
            else {
                _tprintf(_T("Falha na autentica��o. Tente novamente.\n"));
                continue;
            }
        }

        else if (_tcsncmp(operation.msg, _T("buy "), 4) == 0) {
            operation.isCompra = TRUE;
            TCHAR* context = NULL;
            TCHAR* token = _tcstok_s(operation.msg, _T(" "), &context); // saltar "buy"
            token = _tcstok_s(NULL, _T(" "), &context); // Nome da empresa
            _tcscpy_s(operation.nomeEmpresa, MAX_NOME, token);
            token = _tcstok_s(NULL, _T(" "), &context); // N�mero de a��es
            operation.quantidadeAcoes = _tstoi(token);

            if (!sendOperation(hPipe, &operation)) {
                _tprintf(_T("Falha ao enviar comando 'buy'.\n"));
                continue;
            }
        }

        else if (_tcsncmp(operation.msg, _T("sell "), 5) == 0) {
            operation.isCompra = FALSE;
            TCHAR* context = NULL;
            TCHAR* token = _tcstok_s(operation.msg, _T(" "), &context); // saltar "sell"
            token = _tcstok_s(NULL, _T(" "), &context); // Nome da empresa
            _tcscpy_s(operation.nomeEmpresa, MAX_NOME, token);
            token = _tcstok_s(NULL, _T(" "), &context); // N�mero de a��es
            operation.quantidadeAcoes = _tstoi(token);

            if (!sendOperation(hPipe, &operation)) {
                _tprintf(_T("Falha ao enviar comando 'sell'.\n"));
                continue;
            }
        }

        else if (!isAuthenticated) {
            _tprintf(_T("Por favor, autentique-se usando o comando 'login <username> <password>'.\n"));
            continue;
        }

        else {
            if (!sendOperation(hPipe, &operation)) {
                _tprintf(_T("Falha ao enviar comando: %d\n"), GetLastError());
                continue;
            }
        }

        if (!ReadFile(hPipe, response, sizeof(response), &bytesRead, NULL) || bytesRead == 0) {
            _tprintf(_T("Falha ao receber resposta: %d\n"), GetLastError());
            continue;
        }

        _tprintf(_T("Resposta: %s\n"), response);
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
        _tprintf(_T("N�o foi poss�vel conectar ao servidor.\n"));
        return 1;
    }

    userInterface(hPipe);

    CloseHandle(hPipe);
    return 0;
}