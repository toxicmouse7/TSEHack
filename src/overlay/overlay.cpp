#include "overlay/overlay.hpp"

#include <MinHook.h>
#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

#include "core/cheat_state.hpp"
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    using Present = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
    using ResizeBuffers = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

    Present original_present{};
    ResizeBuffers original_resize_buffers{};
    LPVOID present_target{};
    LPVOID resize_buffers_target{};
    WNDPROC original_wndproc{};
    HWND game_window{};
    ID3D11Device* device{};
    ID3D11DeviceContext* device_context{};
    ID3D11RenderTargetView* render_target_view{};
    core::CheatState* state{};
    bool imgui_initialized = false;
    bool menu_visible = true;

    void release_render_target()
    {
        if (render_target_view)
        {
            render_target_view->Release();
            render_target_view = nullptr;
        }
    }

    void create_render_target(IDXGISwapChain* swap_chain)
    {
        ID3D11Texture2D* back_buffer{};
        if (SUCCEEDED(swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))))
        {
            device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
            back_buffer->Release();
        }
    }

    LRESULT WINAPI hooked_wndproc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
    {
        if (message == WM_KEYUP && w_param == VK_INSERT)
        {
            menu_visible = !menu_visible;
            return 0;
        }
        if (menu_visible && imgui_initialized && ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
        {
            return 1;
        }
        return CallWindowProc(original_wndproc, window, message, w_param, l_param);
    }

    bool initialize_imgui(IDXGISwapChain* swap_chain)
    {
        DXGI_SWAP_CHAIN_DESC description{};
        if (FAILED(swap_chain->GetDesc(&description)) || description.OutputWindow == nullptr
            || FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&device))))
        {
            return false;
        }
        device->GetImmediateContext(&device_context);
        if (!device_context)
        {
            device->Release();
            device = nullptr;
            return false;
        }

        game_window = description.OutputWindow;
        original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(
            game_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hooked_wndproc)));
        if (!original_wndproc)
        {
            device_context->Release(); device_context = nullptr;
            device->Release(); device = nullptr;
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        if (!ImGui_ImplWin32_Init(game_window) || !ImGui_ImplDX11_Init(device, device_context))
        {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            SetWindowLongPtr(game_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_wndproc));
            original_wndproc = nullptr; game_window = nullptr;
            device_context->Release(); device_context = nullptr;
            device->Release(); device = nullptr;
            return false;
        }
        create_render_target(swap_chain);
        imgui_initialized = true;
        return true;
    }

    HRESULT WINAPI hooked_present(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags)
    {
        if (!imgui_initialized && !initialize_imgui(swap_chain))
        {
            return original_present(swap_chain, sync_interval, flags);
        }
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if (menu_visible)
        {
            ImGui::Begin("Showdown hack", &menu_visible);
            bool enabled = state->target_assist_enabled.load();
            if (ImGui::Checkbox("Enable target assist", &enabled))
            {
                state->target_assist_enabled.store(enabled);
            }
            bool esp_enabled = state->esp_enabled.load();
            if (ImGui::Checkbox("Enable ESP", &esp_enabled))
            {
                state->esp_enabled.store(esp_enabled);
            }
            ImGui::TextUnformatted("Press Insert to toggle this window.");
            ImGui::End();
        }
        if (state->esp_enabled.load())
        {
            const auto boxes = state->esp_boxes();
            const auto draw_list = ImGui::GetForegroundDrawList();
            for (const auto& box : boxes)
            {
                draw_list->AddRect(ImVec2(box.left, box.top), ImVec2(box.right, box.bottom),
                                   IM_COL32(0, 255, 0, 255), 0.0F, 0, 1.5F);
            }
        }
        ImGui::Render();
        if (render_target_view)
        {
            device_context->OMSetRenderTargets(1, &render_target_view, nullptr);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        }
        return original_present(swap_chain, sync_interval, flags);
    }

    HRESULT WINAPI hooked_resize_buffers(IDXGISwapChain* swap_chain, UINT count, UINT width, UINT height,
                                         DXGI_FORMAT format, UINT swap_chain_flags)
    {
        if (imgui_initialized) release_render_target();
        const auto result = original_resize_buffers(swap_chain, count, width, height, format, swap_chain_flags);
        if (imgui_initialized && SUCCEEDED(result)) create_render_target(swap_chain);
        return result;
    }
}

bool overlay::install(core::CheatState& cheat_state)
{
    state = &cheat_state;
    const auto dummy_window = CreateWindowEx(0, TEXT("STATIC"), TEXT(""), WS_OVERLAPPED, 0, 0, 100, 100,
                                             nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!dummy_window) return false;

    DXGI_SWAP_CHAIN_DESC description{};
    description.BufferCount = 1;
    description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.OutputWindow = dummy_window;
    description.SampleDesc.Count = 1;
    description.Windowed = TRUE;
    description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* swap_chain{};
    ID3D11Device* dummy_device{};
    ID3D11DeviceContext* dummy_context{};
    const auto result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, &description, &swap_chain, &dummy_device, nullptr, &dummy_context);
    if (FAILED(result)) { DestroyWindow(dummy_window); return false; }

    const auto vtable = *reinterpret_cast<void***>(swap_chain);
    present_target = vtable[8];
    resize_buffers_target = vtable[13];
    const auto present_created = MH_CreateHook(present_target, reinterpret_cast<LPVOID>(hooked_present),
        reinterpret_cast<LPVOID*>(&original_present)) == MH_OK;
    const auto resize_created = present_created && MH_CreateHook(resize_buffers_target,
        reinterpret_cast<LPVOID>(hooked_resize_buffers), reinterpret_cast<LPVOID*>(&original_resize_buffers)) == MH_OK;
    const auto present_enabled = resize_created && MH_EnableHook(present_target) == MH_OK;
    const auto resize_enabled = present_enabled && MH_EnableHook(resize_buffers_target) == MH_OK;
    dummy_context->Release(); dummy_device->Release(); swap_chain->Release(); DestroyWindow(dummy_window);
    if (resize_enabled) return true;

    if (present_target) { MH_DisableHook(present_target); MH_RemoveHook(present_target); }
    if (resize_buffers_target) { MH_DisableHook(resize_buffers_target); MH_RemoveHook(resize_buffers_target); }
    original_present = nullptr; original_resize_buffers = nullptr;
    present_target = nullptr; resize_buffers_target = nullptr; state = nullptr;
    return false;
}

void overlay::remove()
{
    if (imgui_initialized)
    {
        ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext(); imgui_initialized = false;
    }
    release_render_target();
    if (original_wndproc && game_window)
    {
        SetWindowLongPtr(game_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_wndproc));
    }
    original_wndproc = nullptr; game_window = nullptr;
    if (device_context) { device_context->Release(); device_context = nullptr; }
    if (device) { device->Release(); device = nullptr; }
    if (present_target) { MH_DisableHook(present_target); MH_RemoveHook(present_target); }
    if (resize_buffers_target) { MH_DisableHook(resize_buffers_target); MH_RemoveHook(resize_buffers_target); }
    original_present = nullptr; original_resize_buffers = nullptr;
    present_target = nullptr; resize_buffers_target = nullptr; state = nullptr;
}
