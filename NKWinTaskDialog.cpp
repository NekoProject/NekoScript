#include "stdafx.h"
#include "Helper.h"
#include "NKWinTaskDialog.h"

NKWinTaskDialog::NKWinTaskDialog(duk_context *ctx) : JSClass(ctx) {
	std::cout << "lkalallalala";
}

NKWinTaskDialog::~NKWinTaskDialog() {
	std::cout << "hahhahahaha";
}

duk_ret_t NKWinTaskDialog::Show() {
	HRESULT hr;
	TASKDIALOGCONFIG tdc = { sizeof(TASKDIALOGCONFIG) };
	std::wstring title, mainInstruction, content, expandedInformation;
	bool isNull;

	title = Utf8ToUtf16(getIvarString("title", &isNull));
	if (!isNull) tdc.pszWindowTitle = title.c_str();

	mainInstruction = Utf8ToUtf16(getIvarString("mainInstruction", &isNull));
	if (!isNull) tdc.pszMainInstruction = mainInstruction.c_str();

	content = Utf8ToUtf16(getIvarString("content", &isNull));
	if (!isNull) tdc.pszContent = content.c_str();

	expandedInformation = Utf8ToUtf16(getIvarString("expandedInformation", &isNull));
	if (!isNull) tdc.pszExpandedInformation = expandedInformation.c_str();

	tdc.dwCommonButtons = TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
	tdc.pszMainIcon = TD_INFORMATION_ICON;

	hr = TaskDialogIndirect(&tdc, NULL, NULL, NULL);
	return 0;
}

void NKWinTaskDialog::setupPrototype(duk_context *ctx) {
	registerMethod<&NKWinTaskDialog::Show>(ctx, "show");
}