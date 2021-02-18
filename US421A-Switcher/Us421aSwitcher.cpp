// This file defines the entry point for the console application.
//

#include "PeripherialSwitchUs421a.h"
#include <iostream>
#include <thread>

using namespace std;

int RunTest() {

  auto list = PeripherialSwitchUs421a::GetDeviceList();

  if (list.empty()) {
    // No device found
    return 1;
  }
  {
    PeripherialSwitchUs421a usb_switch(list.at(0));
    usb_switch.Select();
  }

  this_thread::sleep_for(5s);
  PeripherialSwitchUs421a usb_switch(PeripherialSwitchUs421a::GetDeviceList().at(0));
  auto stat = usb_switch.ReadStatus();
  cout << stat.ToString() << endl;

  usb_switch.Lock(); cout << "Lock" << endl;

  for (size_t i = 0; i < 5; i++)
  {
    auto stat = usb_switch.ReadStatus();
    cout << stat.ToString() << endl;

    usb_switch.LockKeepAlive();
    this_thread::sleep_for(1s);
  }


  usb_switch.Unlock();
  cout << "Continuously unlocking" << endl;
  uint8_t cmd[] = { 0x02, 0x41 };

  for (size_t i = 0; i < 5; i++)
  {
    auto stat = usb_switch.ReadStatus();
    cout << stat.ToString() << endl;
    usb_switch.SendCommand(cmd);
    this_thread::sleep_for(1s);
  }


  return 0;

}

void JustObserve() {
  cout << "Just observing..." << endl;
  for (size_t i = 0; i < 5 * 60; i++)
  {
    auto list = PeripherialSwitchUs421a::GetDeviceList();

    if (!list.empty()) {
      try {
        PeripherialSwitchUs421a usb_switch(list.at(0));
        auto stat = usb_switch.ReadStatus();
        cout << stat.ToString() << endl;
      }
      catch (const std::exception& ex) {
        cout << "Exception: " << ex.what() << endl;
      }
    }
    else {
      cout << "No device found" << endl;
    }

    this_thread::sleep_for(1s);
  }
  cout << "Finished" << endl;
}

void JustObserveAvoidEnumeration() {
  cout << "Just observing..." << endl;
  std::unique_ptr<PeripherialSwitchUs421a> dev;
  auto list = PeripherialSwitchUs421a::GetDeviceList();
  if (!list.empty()) {
    dev = make_unique<PeripherialSwitchUs421a>(list.at(0));
  }
  for (size_t i = 0; i < 5 * 60; i++)
  {
    if (!dev) {
      list = PeripherialSwitchUs421a::GetDeviceList();
      if (!list.empty()) {
        dev = make_unique<PeripherialSwitchUs421a>(list.at(0));
      }
    }

    if (dev) {
      try {
        auto stat = dev->ReadStatus();
        cout << stat.ToString() << endl;
      }
      catch (const std::exception& ex) {
        cout << "Exception: " << ex.what() << endl;
        dev.reset();
      }
    }
    else {
      cout << "No device found" << endl;
    }

    this_thread::sleep_for(1s);
  }
  cout << "Finished" << endl;
}

PeripherialSwitchUs421a GetUsbSwitch()
{
  auto list = PeripherialSwitchUs421a::GetDeviceList();

  if (list.empty()) {
    cout << "No Aten US421A device found" << endl;
    throw runtime_error("No device found");
  }

  return list.at(0);
}

void SwitchToThisPC()
{
  {
    PeripherialSwitchUs421a usb_switch = GetUsbSwitch();
    const auto status = usb_switch.ReadStatus();
    if (!status.self_selected()) {
      usb_switch.Select();
      cout << "Switching to this Host" << endl;
    }
    else
    {
      cout << "No switching needed - this host is already the owner" << endl;
      return;
    }
  }

  this_thread::sleep_for(1s);

  for (int i = 0; i < 10; ++i)
  {
    auto list = PeripherialSwitchUs421a::GetDeviceList();
    if (!list.empty())
    {
      PeripherialSwitchUs421a usb_switch = GetUsbSwitch();
      if(!usb_switch.ReadStatus().self_selected())
      {
        throw runtime_error("Cannot switch to this host");
      }

      cout << "Switched to this host successfully" << endl;
      return; // Everything ok
    }
    this_thread::sleep_for(1s);
  }
  throw runtime_error("Lost connection to switch after switching to this host");
}

void SwitchAndKeepLocked()
{
  SwitchToThisPC();

  PeripherialSwitchUs421a usb_switch = GetUsbSwitch();
  cout << "Locking" << endl;
  usb_switch.Lock();

  cout << "Keeping lock. Use Ctrl-C to unlock and terminate" << endl;
  while (true) {
    this_thread::sleep_for(1s);
    usb_switch.LockKeepAlive();
    cout << usb_switch.ReadStatus().ToString() << endl;
  }
}

int main(int argc, const char* argv[])
{
  cout << "US421A-Switcher: A simple command line utility to control the Aten US421A USB 2.0 Peripherial Switch " << endl;
  cout << "Version 0.1" << endl;

  try {
    if (argc == 1)
    {
      SwitchToThisPC();
    } else if(argc == 2 && string{argv[1]} == "/lock")
    {
        SwitchAndKeepLocked();
    }
    else if (argc == 2 && string{ argv[1] } == "/cancel")
    {
      auto usb_switch = GetUsbSwitch();
      usb_switch.CancelSwitchRequest();
    }
    else if (argc == 2 && string{ argv[1] } == "/status")
    {
      auto usb_switch = GetUsbSwitch();
      auto status = usb_switch.ReadStatus();
      cout << status.ToString() << endl;
    }
    else if (argc == 2 && string{ argv[1] } == "/statusloop")
    {
      while (true) {
        try {
          auto usb_switch = GetUsbSwitch();
          auto status = usb_switch.ReadStatus();
          cout << status.ToString() << endl;
        }
        catch (const std::exception& ex)
        {
          cout << "Exception: " << ex.what() << endl;
        }
        this_thread::sleep_for(1s);
      }
    }
    else
    {
      cout << "Usage:" << endl;
      cout << "If executed without arguments, will try to switch to this host" << endl;
      cout << "/lock       switches to this PC and keeps a lock until the utility is terminated" << endl;
      cout << "/cancel     cancels a switch request" << endl;
      cout << "/status     shows device status" << endl;
      cout << "/statusloop shows device status continuously" << endl;
      cout << "" << endl;
    }

    //return RunTest();
    //JustObserve();
    //JustObserveAvoidEnumeration();

  }
  catch (const std::exception& ex) {
    cout << "Exception: " << ex.what() << endl;
    return 1;
  }
}

