#pragma once
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <thread>

#include <Windows.h>


#include "MemoryAllocator.h"

struct WindowsWaveDevice {
	typedef int16_t WORD;
	HWAVEOUT WO = NULL;
	WAVEFORMATEX Format = { 0, };
	Memory<int16_t> BuffA;
	Memory<int16_t> BuffB;
	WAVEHDR WHA = { 0, };
	WAVEHDR WHB = { 0, };
	double BuffSec = 1;
	int LastWaveWriteToDevice = 0;
	int LastWaveDataWrite= 0;
};

WindowsWaveDevice ConstructWindowsWaveDevice();
bool WaveWrite(WindowsWaveDevice& In);
bool WriteData(WindowsWaveDevice& In, Memory<WORD> M);
bool Play(WindowsWaveDevice& In);
bool Pause(WindowsWaveDevice& In);
DWORD GetPitch(WindowsWaveDevice& In);
bool SetPitch(WindowsWaveDevice& In, DWORD X);
bool Free(WindowsWaveDevice& In);
bool CheckDeviceWriteTiming(WindowsWaveDevice& In);
bool CheckDataWriteTiming(WindowsWaveDevice& In);