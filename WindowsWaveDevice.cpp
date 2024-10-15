#include "WindowsWaveDevice.h"

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
	MMRESULT R = waveOutOpen(&In.WO, WAVE_MAPPER, &In.Format, 0, (HINSTANCE)GetModuleHandle(NULL), WAVE_MAPPED);
	if (R != MMSYSERR_NOERROR) { return false; }
	In.BuffA = ConstructMemroy<int16_t>(In.Format.nSamplesPerSec * In.BuffSec);
	In.BuffB = ConstructMemroy<int16_t>(In.Format.nSamplesPerSec * In.BuffSec);
	double PI = 3.1415926535;
	double Rad = PI / 180.0;
	double T = 360 / In.Format.nSamplesPerSec;
	for (size_t i = 0; i < In.Format.nSamplesPerSec * In.BuffSec; i++) {
		if (Index(In.BuffA, i) == NULL) { return false; }
		if (Index(In.BuffB, i) == NULL) { return false; }
		(*Index(In.BuffA, i)) = sin(Rad * (T * i)) * 0xefff;
		(*Index(In.BuffB, i)) = sin(Rad * (T * i)) * 0xefff;
	}
	In.WHA.lpData = (LPSTR)Index(In.BuffA, 0);
	In.WHA.dwBufferLength = In.Format.nSamplesPerSec * In.BuffSec;
	In.WHA.lpNext = GETPointer(In.BuffB);
	In.WHA.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	In.WHA.dwLoops = 1;

	In.WHA.lpData = (LPSTR)Index(In.BuffB, 0);
	In.WHA.dwBufferLength = In.Format.nSamplesPerSec * In.BuffSec;
	In.WHA.lpNext = GETPointer(In.BuffA);
	In.WHA.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	In.WHA.dwLoops = 1;
	
	MMRESULT X = waveOutPrepareHeader(In.WO, &In.WHA, sizeof(In.WHA));
	if (X != MMSYSERR_NOERROR) { waveOutClose(In.WO); In.WO = NULL; return false; }
	X = waveOutPrepareHeader(In.WO, &In.WHB, sizeof(In.WHB));
	if (X != MMSYSERR_NOERROR) { waveOutClose(In.WO); In.WO = NULL; return false; }

	return true;
}
WindowsWaveDevice ConstructWindowsWaveDevice() {
	WindowsWaveDevice W;
	W.Format = CreateWaveFormatEX();
	if (Open(W) == false) {};
}
bool CheckDeviceWriteTiming(WindowsWaveDevice& In) {
	if (In.LastWaveDataWrite == In.LastWaveWriteToDevice) {
		return false;
	}
	return true;
}
bool CheckDataWriteTiming(WindowsWaveDevice& In) {
	if (In.LastWaveDataWrite != In.LastWaveWriteToDevice) {
		return false;
	}
	return true;
}
bool WaveWrite(WindowsWaveDevice& In) {
	if (In.LastWaveWriteToDevice == 0) {
		WaveWriteA(In);
		In.LastWaveWriteToDevice = 1;
	}
	else {
		WaveWriteB(In);
		In.LastWaveWriteToDevice = 0;
	}
}
bool WaveWrite(WindowsWaveDevice& In) {
	if (In.LastWaveWriteToDevice == 0) {
		if (WaveWriteA(In) == false) { return false; }
		In.LastWaveWriteToDevice = 1;
	}
	else {
		if (WaveWriteB(In) == false) { return false; }
		In.LastWaveWriteToDevice = 0;
	}
	return true;
}
bool WaveWriteA(WindowsWaveDevice& In) {
	MMRESULT M = waveOutWrite(In.WO, &In.WHA, sizeof(In.WHA));
	if (M == MMSYSERR_NOERROR) { return false; }
	return true;
}
bool WaveWriteB(WindowsWaveDevice& In) {
	MMRESULT M = waveOutWrite(In.WO, &In.WHB, sizeof(In.WHB));
	if (M == MMSYSERR_NOERROR) { return false; }
	return true;
}
bool WaveData(WindowsWaveDevice& In,Memory<int16_t>& M) {
	if (In.LastWaveWriteToDevice == 0) {
		if (WriteDataA(In,M) == false) { return false; }
		In.LastWaveDataWrite = 1;
	}
	else {
		if (WriteDataB(In,M) == false) { return false; }
		In.LastWaveDataWrite = 0;
	}
	return true;
}
bool WriteDataA(WindowsWaveDevice& In, Memory<int16_t>& M) {
	for (size_t i = 0; i < Size(In.BuffA); i++) {
		if (Index(In.BuffA, i) == NULL) { return false; }
		if (Index(M, i) == NULL) { return false; }
		(*Index(In.BuffA, i)) = (*Index(M, i));
	}
	return true;
}
bool WriteDataB(WindowsWaveDevice& In, Memory<int16_t>& M) {
	for (size_t i = 0; i < Size(In.BuffB); i++) {
		if (Index(In.BuffB, i) == NULL) { return false; }
		if (Index(M, i) == NULL) { return false; }
		(*Index(In.BuffA, i)) = (*Index(M, i));
	}
	return true;
}
bool Close(WindowsWaveDevice& In) {
	MMRESULT A = waveOutReset(In.WO);
	if (A != MMSYSERR_NOERROR) { return false; }
	MMRESULT B = waveOutUnprepareHeader(In.WO, &In.WHA, sizeof(In.WHA));
	if (B != MMSYSERR_NOERROR) { return false; }
	B = waveOutUnprepareHeader(In.WO, &In.WHB, sizeof(In.WHB));
	if (B != MMSYSERR_NOERROR) { return false; }
	MMRESULT C = waveOutClose(In.WO);
	if (C != MMSYSERR_NOERROR) { return false; }

	WAVEFORMATEX W = { 0, };
	WAVEHDR WW = { 0, };

	In.WO = NULL;
	Free(In.BuffA);
	Free(In.BuffB);
	In.Format = W;
	In.WHA = WW;
	In.WHB = WW;

	return true;
}
bool Play(WindowsWaveDevice& In) {
	MMRESULT M = waveOutRestart(In.WO);
	if (M != MMSYSERR_NOERROR) { return false; }
	return true;

}
bool Pause(WindowsWaveDevice& In) {
	MMRESULT M = waveOutPause(In.WO);
	if (M != MMSYSERR_NOERROR) { return false; }
	return true;
}
DWORD GetPitch(WindowsWaveDevice& In) {
	DWORD X = 0;
	MMRESULT M = waveOutGetPitch(In.WO, &X);

	if (M != MMSYSERR_NOERROR) { return false; }
	return X;
}
bool SetPitch(WindowsWaveDevice& In, DWORD X) {
	MMRESULT M = waveOutSetPitch(In.WO, X);
	if (M != MMSYSERR_NOERROR) { return false; }
	return true;
}
MMTIME GetPosition(WindowsWaveDevice& In) {
	MMTIME MT = { 0, };
	MMTIME X = { 0, };
	MMRESULT M = waveOutGetPosition(In.WO, &MT, sizeof(MT));
	if (M != MMSYSERR_NOERROR) { return X; }
	return MT;
}
bool Free(WindowsWaveDevice& In) {
	if (Close(In) == false) { return false; }
	return true;
}