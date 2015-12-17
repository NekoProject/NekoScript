#pragma once
#include "stdafx.h"
#include "JSClass.h"
#include "Helper.h"
#include "NKWinTaskDialog.h"

class NKWinTaskDialog
	: public JSClass<NKWinTaskDialog>
{
private:
	bool _preventClose;

public:
	NKWinTaskDialog(duk_context *ctx, void *ptr);
	~NKWinTaskDialog();

	duk_ret_t Show();
	duk_ret_t PreventClose();

	static void setupPrototype(duk_context *ctx);

private:
	static HRESULT __stdcall staticCallback(HWND handle, UINT notification, WPARAM wParam, LPARAM lParam, LONG_PTR data);
	HRESULT callback(HWND handle, UINT notification, WPARAM wParam, LPARAM lParam);
};