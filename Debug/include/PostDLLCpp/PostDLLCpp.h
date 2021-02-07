#pragma once

#ifdef POSTDLLCPP_EXPORTS
#define POSTDLLCPP_API __declspec(dllexport)
#else
#define POSTDLLCPP_API __declspec(dllimport)
#endif

struct REQUEST_ERR_INFO
{
	bool failed;
	std::string cause;
};

extern "C" POSTDLLCPP_API void runPostRequestAndSaveToFile(REQUEST_ERR_INFO &errInfo);