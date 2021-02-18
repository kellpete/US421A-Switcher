#pragma once
#include <string>
#include <vector>
#include <sstream>

struct StatusUs421a {
	bool self_selected;
	bool self_locked;
	bool beeper_enabled;
	
	bool port_info_available; // Unfortunately, this is only available when connected to first port
	int selected_port;
	bool connected_ports[4];

	std::string ToString() {
	  std::stringstream s;
	  s << "SelfSelected:" << self_selected << " Lock:" << self_locked << " Beeper:" << beeper_enabled;
	  if (port_info_available) {
		s << " Selected: " << selected_port;
		s << " Host ports connected: ";
		for (int i = 0; i < 4; ++i) {
		  if (connected_ports[i]) {
			s << i << " ";
		  }
		}
	  }
	  return s.str();
	}
};

class PeripherialSwitchUs421a
{
public:
	PeripherialSwitchUs421a(std::wstring path);
	~PeripherialSwitchUs421a();

	StatusUs421a ReadStatus();
	void SendCommand(uint8_t* buf_size_2);
	
	// Selects own port, so that device can be used. Takes a few seconds to complete.
	// Also, the Aten US421A controller will re-enumerate, so don't use this instance anymore after Select()!
	void Select();

	// Locks for 10 seconds
	void Lock();

	// Unlocks the device within a few seconds (3 seconds according to status, 5 sec according to LEDs)
	void Unlock();

	// Keeps an existing lock alive for the next 10 seconds. (Aten's "Peripherial Switch Utility" sends this every 4s when locked)
	void LockKeepAlive();

	// Returns US421A device paths from which an PeripherialSwitchUs421a can be constructed
	static std::vector<std::wstring> GetDeviceList();

private:
	const std::wstring path_;
	void* file_handle_;
};

