#pragma once
#include "stdafx.h"
#include "JSClass.h"
#include "Helper.h"

class NKWinTaskDialog
	: public JSClass<NKWinTaskDialog>
{
private:
	bool _preventClose;
	TASKDIALOGCONFIG _tdc;
	std::wstring _title;
	std::wstring _mainInstruction;
	std::wstring _content;
	std::wstring _expandedInformation;
	std::wstring _footer;
	std::wstring _verificationText;
	std::wstring _expandedControlText;
	std::wstring _collapsedControlText;
	std::vector<std::wstring> _buttons;
	TASKDIALOG_BUTTON* _buttonsRuntime = nullptr;
	std::vector<std::wstring> _radios;
	TASKDIALOG_BUTTON* _radiosRuntime = nullptr;

	HWND _hwnd = nullptr;

	void CreateConfig();
	void FreeRuntime();

public:
	NKWinTaskDialog(duk_context *ctx, void *ptr);
	~NKWinTaskDialog();

	duk_ret_t Show();
	duk_ret_t Navigate();
	duk_ret_t PreventClose();
	duk_ret_t EnableButton();
	duk_ret_t EnableRadioButton();

	static NKWinTaskDialog* tryConstruct(duk_context *ctx, void *ptr);
	static void setupPrototype(duk_context *ctx);

private:
	static HRESULT __stdcall staticCallback(HWND handle, UINT notification, WPARAM wParam, LPARAM lParam, LONG_PTR data);
	HRESULT callback(HWND handle, UINT notification, WPARAM wParam, LPARAM lParam);
};