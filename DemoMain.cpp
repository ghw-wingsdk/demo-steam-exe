#include "wing/wing_api.h"
#include <iostream>

#include "DemoUtils.h"
#include "json.hpp"
using json = nlohmann::json;

#define GLFW_EXPOSE_NATIVE_WIN32

#include <opengl/GLAD/glad.h>
#include <opengl/GLFW/glfw3.h>
#include <opengl/GLFW/glfw3native.h>
#include <string>
#include <atlstr.h>
#include <ctime>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")

void glfwErrorCallback(int error, const char* description) {
	std::cout << "glfwErrorCallback错误回调，code:" << error << ", desc:" << description << std::endl;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
bool glfwWindowCreation();

std::string GetTextInput(HWND hwnd) {
	int len = GetWindowTextLength(hwnd);
	char* buffer = new char[len + 1];
	GetWindowTextA(hwnd, buffer, len + 1);
	std::string str(buffer);
	delete[] buffer;
	return str;
}

LONG WINAPI GenerateDump(EXCEPTION_POINTERS* exceptionPointers)
{
	// 创建Dump文件
	HANDLE hFile = CreateFile(L"CrashDump.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cerr << "无法创建Dump文件" << std::endl;
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// 设置Dump文件选项
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ExceptionPointers = exceptionPointers;
	dumpInfo.ClientPointers = TRUE;

	// 生成Dump文件
	BOOL success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &dumpInfo, NULL, NULL);
	if (!success)
	{
		std::cerr << "生成Dump文件失败" << std::endl;
	}

	// 关闭Dump文件
	CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

// 窗口句柄
HWND g_hWnd;
HWND g_hEdit_ProductId;
HWND g_hEdit_ExtInfo;
HWND g_hEdit_EventName;
HWND g_hEdit_EventParams;
HWND g_hEdit_EventValue;
HWND g_hEdit_ServerId;
HWND g_hEdit_GameUserId;
HWND g_hEdit_Level;
HWND g_hEdit_NickName;

// 按钮控件ID
#define ID_BUTTON_INIT 1001
#define ID_BUTTON_LOGIN 1002
#define ID_BUTTON_LOGOUT 1003

#define ID_BUTTON_QUERY_PRODUCTS 1100
#define ID_BUTTON_IS_STEAM_PAY_AVAILABLE 1101

#define ID_BUTTON_OPENGL_WINDOW 1200
#define ID_EDIT_PRODUCT_ID 1201
#define ID_EDIT_EXTINFO 1202
#define ID_BUTTON_CALL_PAY 1203

#define ID_EDIT_EVENT_NAME 1301
#define ID_EDIT_EVENT_PARAMS 1302
#define ID_EDIT_EVENT_VALUE 1303
#define ID_BUTTON_POST_EVENT 1304

#define ID_EDIT_SET_SERVER_ID 1401
#define ID_EDIT_SET_GAME_USER_ID 1402
#define ID_EDIT_SET_LEVEL 1403
#define ID_EDIT_SET_NICKNAME 1404
#define ID_BUTTON_SET_SERVER_ID 1411
#define ID_BUTTON_SET_GAME_USER_ID 1412
#define ID_BUTTON_SET_LEVEL 1413
#define ID_BUTTON_SET_NICKNAME 1414

// 按钮回调函数
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	auto winProc = DefWindowProc(hWnd, message, wParam, lParam);

	if (message == WM_COMMAND) {
		if (LOWORD(wParam) == ID_BUTTON_INIT) {
			std::string appId = "36dd0dd17eae11eeadec02c85f0429f5";
			std::string appKey = "1fpY1oyhW85ngyHKTZ33Rrx6xQ5EmU8Z";
			WingProxy::Init(appId, appKey, [](int code, std::string msg, std::string data) {
				if (code == 200)
				{
					MessageBoxA(NULL, ("初始化成功..." + msg).c_str(), "提示", MB_OK);
				}
				else {
					MessageBoxA(NULL, ("初始化失败. 错误:" + msg).c_str(), "提示", MB_OK);
				}
				});
		}
		else if (LOWORD(wParam) == ID_BUTTON_LOGIN) {
			WingProxy::Login([](int code, std::string msg, std::string data) {
				if (code == 200)
				{
					json jsonData = json::parse(data);
					long long userId = jsonData["userId"];
					std::string token = jsonData["token"];

					std::string tips = "登录成功.";
					tips.append("\nUserId:").append(std::to_string(userId))
						.append("\nToken:").append(token);
					MessageBoxA(NULL, tips.c_str(), "提示", MB_OK);
				}
				else {
					MessageBoxA(NULL, ("登录失败. 错误:" + msg).c_str(), "提示", MB_OK);
				}
				});
		}
		else if (LOWORD(wParam) == ID_BUTTON_LOGOUT) {
			WingProxy::Logout();
			MessageBoxA(NULL, "退出完成", "提示", MB_OK);
		}
		else if (LOWORD(wParam) == ID_BUTTON_QUERY_PRODUCTS) {
			//WingProxy::QueryProducts([](int code, std::string msg, std::string data) {
			//	std::string tips = "查询结果. code:" + to_string(code) + ", msg:" + msg + ", \ndata:" + data;
			//	MessageBoxA(NULL, tips.c_str(), "提示", MB_OK);
			//	});
		}
		else if (LOWORD(wParam) == ID_BUTTON_IS_STEAM_PAY_AVAILABLE) {
			bool isPayAvailable = WingProxy::IsPayAvailable();
			std::string result = isPayAvailable ? "Yes" : "No";
			std::string tips = "支付是否可用：" + result;
			MessageBoxA(NULL, tips.c_str(), "提示", MB_OK);
		}
		else if (LOWORD(wParam) == ID_BUTTON_OPENGL_WINDOW) {
			std::thread threadCreateGlfwWindow(glfwWindowCreation);
			threadCreateGlfwWindow.detach();
		}
		else if (LOWORD(wParam) == ID_BUTTON_CALL_PAY) {
			bool isPayAvailable = WingProxy::IsPayAvailable();
			if (!isPayAvailable)
			{
				MessageBoxA(NULL, "支付不可用，请稍后重试", "提示", MB_OK);
				return winProc;
			}
			std::string strProductId = GetTextInput(g_hEdit_ProductId);
			std::string strExtInfo = GetTextInput(g_hEdit_ExtInfo);
			if (strProductId.empty())
			{
				MessageBoxA(NULL, "商品ID不能为空", "提示", MB_OK);
				return winProc;
			}
			WingProxy::CallPay(strProductId, strExtInfo, [](int code, std::string msg, std::string data) {

				if (code != 200)
				{
					MessageBoxA(NULL, ("支付失败. 错误:" + msg).c_str(), "提示", MB_OK);
				}
				else {
					json jsonData = json::parse(data);
					std::string orderId = jsonData["orderId"];
					std::string productId = jsonData["productId"];
					std::string currency = jsonData["currency"];
					long long amount = jsonData["amount"];
					int quantity = jsonData["quantity"];
					std::string extInfo = jsonData["extInfo"];
					int status = jsonData["status"];

					std::string tips = "";
					tips.append("\n订单ID：").append(orderId)
						.append("\n商品ID：").append(productId)
						.append("\n基准币种：").append(currency)
						.append("\n基准货币价格：").append(std::to_string(amount))
						.append("\n购买数量：").append(std::to_string(quantity))
						.append("\n透传信息：").append(extInfo)
						.append("\n订单状态：").append(std::to_string(status))
						.append("\n\n")
						.append(status == 1 ? "支付成功" : "支付失败");

					MessageBoxA(NULL, tips.c_str(), "提示", MB_OK);
				}
				});
		}
		else if (LOWORD(wParam) == ID_BUTTON_POST_EVENT) {
			std::string strEventName = GetTextInput(g_hEdit_EventName);
			std::string strEventParams = GetTextInput(g_hEdit_EventParams);
			std::string strEventValue = GetTextInput(g_hEdit_EventValue);
			if (strEventName.empty())
			{
				MessageBoxA(NULL, "事件名不能为空", "提示", MB_OK);
				return winProc;
			}

			WingProxy::PostEvent(strEventName, strEventParams, strEventValue.empty() ? numeric_limits<double>::quiet_NaN() : stod(strEventValue));
			std::string tips = "发送事件：" + strEventName + ", 参数：" + strEventParams + ", 价值：" + strEventValue;
			MessageBoxA(NULL, tips.c_str(), "提示", MB_OK);

		}
		else if (LOWORD(wParam) == ID_BUTTON_SET_SERVER_ID) {
			std::string strInput = GetTextInput(g_hEdit_ServerId);
			WingProxy::SetServerId(strInput);
		}
		else if (LOWORD(wParam) == ID_BUTTON_SET_GAME_USER_ID) {
			std::string strInput = GetTextInput(g_hEdit_GameUserId);
			WingProxy::SetGameUserId(strInput);
		}
		else if (LOWORD(wParam) == ID_BUTTON_SET_LEVEL) {
			std::string strInput = GetTextInput(g_hEdit_Level);
			WingProxy::SetLevel(strInput.empty() ? 0 : stoi(strInput));
		}
		else if (LOWORD(wParam) == ID_BUTTON_SET_NICKNAME) {
			std::string strInput = GetTextInput(g_hEdit_NickName);
			WingProxy::SetNickName(strInput);
		}
	}

	return winProc;
}

// 窗口回调函数
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE: {
		DWORD dwStyleButton = WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE;
		DWORD dwStyleEdit = WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL;
		int spaceCol = 10;
		int spaceRow = 20;
		int width = 140;
		int height = 50;
		int topRow = 10;
		int leftCol = 10;
		CreateWindow(TEXT("button"), TEXT("初始化"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_INIT, NULL, NULL);
		leftCol += width + spaceCol;
		CreateWindow(TEXT("button"), TEXT("登录"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_LOGIN, NULL, NULL);
		leftCol += width + spaceCol;
		CreateWindow(TEXT("button"), TEXT("退出"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_LOGOUT, NULL, NULL);

		leftCol = 10;
		topRow += height + spaceRow;
		int widthGlWin = (width * 4) + (spaceCol * 3);
		//CreateWindow(TEXT("button"), TEXT("查询商品"), dwStyleButton, 10, 50, width, height, hWnd, (HMENU)ID_BUTTON_QUERY_PRODUCTS, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("OpenGL窗口（先开启后支付）"), dwStyleButton, leftCol, topRow, widthGlWin, height, hWnd, (HMENU)ID_BUTTON_OPENGL_WINDOW, NULL, NULL);
		topRow += height + spaceRow;
		topRow -= (spaceRow - 2);
		CreateWindow(TEXT("button"), TEXT("检查支付环境\n是否可用"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_IS_STEAM_PAY_AVAILABLE, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_ProductId = CreateWindow(TEXT("EDIT"), TEXT("game002"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_PRODUCT_ID, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_ExtInfo = CreateWindow(TEXT("EDIT"), TEXT("MyInfo|10086"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_EXTINFO, NULL, NULL);
		leftCol += width + spaceCol;
		CreateWindow(TEXT("button"), TEXT("调起支付"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_CALL_PAY, NULL, NULL);

		leftCol = 10;
		topRow += height + spaceRow;
		g_hEdit_EventName = CreateWindow(TEXT("EDIT"), TEXT("ghw_user_create"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_EVENT_NAME, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_EventParams = CreateWindow(TEXT("EDIT"), TEXT("{\"nickname\":\"Gooooood\",\"registerTime\":1698911752123}"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_EVENT_PARAMS, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_EventValue = CreateWindow(TEXT("EDIT"), TEXT("9.99"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_EVENT_VALUE, NULL, NULL);

		leftCol = 10;
		topRow += height + spaceRow;
		topRow -= (spaceRow - 2);
		widthGlWin = (width * 3) + (spaceCol * 2);
		CreateWindow(TEXT("button"), TEXT("事件发送"), dwStyleButton, leftCol, topRow, widthGlWin, height, hWnd, (HMENU)ID_BUTTON_POST_EVENT, NULL, NULL);

		leftCol = 10;
		topRow += height + spaceRow;
		g_hEdit_ServerId = CreateWindow(TEXT("EDIT"), TEXT("18"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_SET_SERVER_ID, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_GameUserId = CreateWindow(TEXT("EDIT"), TEXT("GGG58987"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_SET_GAME_USER_ID, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_Level = CreateWindow(TEXT("EDIT"), TEXT("118"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_SET_LEVEL, NULL, NULL);
		leftCol += width + spaceCol;
		g_hEdit_NickName = CreateWindow(TEXT("EDIT"), TEXT("Mr.Win"), dwStyleEdit, leftCol, topRow, width, height, hWnd, (HMENU)ID_EDIT_SET_NICKNAME, NULL, NULL);

		leftCol = 10;
		topRow -= (spaceRow - 2);
		topRow += height + spaceRow;
		CreateWindow(TEXT("button"), TEXT("设置ServerId"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_SET_SERVER_ID, NULL, NULL);
		leftCol += width + spaceCol;
		CreateWindow(TEXT("button"), TEXT("设置GameUserId"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_SET_GAME_USER_ID, NULL, NULL);
		leftCol += width + spaceCol;
		CreateWindow(TEXT("button"), TEXT("设置Level"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_SET_LEVEL, NULL, NULL);
		leftCol += width + spaceCol;
		CreateWindow(TEXT("button"), TEXT("设置NickName"), dwStyleButton, leftCol, topRow, width, height, hWnd, (HMENU)ID_BUTTON_SET_NICKNAME, NULL, NULL);

		break; }
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 设置异常处理函数
	SetUnhandledExceptionFilter(GenerateDump);

	// 注册窗口类
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = TEXT("MyWindowClass");
	RegisterClass(&wc);

	// 创建窗口	
	LPCWSTR title = DemoUtils::ToLPCWSTR("WING STEAM EXE SDK v1.0.0  - Build at " + DemoUtils::GetCompileTime());
	g_hWnd = CreateWindow(TEXT("MyWindowClass"), title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

	// 创建按钮1和按钮2的消息处理函数
	WNDPROC buttonProc = (WNDPROC)ButtonProc;
	SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)buttonProc);

	// 显示窗口
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	// 消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 确保视口与新的窗口尺寸相匹配；请注意，宽度和高度将明显大于视网膜显示器上指定的值。
	glViewport(0, 0, width, height);
}

bool glfwWindowCreation() {
	glfwSetErrorCallback(glfwErrorCallback);

	// glfw: initialize and configure
	if (!glfwInit()) {
		return false;
	}

	//指定最低兼容版本，如果在机器上不支持所需的最小版本，则上下文（和窗口）的创建失败。如openGL3.2，MAJOR是主要版本3，MINOR是次要版本2。
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// 指定要为其创建上下文的 OpenGL 配置文件。可能的值是 或 之一GLFW_OPENGL_CORE_PROFILE，GLFW_OPENGL_COMPAT_PROFILE或者GLFW_OPENGL_ANY_PROFILE不请求特定配置文件。
	// 如果请求低于 3.2 的 OpenGL 版本，则GLFW_OPENGL_ANY_PROFILE必须使用。如果请求 OpenGL ES，则忽略此提示。默认值 GLFW_OPENGL_ANY_PROFILE。
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// 隐藏边框，若不设置则CEF浏览器窗口会失去焦点无法操作
	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(1200, 700, "WingSdk Demo", NULL, NULL);
	if (window == NULL) {
		std::cout << "无法创建glfw支付窗口" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);
	//键盘回调
	//glfwSetKeyCallback(window, glfwKeyCallback);
	//每当窗口大小发生变化（通过操作系统或用户调整大小）时，此回调函数就会执行
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//监听鼠标位置
	//glfwSetCursorPosCallback(window, glfwCursorPositionCallback);
	//glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
	// 设置窗口位置
	glfwSetWindowPos(window, 0, 30);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "GLAD initialize 失败" << std::endl;
		return false;
	}

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// input
		//if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			//glfwSetWindowShouldClose(window, true);
			//LOG_DEBUG("按下ESC，退出GLFW渲染循环");
		//}

		// 交换缓冲区
		glfwSwapBuffers(window);
		// 处理事件
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwDestroyWindow(window);
	glfwTerminate();

	glfwSetErrorCallback(NULL);

	return true;
}
