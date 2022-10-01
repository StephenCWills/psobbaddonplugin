#include "imgui_dinput8_dev.h"

#include "imgui/imgui.h"
#include "log.h"

STDMETHODIMP ImguiDInput::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    return inner->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) ImguiDInput::AddRef(void)
{
    return inner->AddRef();
}

STDMETHODIMP_(ULONG) ImguiDInput::Release(void)
{
    return inner->Release();
}

STDMETHODIMP ImguiDInput::CreateDevice(REFGUID guid, LPDIRECTINPUTDEVICE8A * devptr, LPUNKNOWN unk)
{
    auto ret = inner->CreateDevice(guid, devptr, unk);
    if (guid == GUID_SysKeyboard) {
        *devptr = (LPDIRECTINPUTDEVICE8A) new ImguiDInputDevice(*devptr, true);
    }
    if (guid == GUID_SysMouse) {
        *devptr = (LPDIRECTINPUTDEVICE8A) new ImguiDInputDevice(*devptr, false);
    }

    bool match =
        guid.Data1 == 0xEC77CE70 &&
        guid.Data2 == 0xC687 &&
        guid.Data3 == 0x11E9 &&
        guid.Data4[0] == 0x80 &&
        guid.Data4[1] == 0x01 &&
        guid.Data4[2] == 0x44 &&
        guid.Data4[3] == 0x45 &&
        guid.Data4[4] == 0x53 &&
        guid.Data4[5] == 0x54 &&
        guid.Data4[6] == 0x00 &&
        guid.Data4[7] == 0x00;

    if (match) {
        *devptr = (LPDIRECTINPUTDEVICE8A) new ImguiDInputDeviceDS3(guid, inner, *devptr);
    }

    return ret;
}

STDMETHODIMP ImguiDInput::EnumDevices(DWORD arg1, LPDIENUMDEVICESCALLBACKA arg2, LPVOID arg3, DWORD arg4)
{
    return inner->EnumDevices(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInput::GetDeviceStatus(REFGUID guid)
{
    return inner->GetDeviceStatus(guid);
}

STDMETHODIMP ImguiDInput::RunControlPanel(HWND hwnd, DWORD arg2)
{
    return inner->RunControlPanel(hwnd, arg2);
}

STDMETHODIMP ImguiDInput::Initialize(HINSTANCE arg1, DWORD arg2)
{
    return inner->Initialize(arg1, arg2);
}

STDMETHODIMP ImguiDInput::FindDevice(REFGUID guid, LPCSTR arg2, LPGUID arg3)
{
    return inner->FindDevice(guid, arg2, arg3);
}

STDMETHODIMP ImguiDInput::EnumDevicesBySemantics(LPCSTR arg1, LPDIACTIONFORMATA arg2, LPDIENUMDEVICESBYSEMANTICSCBA arg3, LPVOID arg4, DWORD arg5)
{
    return inner->EnumDevicesBySemantics(arg1, arg2, arg3, arg4, arg5);
}

STDMETHODIMP ImguiDInput::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK arg1, LPDICONFIGUREDEVICESPARAMSA arg2, DWORD arg3, LPVOID arg4)
{
    return inner->ConfigureDevices(arg1, arg2, arg3, arg4);
}


// -----------------------------------------------------------------------


STDMETHODIMP ImguiDInputDevice::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    return device->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) ImguiDInputDevice::AddRef(void)
{
    return device->AddRef();
}

STDMETHODIMP_(ULONG) ImguiDInputDevice::Release(void)
{
    return device->Release();
}

STDMETHODIMP ImguiDInputDevice::GetCapabilities(LPDIDEVCAPS caps)
{
    return device->GetCapabilities(caps);
}

STDMETHODIMP ImguiDInputDevice::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA arg1, LPVOID arg2, DWORD arg3)
{
    return device->EnumObjects(arg1, arg2 ,arg3);
}

