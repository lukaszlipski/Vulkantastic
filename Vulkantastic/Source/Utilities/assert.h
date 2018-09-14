#pragma once

#if _DEBUG
#define Assert(x) if(x) { } else { __debugbreak();  }
#else
#define Assert(x)
#endif