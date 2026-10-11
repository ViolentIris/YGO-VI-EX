#include "builtin_browser.h"
#ifdef _WIN32
#include <windows.h>
#include <combaseapi.h>
#include <ole2.h>
#include <shobjidl.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <wrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "WebView2LoaderStatic.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "runtimeobject.lib")

using namespace Microsoft::WRL;

void BuiltinBrowserLog(const wchar_t* fmt, ...) {
    wchar_t msg[1024]{};
    va_list ap; va_start(ap, fmt);
    _vsnwprintf_s(msg, _TRUNCATE, fmt, ap);
    va_end(ap);
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(GetModuleHandleW(NULL), exe, MAX_PATH);
    std::wstring path(exe);
    size_t p = path.find_last_of(L"\\");
    if (p != std::wstring::npos) path = path.substr(0, p + 1);
    path += L"builtin_browser.log";
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"a, ccs=UTF-8") != 0 || !f) return;
    SYSTEMTIME st; GetLocalTime(&st);
    fwprintf(f, L"[%02d:%02d:%02d] %ls\n", st.wHour, st.wMinute, st.wSecond, msg);
    fclose(f);
}

static void SaveWebViewPreview(ICoreWebView2* webview) {
    if (!webview) return;
    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &stream)) || !stream) return;
    HRESULT hr = webview->CapturePreview(
        COREWEBVIEW2_CAPTURE_PREVIEW_IMAGE_FORMAT_PNG, stream,
        Callback<ICoreWebView2CapturePreviewCompletedHandler>(
            [stream](HRESULT result) -> HRESULT {
                BuiltinBrowserLog(L"CapturePreview 完成 hr=0x%08X", result);
                if (SUCCEEDED(result)) {
                    HGLOBAL hg = nullptr;
                    if (SUCCEEDED(GetHGlobalFromStream(stream, &hg)) && hg) {
                        SIZE_T sz = GlobalSize(hg);
                        void* p = GlobalLock(hg);
                        if (p && sz > 0) {
                            wchar_t exe[MAX_PATH]{}; GetModuleFileNameW(GetModuleHandleW(NULL), exe, MAX_PATH);
                            std::wstring path(exe); size_t i = path.find_last_of(L"\\");
                            if (i != std::wstring::npos) path = path.substr(0, i + 1);
                            path += L"page_preview.png";
                            FILE* f = nullptr;
                            if (_wfopen_s(&f, path.c_str(), L"wb") == 0 && f) {
                                fwrite(p, 1, (size_t)sz, f); fclose(f);
                                BuiltinBrowserLog(L"页面截图已保存：%ls（%llu 字节）", path.c_str(), (unsigned long long)sz);
                            }
                        }
                        if (p) GlobalUnlock(hg);
                    }
                }
                stream->Release();
                return S_OK;
            }).Get());
    BuiltinBrowserLog(L"已调用 CapturePreview hr=0x%08X", hr);
}


// Message IDs for communication between main thread and STA thread
const UINT WM_OPEN_URL = WM_APP + 1;
const UINT WM_CLOSE_BROWSER = WM_APP + 2;
const UINT WM_NAVIGATE_URL = WM_APP + 3;
const UINT WM_SET_CONTENT_RECT = WM_APP + 4;
const UINT WM_GO_BACK = WM_APP + 5;
const UINT WM_GO_FORWARD = WM_APP + 6;
const UINT WM_RELOAD = WM_APP + 7;

// Forward declaration of the STA thread function
DWORD WINAPI StaThreadFunc(LPVOID param);

BuiltinBrowser& BuiltinBrowser::Instance() {
    static BuiltinBrowser instance;
    return instance;
}