STDMETHODIMP ImguiDInputDevice::GetProperty(REFGUID arg1, LPDIPROPHEADER arg2)
{
    return device->GetProperty(arg1, arg2);
}

STDMETHODIMP ImguiDInputDevice::SetProperty(REFGUID arg1, LPCDIPROPHEADER arg2)
{
    return device->SetProperty(arg1, arg2);
}

STDMETHODIMP ImguiDInputDevice::Acquire(void)
{
    return device->Acquire();
}

STDMETHODIMP ImguiDInputDevice::Unacquire(void)
{
    return device->Unacquire();
}

STDMETHODIMP ImguiDInputDevice::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
    // PSO uses device state... and doesn't read the whole input buffer with keyboard...
    // who ported this damn game
    auto ret = device->GetDeviceState(cbData, lpvData);
    if (ret) {
        return ret;
    }

    auto io = ImGui::GetIO();

    if (isKeyboard) {
        // is keyboard

        BYTE* keys = (BYTE*) lpvData;

        if (io.WantCaptureKeyboard) {
            // all keys up
            for (unsigned int i = 0; i < cbData; i++) {
                keys[i] = 0;
            }
        }
    }
    else {
        // is mouse

        DIMOUSESTATE2* state = (DIMOUSESTATE2*) lpvData;

        if (io.WantCaptureMouse) {
            // all buttons up
            state->rgbButtons[0] = 0;
            state->rgbButtons[1] = 0;
            state->rgbButtons[2] = 0;
            state->rgbButtons[3] = 0;
            state->lZ = 0;
        }
    }

    return ret;
}

