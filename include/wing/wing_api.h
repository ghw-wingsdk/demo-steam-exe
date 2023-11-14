#pragma once
#if defined(DLL_EXPORTS)
#define WING_API __declspec(dllexport)
#else
#define WING_API __declspec(dllimport)
#endif

#include <functional>
#include <string>
using namespace std;

// 回调参数：int code, string msg, string data
typedef void (*WingCallback)(int, string, string);

namespace WingProxy {
	WING_API void Init(string appId, string appKey, WingCallback callback);

	WING_API void Login(WingCallback callback);
	WING_API void Logout();

	WING_API bool IsPayAvailable();
	WING_API void CallPay(string productId, string extInfo, WingCallback callback);

	WING_API void SetServerId(string serverId);
	WING_API void SetGameUserId(string gameUserId);
	WING_API void SetNickName(string nickName);
	WING_API void SetLevel(int level);

	WING_API void PostEvent(const string& eventName, const string& params = "", double eventValue = numeric_limits<double>::quiet_NaN());
}