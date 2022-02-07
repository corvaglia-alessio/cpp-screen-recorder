#pragma once

enum {
    REC_BTN_ID,
    PAUSE_BTN_ID,
    SETTINGS_BTN_ID,
    MIC_BTN_ID,
    CROP_BTN_ID,
    LOGGER_ID,
    PICKER_ID,
    FILE_LABEL_ID,
    UP_CROP_ID,
    DOWN_CROP_ID,
    LEFT_CROP_ID,
    RIGHT_CROP_ID,
    VIDEO_COMBOBOX_ID,
    AUDIO_COMBOBOX_ID,
    FPS_ID,
    CROP_FRAME_CROP_BTN_ID,
    CROP_FRAME_CANCEL_BTN_ID
};

static std::wstring ExePath() {
#ifdef UNIX
    char pBuf[MAX_BYTES];
    size_t len = size(pBuf);
    int bytes = MIN(readlink("/proc/self/exe", pBuf, len), len - 1);
    if (bytes >= 0)
        pBuf[bytes] = '\0';
    int last_slash = 0;
    for (int i = 0; i < bytes; i++) {
        if (pBuf[i] == '/')
            last_slash = i;
    }
    return std::wstring(&pBuf[0], &pBuf[last_slash]);
#else
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    std::wstring exe_path = std::wstring(buffer).substr(0, pos);
    std::replace(exe_path.begin(), exe_path.end(), '\\', '/');
    return exe_path;
#endif
}