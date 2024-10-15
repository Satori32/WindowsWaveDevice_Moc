#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <thread>

#include <Windows.h>


#include "MemoryAllocator.h"

struct WindowsWaveDevice {
	typedef int16_t WORD;
	HWAVEOUT WO = NULL;
	WAVEFORMATEX Format = {0,};
	Memory<WORD> Buff;
	WAVEHDR WH = {0,};
	double BuffSec = 1;
};

WAVEFORMATEX CreateWaveFormatEX() {
	WAVEFORMATEX W;
	W.cbSize = sizeof(W);
	W.wFormatTag = WAVE_FORMAT_PCM;
	W.nChannels = 1;
	W.nSamplesPerSec = 44100;
	W.wBitsPerSample = 16;
	W.nAvgBytesPerSec = (W.wBitsPerSample / 8) * (W.nSamplesPerSec) * (W.nChannels);
	W.nBlockAlign = 0;

	return W;
}
bool Open(WindowsWaveDevice& In) {
	MMRESULT R =  waveOutOpen(&In.WO, WAVE_MAPPER, &In.Format, 0, (HINSTANCE)GetModuleHandle(NULL), WAVE_MAPPED);
	if (R != MMSYSERR_NOERROR) { return false; }
	In.Buff = ConstructMemroy<int16_t>(In.Format.nSamplesPerSec * In.BuffSec);
	double PI = 3.1415926535;
	double Rad = PI / 180.0;
	double T = 360 / In.Format.nSamplesPerSec;
	for (size_t i = 0; i < In.Format.nSamplesPerSec * In.BuffSec; i++) {
		if (Index(In.Buff, i) == NULL) { return false; }
		(*Index(In.Buff, i)) = sin(Rad * (T * i))*0xefff;
	}
	In.WH.lpData = (LPSTR)Index(In.Buff,0);
	In.WH.dwBufferLength = In.Format.nSamplesPerSec * In.BuffSec;
	In.WH.lpNext = NULL;
	In.WH.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	In.WH.dwLoops=1;

	MMRESULT X = waveOutPrepareHeader(In.WO, &In.WH, sizeof(In.WH));
	if (X != MMSYSERR_NOERROR) { waveOutClose(In.WO); In.WO = NULL; return false; }

	return true;
}
WindowsWaveDevice ConstructWindowsWaveDevice() {
	WindowsWaveDevice W;
	W.Format = CreateWaveFormatEX();
	if (Open(W) == false) {};
}
bool WaveWrite(WindowsWaveDevice& In){
	MMRESULT M = waveOutWrite(In.WO, &In.WH, sizeof(In.WH));
	if (M == MMSYSERR_NOERROR) { return false; }
	return true;
}
bool WriteData(WindowsWaveDevice& In, Memory<WORD> M) {
	for (size_t i = 0; i < Size(In.Buff); i++) {
		if (Index(In.Buff, i) == NULL) { return false; }
		if (Index(M, i) == NULL) { return false; }
		(*Index(In.Buff, i)) = (*Index(M,i));
	}
	return true;
}
bool Close(WindowsWaveDevice& In) {
	MMRESULT A = waveOutReset(In.WO);
	if (A != MMSYSERR_NOERROR) { return false; }
	MMRESULT B = waveOutUnprepareHeader(In.WO, &In.WH, sizeof(In.WH));
	if (B != MMSYSERR_NOERROR) { return false; }
	MMRESULT C = waveOutClose(In.WO);
	if (C != MMSYSERR_NOERROR) { return false; }

	WAVEFORMATEX W = {0,};
	WAVEHDR WW = {0,};

	In.WO = NULL;
	Free(In.Buff);
	In.Format = W;
	In.WH = WW;

	return true;
}
bool Play(WindowsWaveDevice& In) {
	MMRESULT M = waveOutRestart(In.WO);
	if (M != MMSYSERR_NOERROR) { return false; }
	return true;

}bool Pause(WindowsWaveDevice& In) {
	MMRESULT M = waveOutPause(In.WO);
	if (M != MMSYSERR_NOERROR) { return false; }
	return true;
}
DWORD GetPitch(WindowsWaveDevice& In) {
	DWORD X = 0;
	MMRESULT M = waveOutGetPitch(In.WO,&X);
	
	if (M != MMSYSERR_NOERROR) { return false; }
	return X;
}
bool SetPitch(WindowsWaveDevice& In,DWORD X) {
	MMRESULT M = waveOutSetPitch(In.WO,X);
	if (M != MMSYSERR_NOERROR) { return false; }
	return true;
}
MMTIME GetPosition(WindowsWaveDevice& In) {
	MMTIME MT = {0,};
	MMTIME X = {0,};
	MMRESULT M = waveOutGetPosition(In.WO,&MT,sizeof(MT));
	if (M != MMSYSERR_NOERROR) { return X; }
	return MT;
}
bool Free(WindowsWaveDevice& In) {
	if (Close(In) == false) { return false; }
	return true;
}

int main() {
	WindowsWaveDevice W;
	
	W = ConstructWindowsWaveDevice();
	if(W.WO == NULL) { return 0; }
	WaveWrite(W);
	Play(W);

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	Pause(W);
	Free(W);

	return 0;

}