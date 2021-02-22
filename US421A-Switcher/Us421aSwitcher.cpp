// This file defines the entry point for the console application.
//

#include "PeripheralSwitchUs421a.h"
#include <iostream>
#include <thread>

using namespace std;

PeripheralSwitchUs421a GetUsbSwitch() {
    auto list = PeripheralSwitchUs421a::GetDeviceList();

    if (list.empty()) {
        cout << "No Aten US421A device found" << endl;
        throw runtime_error("No device found");
    }

    return list.at(0);
}

void SwitchToThisPC() {
    {
        PeripheralSwitchUs421a usb_switch = GetUsbSwitch();
        const auto status = usb_switch.ReadStatus();
        if (!status.self_selected()) {
            usb_switch.Select();
            cout << "Switching to this host" << endl;
        }
        else {
            cout << "No switching needed - this host is already the owner" << endl;
            return;
        }
    }

    this_thread::sleep_for(1s);

    for (int i = 0; i < 10; ++i) {
        auto list = PeripheralSwitchUs421a::GetDeviceList();
        if (!list.empty()) {
            PeripheralSwitchUs421a usb_switch = GetUsbSwitch();
            if (!usb_switch.ReadStatus().self_selected()) {
                throw runtime_error("Cannot switch to this host");
            }

            cout << "Switched to this host successfully" << endl;
            return; // Everything ok
        }
        this_thread::sleep_for(1s);
    }
    throw runtime_error("Lost connection to switch after switching to this host");
}

void SwitchAndKeepLocked() {
    SwitchToThisPC();

    PeripheralSwitchUs421a usb_switch = GetUsbSwitch();
    cout << "Locking" << endl;
    usb_switch.Lock();

    cout << "Keeping lock. Use Ctrl-C to unlock and terminate" << endl;
    while (true) {
        this_thread::sleep_for(4s);
        usb_switch.LockKeepAlive();
    }
}

int main(int argc, const char* argv[]) {
    cout << "US421A-Switcher: A simple command line utility to control the Aten US421A USB 2.0 Peripheral Switch" << endl;
    cout << "Version 0.1" << endl << endl;

    try {
        if (argc == 1) {
            SwitchToThisPC();
        }
        else if (argc == 2 && string{argv[1]} == "--lock") {
            SwitchAndKeepLocked();
        }
        else if (argc == 2 && string{argv[1]} == "--cancel") {
            auto usb_switch = GetUsbSwitch();
            usb_switch.CancelSwitchRequest();
        }
        else if (argc == 2 && string{argv[1]} == "--status") {
            auto usb_switch = GetUsbSwitch();
            auto status = usb_switch.ReadStatus();
            cout << status.ToString() << endl;
        }
        else if (argc == 2 && string{argv[1]} == "--statusloop") {
            while (true) {
                try {
                    auto usb_switch = GetUsbSwitch();
                    auto status = usb_switch.ReadStatus();
                    cout << status.ToString() << endl;
                }
                catch (const std::exception& ex) {
                    cout << "Exception: " << ex.what() << endl;
                }
                this_thread::sleep_for(1s);
            }
        }
        else {
            cout << "Usage:" << endl;
            cout << "If no arguments are present, will request a switch to this host. If the US421A" << endl;
            cout << "is not locked by a foreign host, the switch will be executed before this utility" << endl;
            cout << "terminates. If a foreign host keeps a lock, the utility terminates immediately," << endl;
            cout << "and the request will be executed upon unlock from the foreign host" << endl;
            cout << endl;
            cout << "Options:" << endl;
            cout << "--lock       switches to this host and keeps a lock until the utility is terminated" << endl;
            cout << "--cancel     cancels a switch request" << endl;
            cout << "--status     shows device status" << endl;
            cout << "--statusloop shows device status continuously" << endl;
            cout << "" << endl;
        }

    }
    catch (const std::exception& ex) {
        cout << "Exception: " << ex.what() << endl;
        return 1;
    }
}
