#pragma once
#include <array>
#include <string>
#include <vector>
#include <sstream>

struct StatusUs421a {
    StatusUs421a(std::array<uint8_t, 4> raw) : raw_status{ raw } {  }

	bool self_selected() const {
	  return raw_status[1] & 0x01;
	}
	bool self_locked() const {
	  return raw_status[1] & 0x02;
	}
	bool switch_requested() const {
	  return raw_status[1] & 0x04;
	}
	bool beeper_enabled() const {
	  return raw_status[3] & 0x01;
	}
	
    std::string ToString();

	const std::array<uint8_t, 4> raw_status;
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
    // If the device is locked by another host, this will only set the "switch requested" bit (see status). As soon as the other host unlocks, the device switches to this host
	void Select();

    // Clears the "switch requested" bit
	void CancelSwitchRequest();

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

