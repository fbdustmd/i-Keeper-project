#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static int check_case(const wchar_t *exe, UINT page, const wchar_t *arguments,
                      DWORD expected, const wchar_t *message)
{
    SECURITY_ATTRIBUTES security = {sizeof(security), NULL, TRUE};
    HANDLE screen = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, &security, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (screen == INVALID_HANDLE_VALUE) { return 1; }
    COORD size = {160, 100};
    if (!SetConsoleScreenBufferSize(screen, size) || !SetConsoleOutputCP(page)) {
        CloseHandle(screen); return 1;
    }
    wchar_t command[8192];
    if (swprintf(command, 8192, L"\"%ls\" %ls", exe, arguments) < 0) {
        CloseHandle(screen); return 1;
    }
    STARTUPINFOW startup = {0};
    PROCESS_INFORMATION child = {0};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = screen;
    startup.hStdError = screen;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if (!CreateProcessW(exe, command, NULL, NULL, TRUE, 0, NULL, NULL, &startup, &child)) {
        CloseHandle(screen); return 1;
    }
    int result = 1;
    if (WaitForSingleObject(child.hProcess, 10000) != WAIT_OBJECT_0) {
        TerminateProcess(child.hProcess, 1);
        WaitForSingleObject(child.hProcess, 5000);
    } else {
        DWORD status, count;
        wchar_t text[16001];
        COORD origin = {0, 0};
        if (GetExitCodeProcess(child.hProcess, &status) && status == expected &&
            ReadConsoleOutputCharacterW(screen, text, 16000, origin, &count)) {
            text[count] = L'\0';
            if (wcsstr(text, message) != NULL && GetConsoleOutputCP() == page) { result = 0; }
        }
    }
    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    CloseHandle(screen);
    return result;
}

int wmain(int argc, wchar_t **argv)
{
    if (argc != 2) { return 2; }
    /* 호출자의 콘솔 설정을 건드리지 않는 전용 테스트 콘솔이다. */
    FreeConsole();
    if (!AllocConsole()) { return 2; }
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    const UINT pages[] = {949, 65001};
    int result = 0;
    for (size_t i = 0; i < sizeof(pages) / sizeof(pages[0]); ++i) {
        result |= check_case(argv[1], pages[i], L"--help", 0, L"사용법: netsentry");
        result |= check_case(argv[1], pages[i], L"--count 5", 1, L"오류: 인자·범위·중복 옵션을 확인하세요.");
        result |= check_case(argv[1], pages[i], L"--list", 0, L"인터페이스 목록");
    }
    FreeConsole();
    /* 결과 문구는 호출하는 PowerShell 스크립트가 출력한다. */
    return result;
}