STDMETHODIMP ImguiDInputDevice::GetDeviceData(DWORD arg1, LPDIDEVICEOBJECTDATA arg2, LPDWORD arg3, DWORD arg4)
{
    return device->GetDeviceData(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDevice::SetDataFormat(LPCDIDATAFORMAT arg1)
{
    return device->SetDataFormat(arg1);
}

STDMETHODIMP ImguiDInputDevice::SetEventNotification(HANDLE arg1)
{
    return device->SetEventNotification(arg1);
}

STDMETHODIMP ImguiDInputDevice::SetCooperativeLevel(HWND arg1, DWORD arg2)
{
    return device->SetCooperativeLevel(arg1, arg2);
}

STDMETHODIMP ImguiDInputDevice::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA arg1, DWORD arg2, DWORD arg3)
{
    return device->GetObjectInfo(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDevice::GetDeviceInfo(LPDIDEVICEINSTANCEA arg1)
{
    return device->GetDeviceInfo(arg1);
}

STDMETHODIMP ImguiDInputDevice::RunControlPanel(HWND arg1, DWORD arg2)
{
    return device->RunControlPanel(arg1, arg2);
}

STDMETHODIMP ImguiDInputDevice::Initialize(HINSTANCE arg1, DWORD arg2, REFGUID arg3)
{
    return device->Initialize(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDevice::CreateEffect(REFGUID arg1, LPCDIEFFECT arg2, LPDIRECTINPUTEFFECT * arg3, LPUNKNOWN arg4)
{
    return device->CreateEffect(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDevice::EnumEffects(LPDIENUMEFFECTSCALLBACKA arg1, LPVOID arg2, DWORD arg3)
{
    return device->EnumEffects(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDevice::GetEffectInfo(LPDIEFFECTINFOA arg1, REFGUID arg2)
{
    return device->GetEffectInfo(arg1, arg2);
}

STDMETHODIMP ImguiDInputDevice::GetForceFeedbackState(LPDWORD arg1)
{
    return device->GetForceFeedbackState(arg1);
}

STDMETHODIMP ImguiDInputDevice::SendForceFeedbackCommand(DWORD arg1)
{
    return device->SendForceFeedbackCommand(arg1);
}

STDMETHODIMP ImguiDInputDevice::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK arg1, LPVOID arg2, DWORD arg3)
{
    return device->EnumCreatedEffectObjects(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDevice::Escape(LPDIEFFESCAPE arg1)
{
    return device->Escape(arg1);
}

STDMETHODIMP ImguiDInputDevice::Poll(void)
{
    return device->Poll();
}

STDMETHODIMP ImguiDInputDevice::SendDeviceData(DWORD arg1, LPCDIDEVICEOBJECTDATA arg2, LPDWORD arg3, DWORD arg4)
{
    return device->SendDeviceData(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDevice::EnumEffectsInFile(LPCSTR arg1, LPDIENUMEFFECTSINFILECALLBACK arg2, LPVOID arg3, DWORD arg4)
{
    return device->EnumEffectsInFile(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDevice::WriteEffectToFile(LPCSTR arg1, DWORD arg2, LPDIFILEEFFECT arg3, DWORD arg4)
{
    return device->WriteEffectToFile(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDevice::BuildActionMap(LPDIACTIONFORMATA arg1, LPCSTR arg2, DWORD arg3)
{
    return device->BuildActionMap(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDevice::SetActionMap(LPDIACTIONFORMATA arg1, LPCSTR arg2, DWORD arg3)
{
    return device->SetActionMap(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDevice::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA arg1)
{
    return device->GetImageInfo(arg1);
}


// -----------------------------------------------------------------------


STDMETHODIMP ImguiDInputDeviceDS3::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if (device == nullptr)
        return -2147220983;

    return device->QueryInterface(riid, ppvObj);
}

STDMETHODIMP_(ULONG) ImguiDInputDeviceDS3::AddRef(void)
{
    if (device == nullptr)
        return -2147220983;

    return device->AddRef();
}

STDMETHODIMP_(ULONG) ImguiDInputDeviceDS3::Release(void)
{
    if (device == nullptr)
        return -2147220983;

    return device->Release();
}

STDMETHODIMP ImguiDInputDeviceDS3::GetCapabilities(LPDIDEVCAPS caps)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetCapabilities(caps);
}

STDMETHODIMP ImguiDInputDeviceDS3::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA arg1, LPVOID arg2, DWORD arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->EnumObjects(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetProperty(REFGUID arg1, LPDIPROPHEADER arg2)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetProperty(arg1, arg2);
}

STDMETHODIMP ImguiDInputDeviceDS3::SetProperty(REFGUID arg1, LPCDIPROPHEADER arg2)
{
    if (device == nullptr)
        return -2147220983;

    return device->SetProperty(arg1, arg2);
}

STDMETHODIMP ImguiDInputDeviceDS3::Acquire(void)
{
    if (device == nullptr)
        return -2147220983;

    return device->Acquire();
}

STDMETHODIMP ImguiDInputDeviceDS3::Unacquire(void)
{
    if (device == nullptr)
        return -2147220983;

    return device->Unacquire();
}

STDMETHODIMP ImguiDInputDeviceDS3::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetDeviceState(cbData, lpvData);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetDeviceData(DWORD arg1, LPDIDEVICEOBJECTDATA arg2, LPDWORD arg3, DWORD arg4)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetDeviceData(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDeviceDS3::SetDataFormat(LPCDIDATAFORMAT arg1)
{
    if (device == nullptr)
        return -2147220983;

    dataFormat = arg1;
    return device->SetDataFormat(arg1);
}

STDMETHODIMP ImguiDInputDeviceDS3::SetEventNotification(HANDLE arg1)
{
    if (device == nullptr)
        return -2147220983;

    return device->SetEventNotification(arg1);
}

STDMETHODIMP ImguiDInputDeviceDS3::SetCooperativeLevel(HWND arg1, DWORD arg2)
{
    if (device == nullptr)
        return -2147220983;

    cooperativeLevelWnd = arg1;
    cooperativeLevelWord = arg2;
    return device->SetCooperativeLevel(arg1, arg2);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA arg1, DWORD arg2, DWORD arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetObjectInfo(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetDeviceInfo(LPDIDEVICEINSTANCEA arg1)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetDeviceInfo(arg1);
}

STDMETHODIMP ImguiDInputDeviceDS3::RunControlPanel(HWND arg1, DWORD arg2)
{
    if (device == nullptr)
        return -2147220983;

    return device->RunControlPanel(arg1, arg2);
}

STDMETHODIMP ImguiDInputDeviceDS3::Initialize(HINSTANCE arg1, DWORD arg2, REFGUID arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->Initialize(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::CreateEffect(REFGUID arg1, LPCDIEFFECT arg2, LPDIRECTINPUTEFFECT* arg3, LPUNKNOWN arg4)
{
    if (device == nullptr)
        return -2147220983;

    return device->CreateEffect(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDeviceDS3::EnumEffects(LPDIENUMEFFECTSCALLBACKA arg1, LPVOID arg2, DWORD arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->EnumEffects(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetEffectInfo(LPDIEFFECTINFOA arg1, REFGUID arg2)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetEffectInfo(arg1, arg2);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetForceFeedbackState(LPDWORD arg1)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetForceFeedbackState(arg1);
}

STDMETHODIMP ImguiDInputDeviceDS3::SendForceFeedbackCommand(DWORD arg1)
{
    if (device == nullptr)
        return -2147220983;

    return device->SendForceFeedbackCommand(arg1);
}

STDMETHODIMP ImguiDInputDeviceDS3::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK arg1, LPVOID arg2, DWORD arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->EnumCreatedEffectObjects(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::Escape(LPDIEFFESCAPE arg1)
{
    if (device == nullptr)
        return -2147220983;

    return device->Escape(arg1);
}

STDMETHODIMP ImguiDInputDeviceDS3::Poll(void)
{
    auto ret = -2147220983;

    if (device != nullptr) {
        ret = device->Poll();

        if (ret == -2147220983) {
            device->Unacquire();
            device->Release();
            device = nullptr;
        }
    }

    if (device == nullptr) {
        LPDIRECTINPUTDEVICE8A newDevice;
        auto devret = dinput->CreateDevice(deviceGuid, &newDevice, nullptr);

        if (devret == DI_OK)
        {
            newDevice->AddRef();
            newDevice->SetDataFormat(dataFormat);
            newDevice->SetCooperativeLevel(cooperativeLevelWnd, cooperativeLevelWord);
            newDevice->Acquire();
            device = newDevice;
            ret = device->Poll();
        }
    }

    return ret;
}

STDMETHODIMP ImguiDInputDeviceDS3::SendDeviceData(DWORD arg1, LPCDIDEVICEOBJECTDATA arg2, LPDWORD arg3, DWORD arg4)
{
    if (device == nullptr)
        return -2147220983;

    return device->SendDeviceData(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDeviceDS3::EnumEffectsInFile(LPCSTR arg1, LPDIENUMEFFECTSINFILECALLBACK arg2, LPVOID arg3, DWORD arg4)
{
    if (device == nullptr)
        return -2147220983;

    return device->EnumEffectsInFile(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDeviceDS3::WriteEffectToFile(LPCSTR arg1, DWORD arg2, LPDIFILEEFFECT arg3, DWORD arg4)
{
    if (device == nullptr)
        return -2147220983;

    return device->WriteEffectToFile(arg1, arg2, arg3, arg4);
}

STDMETHODIMP ImguiDInputDeviceDS3::BuildActionMap(LPDIACTIONFORMATA arg1, LPCSTR arg2, DWORD arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->BuildActionMap(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::SetActionMap(LPDIACTIONFORMATA arg1, LPCSTR arg2, DWORD arg3)
{
    if (device == nullptr)
        return -2147220983;

    return device->SetActionMap(arg1, arg2, arg3);
}

STDMETHODIMP ImguiDInputDeviceDS3::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA arg1)
{
    if (device == nullptr)
        return -2147220983;

    return device->GetImageInfo(arg1);
}
