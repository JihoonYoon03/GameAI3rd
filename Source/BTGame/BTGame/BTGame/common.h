#pragma once

#include <iostream>
#include <array>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <utility>

#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif