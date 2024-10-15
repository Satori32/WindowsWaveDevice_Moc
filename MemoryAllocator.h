#pragma once
#include <iostream>
#include <stdlib.h>
#include <memory.h>
//#include <crtdbg.h>//msvc only

template <class T>
struct Memory {
	T* M = NULL;
	size_t L = 0;
};

template <class T> Memory<T> ConstructMemroy(size_t N);
template <class T> Memory<T> ConstructMemroyByArray(T* Te, size_t L);
template <class T> bool Free(Memory<T>& In);
template <class T> T* Index(Memory<T>& In, size_t Pos);
template <class T> bool ReAllocateMemory(Memory<T>& In, size_t L);
template <class T> size_t Size(Memory<T>& In);
template <class T> Memory<T> Duplicate(Memory<T>& In);