#include "output.h"
#include <windows.h>
#include <io.h>
#include <stdarg.h>
#include <stdlib.h>

static int write_console(HANDLE handle, const char *text, int length)
{
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, length, NULL, 0);
    if (count <= 0) { return -1; }
    wchar_t *wide = malloc((size_t)count * sizeof(*wide));
    if (wide == NULL) { return -1; }
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, length, wide, count) != count) {
        free(wide);
        return -1;
    }
    int offset = 0;
    while (offset < count) {
        DWORD chunk = (DWORD)(count - offset);
        if (chunk > 16000) { chunk = 16000; }
        /* UTF-16 서로게이트 쌍을 두 번의 쓰기로 나누지 않는다. */
        if (offset + (int)chunk < count &&
            wide[offset + chunk - 1] >= 0xD800 && wide[offset + chunk - 1] <= 0xDBFF) {
            --chunk;
        }
        DWORD written = 0;
        if (!WriteConsoleW(handle, wide + offset, chunk, &written, NULL) || written == 0) {
            free(wide);
            return -1;
        }
        offset += (int)written;
    }
    free(wide);
    return length;
}

int output_printf(FILE *stream, const char *format, ...)
{
    va_list arguments, measure;
    va_start(arguments, format);
    va_copy(measure, arguments);
    int length = vsnprintf(NULL, 0, format, measure);
    va_end(measure);
    if (length < 0) { va_end(arguments); return -1; }
    char *text = malloc((size_t)length + 1);
    if (text == NULL) { va_end(arguments); return -1; }
    int formatted = vsnprintf(text, (size_t)length + 1, format, arguments);
    va_end(arguments);
    if (formatted != length) { free(text); return -1; }

    HANDLE handle = (HANDLE)_get_osfhandle(_fileno(stream));
    DWORD mode;
    int result;
    if (length == 0) {
        result = 0;
    } else if (GetConsoleMode(handle, &mode)) {
        /* 코드 페이지를 바꾸지 않고 콘솔에 Unicode를 직접 전달한다. */
        result = fflush(stream) == 0 ? write_console(handle, text, length) : -1;
    } else {
        result = fwrite(text, 1, (size_t)length, stream) == (size_t)length ? length : -1;
    }
    free(text);
    return result;
}