// STA thread function
DWORD WINAPI StaThreadFunc(LPVOID param) {
	{ MSG _m0; PeekMessageW(&_m0, NULL, WM_USER, WM_USER, PM_NOREMOVE); }
	BuiltinBrowserLog(L"STA线程 已强制创建消息队列 OK");
    BuiltinBrowserImpl* impl = static_cast<BuiltinBrowserImpl*>(param);
    
    // Initialize COM in STA mode
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    BuiltinBrowserLog(L"STA线程 CoInitializeEx(STA) 返回 hr=0x%08X", hr);
    
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        CoUninitialize();
        return hr;
    }
    
    // Store the thread ID
    impl->thread_id = GetCurrentThreadId();
    impl->thread_running = true;
    
    // Create child window in this thread (HWND must belong to the thread that creates it)
    if (impl->parent_hwnd && IsWindow(impl->parent_hwnd)) {
        RECT rect;
        if (GetClientRect(impl->parent_hwnd, &rect)) {
            impl->child_hwnd = CreateWindowExW(
                0,
                L"STATIC",
                L"",
                WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                0, 0,
                1,
                1,
                impl->parent_hwnd,
                NULL,
                GetModuleHandleW(NULL),
                NULL
            );
            
            if (impl->child_hwnd) {
                ShowWindow(impl->child_hwnd, SW_SHOW);
                UpdateWindow(impl->child_hwnd);
                BuiltinBrowserLog(L"STA线程 创建子窗口成功 child_hwnd=%p", (void*)impl->child_hwnd);
            } else {
                BuiltinBrowserLog(L"STA线程 创建子窗口失败");
            }
        }
    }
    
    // Prepare user data directory
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(GetModuleHandleW(NULL), exe_path, MAX_PATH);
    PathRemoveFileSpecW(exe_path);
    std::wstring user_data_dir = std::wstring(exe_path) + L"\\webview2_data";
    
    // Create the environment options
    auto options = Make<CoreWebView2EnvironmentOptions>();
    
    // Create the webview environment
    BuiltinBrowserLog(L"STA线程 调用 CreateCoreWebView2EnvironmentWithOptions；user_data=%ls", user_data_dir.c_str());
    hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        user_data_dir.c_str(),
        options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [impl](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                BuiltinBrowserLog(L"STA线程 环境创建回调 hr=0x%08X", result);
                
                if (SUCCEEDED(result) && env) {
                    // Create the webview controller
                    HRESULT hr_ctrl = env->CreateCoreWebView2Controller(
                        impl->child_hwnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [impl](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                                BuiltinBrowserLog(L"STA线程 控制器创建回调 hr=0x%08X", result);
                                
                                if (SUCCEEDED(result) && controller) {
                                    impl->controller = controller;
                                    impl->controller->AddRef();

                                    // Get the webview instance
                                    controller->get_CoreWebView2(&impl->webview);
                                    if (impl->webview) {
                                        impl->webview->AddRef();
                                    }

                                    // Set bounds mode to use raw pixels if available
                                    #ifdef COREWEBVIEW2_BOUNDS_MODE_USE_RAW_PIXELS
                                    controller->put_BoundsMode(COREWEBVIEW2_BOUNDS_MODE_USE_RAW_PIXELS);
                                    #endif

                                    // Position the webview to fill the container
                                    if (impl->child_hwnd) {
                                        RECT bounds;
                                        GetClientRect(impl->child_hwnd, &bounds);
                                        controller->put_Bounds(bounds);
                                        controller->put_IsVisible(TRUE);
                                        PostThreadMessage(GetCurrentThreadId(), WM_SET_CONTENT_RECT, 0, 0);
                                        BuiltinBrowserLog(L"控制器就绪 → 已补发 WM_SET_CONTENT_RECT ✓");
                                        {
                                            std::lock_guard<std::mutex> lk(impl->url_mutex);
                                            if (!impl->current_url.empty() && impl->webview) {
                                                BuiltinBrowserLog(L"控制器就绪后补导航 -> %ls", impl->current_url.c_str());
                                                impl->webview->Navigate(impl->current_url.c_str());
                                                impl->current_url.clear();
                                            }
                                        }
                                        impl->webview->add_NavigationCompleted(
                                            Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                                [impl](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                                    BOOL ok = FALSE; args->get_IsSuccess(&ok);
                                                    COREWEBVIEW2_WEB_ERROR_STATUS st = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
                                                    args->get_WebErrorStatus(&st);
                                                    PWSTR uri = nullptr; sender->get_Source(&uri);
                                                    PWSTR title = nullptr; sender->get_DocumentTitle(&title);
                                                    BuiltinBrowserLog(L"★页面加载完成：success=%d err=%d title=%ls uri=%ls", (int)ok, (int)st, title ? title : L"", uri ? uri : L"");
                                                    // Store the page title for later retrieval
                                                    if (title) {
                                                        std::lock_guard<std::mutex> lock(impl->title_mutex);
                                                        impl->current_title = std::wstring(title);
                                                        CoTaskMemFree(title);
                                                    }
                                                    if (uri) CoTaskMemFree(uri);
                                                    SaveWebViewPreview(sender);
                                                    if (impl->parent_hwnd) {
                                                        SetFocus(impl->parent_hwnd);
                                                        BuiltinBrowserLog(L"已把焦点还给游戏窗口 → Esc 可用了 ✓");
                                                    }
                                                    return S_OK;
                                                }).Get(), nullptr);
                                        BuiltinBrowserLog(L"已注册 NavigationCompleted 回调 ✓");
                                        BuiltinBrowserLog(L"已 put_IsVisible(TRUE) —— 内置浏览器应可见了 ✓");
                                    }

                                    // Handle accelerator key pressed event to close on ESC
                                    auto esc_handler = Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
                                        [impl](ICoreWebView2Controller* sender, ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {
                                            COREWEBVIEW2_KEY_EVENT_KIND event_kind;
                                            args->get_KeyEventKind(&event_kind);
                                            
                                            if (event_kind == COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN) {
                                                UINT virtual_key;
                                                args->get_VirtualKey(&virtual_key);
                                                
                                                if (virtual_key == VK_ESCAPE) {
                                                    // Send close message to our own thread
                                                    PostThreadMessage(impl->thread_id.load(), WM_CLOSE_BROWSER, 0, 0);
                                                }
                                            }
                                            return S_OK;
                                        });

                                    if (impl->webview) {
                                        impl->controller->add_AcceleratorKeyPressed(esc_handler.Get(), &impl->accelerator_token);
                                    }

                                    impl->is_open = true;
                                    BuiltinBrowserLog(L"STA线程 WebView2 初始化完成，开始消息循环");
                                } else {
                                    // Hide the child window if controller creation failed
                                    if (impl->child_hwnd) {
                                        ShowWindow(impl->child_hwnd, SW_HIDE);
                                    }
                                    BuiltinBrowserLog(L"STA线程 控制器创建失败，准备回退到系统浏览器");
                                }
                                return S_OK;
                            }).Get()
                    );
                } else {
                    // Hide the child window if environment creation failed
                    if (impl->child_hwnd) {
                        ShowWindow(impl->child_hwnd, SW_HIDE);
                    }
                    BuiltinBrowserLog(L"STA线程 环境创建失败 hr=0x%08X，准备回退到系统浏览器", result);
                }
                return S_OK;
            }).Get()
    );
    
    if (FAILED(hr)) {
        BuiltinBrowserLog(L"STA线程 CreateCoreWebView2EnvironmentWithOptions 失败 hr=0x%08X", hr);
    }
    
    // Message loop for the STA thread
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        // Handle custom messages
        if (msg.message == WM_OPEN_URL || msg.message == WM_NAVIGATE_URL) {
            if (impl->webview && !impl->current_url.empty()) {
                BuiltinBrowserLog(L"STA线程 Navigate to URL: %ls", impl->current_url.c_str());
                impl->webview->Navigate(impl->current_url.c_str());
                
                // Clear the URL after navigating
                std::lock_guard<std::mutex> lock(impl->url_mutex);
                impl->current_url.clear();
            }
        } else if (msg.message == WM_CLOSE_BROWSER) {
            BuiltinBrowserLog(L"STA线程 收到关闭消息，开始清理");
            
            // Remove accelerator key handler
            if (impl->webview && impl->accelerator_token.value != 0) {
                impl->controller->remove_AcceleratorKeyPressed(impl->accelerator_token);
                impl->accelerator_token.value = 0;
            }
            
            // Close the webview controller
            if (impl->controller) {
                impl->controller->Close();
                impl->controller->Release();
                impl->controller = nullptr;
            }
            
            // Release the webview
            if (impl->webview) {
                impl->webview->Release();
                impl->webview = nullptr;
            }
            
            // Hide and destroy the child window
            if (impl->child_hwnd) {
                ShowWindow(impl->child_hwnd, SW_HIDE);
                DestroyWindow(impl->child_hwnd);
                impl->child_hwnd = nullptr;
            }
            
            impl->is_open = false;
            impl->thread_running = false;
            
            BuiltinBrowserLog(L"STA线程 清理完成，退出消息循环");
            break; // Exit the message loop
        } else if (msg.message == WM_SET_CONTENT_RECT) {
                if (impl->child_hwnd) {
                    RECT cr = impl->content_rect;
                    SetWindowPos(impl->child_hwnd, NULL, cr.left, cr.top,
                                 cr.right - cr.left, cr.bottom - cr.top,
                                 SWP_NOZORDER | SWP_NOACTIVATE);
                    BuiltinBrowserLog(L"容器窗口已移到内容区 (%d,%d,%d,%d) ✓", cr.left, cr.top, cr.right, cr.bottom);
                }
            if (impl->controller) {
                // Convert client coordinates to screen coordinates
                RECT client_rect = impl->content_rect;
                POINT pt_top_left = {client_rect.left, client_rect.top};
                POINT pt_bottom_right = {client_rect.right, client_rect.bottom};
                
                // Convert top-left corner to screen coordinates
                ClientToScreen(impl->parent_hwnd, &pt_top_left);
                
                // Convert bottom-right corner to screen coordinates  
                ClientToScreen(impl->parent_hwnd, &pt_bottom_right);
                
                // Calculate the screen rectangle
                RECT screen_rect;
                screen_rect.left = pt_top_left.x;
                screen_rect.top = pt_top_left.y;
                screen_rect.right = pt_bottom_right.x;
                screen_rect.bottom = pt_bottom_right.y;
                
                BuiltinBrowserLog(L"STA线程 SetContentRect: client(%d,%d,%d,%d) -> screen(%d,%d,%d,%d)", 
                    client_rect.left, client_rect.top, client_rect.right, client_rect.bottom,
                    screen_rect.left, screen_rect.top, screen_rect.right, screen_rect.bottom);
                
                // Apply the bounds to the webview controller
                    RECT rel_rect;
                    rel_rect.left = 0;
                    rel_rect.top = 0;
                    rel_rect.right = client_rect.right - client_rect.left;
                    rel_rect.bottom = client_rect.bottom - client_rect.top;
                    BuiltinBrowserLog(L"容器内相对 bounds = (0,0,%d,%d)", rel_rect.right, rel_rect.bottom);
                    HRESULT hr = impl->controller->put_Bounds(rel_rect);
                BuiltinBrowserLog(L"STA线程 put_Bounds hr=0x%08X", hr);
            }
        } else if (msg.message == WM_GO_BACK) {
            if (impl->webview) {
                BuiltinBrowserLog(L"STA线程 执行 GoBack");
                BOOL can_go_back = FALSE;
                impl->webview->get_CanGoBack(&can_go_back);
                if (can_go_back) {
                    impl->webview->GoBack();
                } else {
                    BuiltinBrowserLog(L"STA线程 Cannot go back");
                }
            }
        } else if (msg.message == WM_GO_FORWARD) {
            if (impl->webview) {
                BuiltinBrowserLog(L"STA线程 执行 GoForward");
                BOOL can_go_forward = FALSE;
                impl->webview->get_CanGoForward(&can_go_forward);
                if (can_go_forward) {
                    impl->webview->GoForward();
                } else {
                    BuiltinBrowserLog(L"STA线程 Cannot go forward");
                }
            }
        } else if (msg.message == WM_RELOAD) {
            if (impl->webview) {
                BuiltinBrowserLog(L"STA线程 执行 Reload");
                impl->webview->Reload();
            }
        }
    }
    
    // Clean up COM
    CoUninitialize();
    BuiltinBrowserLog(L"STA线程 退出，COM已清理");
    
    return 0;
}

