#pragma once
#include <iostream>
#include <string>
#include <Windows.h>

class DemoUtils
{
public:
	static std::string GetCompileTime() {
        std::string compileDate = __DATE__;
        std::string compileTime = __TIME__;

        // ��ȡ�·ݡ����ں����
        std::string month = compileDate.substr(0, 3);
        std::string day = compileDate.substr(4, 2);
        std::string year = compileDate.substr(7, 4);

        // ��ȡСʱ�ͷ���
        std::string hour = compileTime.substr(0, 2);
        std::string minute = compileTime.substr(3, 2);

        // �����·ݵ���дת��Ϊ��Ӧ������
        std::string monthNum;
        if (month == "Jan") monthNum = "01";
        else if (month == "Feb") monthNum = "02";
        else if (month == "Mar") monthNum = "03";
        else if (month == "Apr") monthNum = "04";
        else if (month == "May") monthNum = "05";
        else if (month == "Jun") monthNum = "06";
        else if (month == "Jul") monthNum = "07";
        else if (month == "Aug") monthNum = "08";
        else if (month == "Sep") monthNum = "09";
        else if (month == "Oct") monthNum = "10";
        else if (month == "Nov") monthNum = "11";
        else if (month == "Dec") monthNum = "12";

        // ƴ�����ں�ʱ��
        std::string compileTimeFormatted = year + "-" + monthNum + "-" + day + " " + hour + ":" + minute;
        return compileTimeFormatted;
    }
	static LPCWSTR ToLPCWSTR(std::string str) {

		// ���ַ���ת��Ϊ���ַ��汾
		int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		wchar_t* wstr = new wchar_t[size];
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, size);

		// �����ַ��ַ������ݸ�LPCWSTR���͵ı���
		LPCWSTR lpcwstr = wstr;
		return lpcwstr;
	}
private:

};