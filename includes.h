#pragma once

#pragma warning( disable : 4307 ) // '*': integral constant overflow
#pragma warning( disable : 4244 ) // possible loss of data
#pragma warning( disable : 4800 ) // forcing value to bool 'true' or 'false'
#pragma warning( disable : 4838 ) // conversion from '::size_t' to 'int' requires a narrowing conversion

// You can define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
//_except_handler4_common 
using ulong_t = unsigned long;

// windows / stl includes.
#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <xmmintrin.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <functional>
#include <map>
#include <shlobj.h>
#include <filesystem>
#include <streambuf>
#include <d3d9.h>
#include <d3dx9.h>
#include <shellapi.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")



// our custom wrapper.
#include "unique_vector.h"
#include "tinyformat.h"

// other includes.
#include "hash.h"
#include "xorstr.h"
#include "pe.h"
#include "winapir.h"
#include "address.h"
#include "util.h"
#include "modules.h"
#include "pattern.h"
#include "vmt.h"
#include "stack.h"
#include "nt.h"
#include "x86.h"
#include "syscall.h"

// hack includes.
#include "interfaces.h"
#include "sdk.h"
#include "csgo.h"
#include "penetration.h"
#include "client.h"
#include "netvars.h"
#include "entoffsets.h"
#include "entity.h"
#include "gamerules.h"
#include "hooks.h"
#include "render.h"
#include "pred.h"
#include "lagrecord.h"
#include "bonearray.h"
#include "visuals.h"
#include "movement.h"
#include "bonesetup.h"
#include "hvh.h"
#include "lagcomp.h"
#include "aimbot.h"
#include "netdata.h"
#include "chams.h"
#include "notify.h"
#include "resolver.h"
#include "grenades.h"
#include "skins.h"
#include "events.h"
#include "shots.h"

// gui includes.
#include "json.h"
#include "base64.h"
#include "element.h"
#include "checkbox.h"
#include "dropdown.h"
#include "multidropdown.h"
#include "slider.h"
#include "colorpicker.h"
#include "edit.h"
#include "keybind.h"
#include "button.h"
#include "tab.h"
#include "form.h"
#include "gui.h"
#include "callbacks.h"
#include "menu.h"
#include "config.h"
#include "plist.h"
#include "imgui.h"
#include "ServerSound.h"
#include "gw.h"

#include "detours.h"
#include "lazy_importer.hpp"
#include "instr.h"
#define prod 1
//#ifdef DOVMP
//#include <VMProtectSDK.h>
//#define VMPBSTART(tag)  VMProtectBeginUltra(tag);
//#define VMPLSTART(tag)  VMProtectBeginMutation(tag);
//#define VMPEND()     VMProtectEnd();
//#else
//#define VMPBSTART(tag)
//#define VMPLSTART(tag)
//#define VMPEND()
//#define VMProtectIsDebuggerPresent(x) false
//#define VMProtectGetCurrentHWID(x, z) 0
//#define VMProtectBeginUltra(tag)
//#define VMProtectBeginMutation(tag)
//#define VMProtectEnd()
//#endif

#ifdef DOVMP
#include <VMProtectSDK.h>
#define VMPBSTART(tag)  VMProtectBeginUltra(tag);
#define VMPLSTART(tag)  VMProtectBeginMutation(tag);
#define VMPEND()     VMProtectEnd();
#else
#define VMPBSTART(tag)
#define VMPLSTART(tag)
#define VMPEND()
#define VMProtectIsDebuggerPresent(x) false
#define VMProtectGetCurrentHWID(x, z) 0
#define VMProtectBeginUltra(x)
#define VMProtectEnd(x)
#define VMProtectBeginMutation(x)
#endif




// и где