bool BuiltinBrowser::Open(const wchar_t* url, HWND parent) {
    BuiltinBrowserLog(L"Open(url=%ls) 进入；parent=%p IsWindow=%d", url ? url : L"(null)", (void*)parent, parent ? IsWindow(parent) : 0);
    
    if (!parent || !IsWindow(parent)) {
        return false;
    }
    
    if (!impl_) {
        impl_ = new BuiltinBrowserImpl();
    }
    
    // Store the parent window
    impl_->parent_hwnd = parent;
    
    // If webview is already open, navigate to the new URL
    if (impl_->is_open.load() && impl_->thread_running.load()) {
        // Update the URL to navigate to
        {
            std::lock_guard<std::mutex> lock(impl_->url_mutex);
            impl_->current_url = std::wstring(url);
        }
        
        // Post message to navigate to the new URL
        if (impl_->thread_id.load() != 0) {
            PostThreadMessage(impl_->thread_id.load(), WM_NAVIGATE_URL, 0, 0);
            return true;
        }
    }
    
    // If STA thread is not running, start it
    if (!impl_->thread_running.load()) {
        // Store the URL to navigate to
        {
            std::lock_guard<std::mutex> lock(impl_->url_mutex);
            impl_->current_url = std::wstring(url);
        }
        
        // Start the STA thread
        HANDLE thread_handle = CreateThread(NULL, 0, StaThreadFunc, impl_, 0, NULL);
        if (thread_handle) {
            CloseHandle(thread_handle); // We don't need the handle
            BuiltinBrowserLog(L"主线程 已启动 STA 线程");
            
            // Wait briefly for the thread to initialize
            Sleep(100);
            
            // Post message to open the URL
            if (impl_->thread_id.load() != 0) {
                PostThreadMessage(impl_->thread_id.load(), WM_OPEN_URL, 0, 0);
                return true;
            } else {
                BuiltinBrowserLog(L"主线程 线程ID未设置，发送消息失败");
                return false;
            }
        } else {
            BuiltinBrowserLog(L"主线程 创建STA线程失败");
            // Fallback to system browser
            BuiltinBrowserLog(L"！！回退到系统浏览器（创建STA线程失败）url=%ls", url ? url : L"(null)");
            ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
            return false;
        }
    }
    
    return true;
}

