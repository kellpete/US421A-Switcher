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


int main()
{
	try {
	  return RunTest();
      //JustObserve();
      //JustObserveAvoidEnumeration();
     
	}
	catch (const std::exception& ex) {
		cout << "Exception: " << ex.what() << endl;
		return 1;
	}
}

