#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <msiquery.h>
#include <strsafe.h>

// WiX Header Files:
#include <wcautil.h>
#include <strutil.h>
#include <iostream>
#include <fstream>
// TODO: reference additional headers your program requires here
#include "jsoncons/json.hpp"