void BuiltinBrowser::Close() {
    if (impl_ && impl_->thread_running && impl_->thread_id.load() != 0) {
        // Post a close message to the STA thread
        PostThreadMessage(impl_->thread_id.load(), WM_CLOSE_BROWSER, 0, 0);
    }
}

bool BuiltinBrowser::IsOpen() const {
    return impl_ ? impl_->is_open.load() : false;
}

// Destructor to ensure cleanup
BuiltinBrowser::~BuiltinBrowser() {
    if (impl_ && impl_->thread_running && impl_->thread_id.load() != 0) {
        // Post a close message to the STA thread
        PostThreadMessage(impl_->thread_id.load(), WM_CLOSE_BROWSER, 0, 0);
        
        // Wait briefly for the thread to finish
        if (impl_->sta_thread.joinable()) {
            impl_->sta_thread.join();
        }
    }
    
    if (impl_) {
        delete impl_;
        impl_ = nullptr;
    }
}

void BuiltinBrowser::SetContentRect(const RECT& r) {
    if (!impl_) {
        BuiltinBrowserLog(L"SetContentRect: impl_ 尚未创建，忽略");
        return;
    }
    impl_->content_rect = r;
    BuiltinBrowserLog(L"SetContentRect 已记录矩形 (%d,%d,%d,%d)", r.left, r.top, r.right, r.bottom);
    if (!impl_->thread_running.load() || impl_->thread_id.load() == 0) {
        BuiltinBrowserLog(L"SetContentRect: 线程未就绪，稍后由控制器就绪时补发 ✓");
        return;
    }
    
    BuiltinBrowserLog(L"SetContentRect: (%d,%d,%d,%d)", r.left, r.top, r.right, r.bottom);
    
    // Store the content rectangle in the implementation
    impl_->content_rect = r;
    
    // Post message to the STA thread to update the bounds
    PostThreadMessage(impl_->thread_id.load(), WM_SET_CONTENT_RECT, 0, 0);
}

