#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

/* 테스트 전용 콘솔에서만 Ctrl+C를 발생시킨다. 호출자의 콘솔에는 보내지 않는다. */
static BOOL WINAPI ignore_test_event(DWORD event)
{ return event == CTRL_C_EVENT ? TRUE : FALSE; }

int wmain(int argc, wchar_t **argv)
{
    if (argc != 3 || wcscmp(argv[1], L"\\Device\\NPF_Loopback") != 0) { return 2; }
    wchar_t *end = NULL;
    unsigned long port = wcstoul(argv[2], &end, 10);
    if (*end != L'\0' || port == 0 || port > 65535) { return 2; }
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE error = GetStdHandle(STD_ERROR_HANDLE);
    FreeConsole();
    if (!AllocConsole()) { return 3; }
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    /* 자식 프로그램이 상속된 무시 설정을 해제하는지도 검증한다. */
    if (!SetConsoleCtrlHandler(NULL, TRUE)) { return 3; }
    if (!SetConsoleCtrlHandler(ignore_test_event, TRUE)) { return 3; }
    wchar_t path[4096], command[8192];
    DWORD length = GetModuleFileNameW(NULL, path, 4096);
    if (length == 0 || length >= 4096) { return 3; }
    wchar_t *base = wcsrchr(path, L'\\');
    if (base == NULL || (size_t)(base - path) + 15 >= 4096) { return 3; }
    wcscpy(base + 1, L"netsentry.exe");
    if (swprintf(command, 8192,
        L"\"%ls\" --interface \"%ls\" --count 1 --duration 20 --filter \"ip and udp and dst port %lu\"",
        path, argv[1], port) < 0) { return 3; }
    STARTUPINFOW startup = {0};
    PROCESS_INFORMATION child = {0};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = output;
    startup.hStdError = error;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if (!SetHandleInformation(output, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT) ||
        !SetHandleInformation(error, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) { return 3; }
    if (!CreateProcessW(path, command, NULL, NULL, TRUE, 0, NULL, NULL, &startup, &child)) { return 3; }
    Sleep(1500);
    BOOL sent = GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
    DWORD waited = WaitForSingleObject(child.hProcess, 5000);
    DWORD status = 1;
    if (!sent || waited != WAIT_OBJECT_0) {
        char diagnostic[128];
        int size = snprintf(diagnostic, sizeof(diagnostic),
                            "Ctrl+C 전달 실패: 전송=%d 대기=%lu 오류=%lu\n",
                            (int)sent, (unsigned long)waited, (unsigned long)GetLastError());
        DWORD written;
        WriteFile(error, diagnostic, (DWORD)size, &written, NULL);
        TerminateProcess(child.hProcess, 4);
        WaitForSingleObject(child.hProcess, 5000);
    } else {
        GetExitCodeProcess(child.hProcess, &status);
    }
    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    FreeConsole();
    return (int)status;
}
