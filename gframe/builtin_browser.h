#pragma once
#ifndef _WIN32
#else
#include <windows.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <shobjidl.h>
#include <WebView2.h>

void BuiltinBrowserLog(const wchar_t* fmt, ...);

// Define the implementation structure here so it's visible to the class
struct BuiltinBrowserImpl {
    std::thread sta_thread;
    std::atomic<DWORD> thread_id{0};
    std::atomic<bool> is_open{false};
    std::atomic<bool> thread_running{false};
    HWND parent_hwnd = nullptr;
    HWND child_hwnd = nullptr;
    ICoreWebView2* webview = nullptr;
    ICoreWebView2Controller* controller = nullptr;
    EventRegistrationToken accelerator_token{};
    std::wstring current_url;
    std::mutex url_mutex;
    RECT content_rect{};  // Content area rectangle in client coordinates
    std::wstring current_title;  // Current page title
    std::mutex title_mutex;  // Mutex for protecting title access
};

class BuiltinBrowser {
public:
    static BuiltinBrowser& Instance();
    bool Open(const wchar_t* url, HWND parent);
    void Close();
    bool IsOpen() const;
    void SetContentRect(const RECT& r);
    void GoBack();
    void GoForward();
    void Reload();
    std::wstring GetTitle();
private:
    BuiltinBrowser() = default;
    ~BuiltinBrowser();  // Allow definition in cpp file
    BuiltinBrowser(const BuiltinBrowser&) = delete;
    BuiltinBrowser& operator=(const BuiltinBrowser&) = delete;
    BuiltinBrowserImpl* impl_ = nullptr;
};
#endif