void BuiltinBrowser::GoBack() {
    if (!impl_ || !impl_->thread_running.load() || impl_->thread_id.load() == 0) {
        BuiltinBrowserLog(L"GoBack: Browser not initialized or thread not running");
        return;
    }
    
    BuiltinBrowserLog(L"GoBack called");
    
    // Post message to the STA thread to navigate back
    PostThreadMessage(impl_->thread_id.load(), WM_GO_BACK, 0, 0);
}

void BuiltinBrowser::GoForward() {
    if (!impl_ || !impl_->thread_running.load() || impl_->thread_id.load() == 0) {
        BuiltinBrowserLog(L"GoForward: Browser not initialized or thread not running");
        return;
    }
    
    BuiltinBrowserLog(L"GoForward called");
    
    // Post message to the STA thread to navigate forward
    PostThreadMessage(impl_->thread_id.load(), WM_GO_FORWARD, 0, 0);
}

void BuiltinBrowser::Reload() {
    if (!impl_ || !impl_->thread_running.load() || impl_->thread_id.load() == 0) {
        BuiltinBrowserLog(L"Reload: Browser not initialized or thread not running");
        return;
    }
    
    BuiltinBrowserLog(L"Reload called");
    
    // Post message to the STA thread to reload the page
    PostThreadMessage(impl_->thread_id.load(), WM_RELOAD, 0, 0);
}

std::wstring BuiltinBrowser::GetTitle() {
    if (!impl_) {
        BuiltinBrowserLog(L"GetTitle: Browser not initialized");
        return std::wstring();
    }
    
    // Lock the mutex to safely access the title
    std::lock_guard<std::mutex> lock(impl_->title_mutex);
    std::wstring title = impl_->current_title;
    BuiltinBrowserLog(L"GetTitle: returning '%ls'", title.c_str());
    return title;
}
#endif