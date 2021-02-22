#include "PeripheralSwitchUs421a.h"
#include <Windows.h>
#include <hidsdi.h>
#include <SetupAPI.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

std::string GetErrorMessage(DWORD error_code) {
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

std::string StatusUs421a::ToString() {
    std::stringstream s;
    s << "Raw Status (hex): " << hex << setfill('0') << std::setw(2);
    s << std::setw(2) << static_cast<int>(raw_status_[0]) << " ";
    s << std::setw(2) << static_cast<int>(raw_status_[1]) << " ";
    s << std::setw(2) << static_cast<int>(raw_status_[2]) << " ";
    s << std::setw(2) << static_cast<int>(raw_status_[3]) << endl;

    s << "SelfSelected:" << self_selected();
    s << " SelfLock:" << self_locked();
    s << " SwitchRequested:" << switch_requested();
    s << " Beeper:" << beeper_enabled();

    return s.str();
}

PeripheralSwitchUs421a::PeripheralSwitchUs421a(std::wstring path) : path_{path} {
    file_handle_ = CreateFile(path_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    DWORD error_code_create_file = GetLastError();

    if (error_code_create_file != 0) {
        throw runtime_error("Failed to connect to device. " + GetErrorMessage(error_code_create_file));
    }
}

PeripheralSwitchUs421a::~PeripheralSwitchUs421a() {
    if (file_handle_ != 0) {
        CloseHandle(file_handle_);
        file_handle_ = nullptr;
    }
}

StatusUs421a PeripheralSwitchUs421a::ReadStatus() {
    std::array<uint8_t, 4> read_buffer;
    DWORD bytes_read;
    const BOOL read_ok = ReadFile(file_handle_, read_buffer.data(), 4, &bytes_read, nullptr);
    if (!read_ok) {
        DWORD error_code = GetLastError();
        throw runtime_error("Failed to read from device. " + GetErrorMessage(error_code));
    }

    StatusUs421a s{read_buffer};
    return s;
}

void PeripheralSwitchUs421a::SendCommand(uint8_t* buf_size_2) {
    DWORD bytes_written;
    const BOOL write_ok = WriteFile(file_handle_, buf_size_2, 2, &bytes_written, nullptr);
    if (!write_ok) {
        DWORD error_code = GetLastError();
        throw runtime_error("Failed to send command to device. " + GetErrorMessage(error_code));
    }
}

void PeripheralSwitchUs421a::Select() {
    uint8_t cmd[]{0x02, 0x11};
    SendCommand(cmd);
}

void PeripheralSwitchUs421a::CancelSwitchRequest() {
    uint8_t cmd[]{0x02, 0x10};
    SendCommand(cmd);
}

void PeripheralSwitchUs421a::Lock() {
    uint8_t cmd[]{0x02, 0x21};
    SendCommand(cmd);
}

void PeripheralSwitchUs421a::Unlock() {
    uint8_t cmd[]{0x02, 0x20};
    SendCommand(cmd);
}

void PeripheralSwitchUs421a::LockKeepAlive() {
    uint8_t cmd[]{0x02, 0x40};
    SendCommand(cmd);
}

std::vector<std::wstring> PeripheralSwitchUs421a::GetDeviceList() {
    vector<wstring> device_list;
    GUID hid_guid;
    HidD_GetHidGuid(&hid_guid);

    auto dev_info_handle = SetupDiGetClassDevs(&hid_guid, nullptr, nullptr, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    int dev_index = 0;
    SP_DEVICE_INTERFACE_DATA dev_if_data{};
    dev_if_data.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    while (SetupDiEnumDeviceInterfaces(dev_info_handle, nullptr, &hid_guid, dev_index, &dev_if_data)) {
        DWORD required_size = 0;
        SetupDiGetDeviceInterfaceDetail(dev_info_handle, &dev_if_data, nullptr, 0, &required_size, nullptr);

        PSP_DEVICE_INTERFACE_DETAIL_DATA dev_if_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, required_size);
        dev_if_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        SetupDiGetDeviceInterfaceDetail(dev_info_handle, &dev_if_data, dev_if_detail, required_size, &required_size, nullptr);
        wstring device_path = dev_if_detail->DevicePath;
        device_list.push_back(device_path);

        LocalFree(dev_if_detail);
        ++dev_index;
    }

    SetupDiDestroyDeviceInfoList(dev_info_handle);

    // Filter for Aten US421A devices
    vector<wstring> us421a_devices;
    copy_if(device_list.begin(), device_list.end(), back_inserter(us421a_devices),
            [](wstring candidate) { return candidate.find(L"vid_0557&pid_2406&mi_01") != wstring::npos; });

    return us421a_devices;
}
