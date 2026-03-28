#pragma once

#if defined(_WIN32) && defined(TSP_SOLVER_BUILDING_DLL)
#define TSP_SOLVER_API __declspec(dllexport)
#else
#define TSP_SOLVER_API
#endif
