#include "stdafx.h"
#include "Helper.h"
#include "JSEventEmitter.h"
#include "NKWinTaskDialog.h"

NKWinTaskDialog::NKWinTaskDialog(duk_context *ctx, void *ptr) : JSClass(ctx, ptr) {
	_preventClose = false;
}

NKWinTaskDialog::~NKWinTaskDialog() {
}

NKWinTaskDialog* NKWinTaskDialog::tryConstruct(duk_context *ctx, void *ptr) {
	return new NKWinTaskDialog(ctx, ptr);
}

static inline PCWSTR ParseIcon(const std::string &icon) {
	if (icon == "") return 0;
	if (icon == "null") return 0;
	if (icon == "undefined") return 0;
	if (icon == "info") return TD_INFORMATION_ICON;
	if (icon == "information") return TD_INFORMATION_ICON;
	if (icon == "warn") return TD_WARNING_ICON;
	if (icon == "warning") return TD_WARNING_ICON;
	if (icon == "err") return TD_ERROR_ICON;
	if (icon == "error") return TD_ERROR_ICON;
	if (icon == "shield") return TD_SHIELD_ICON;
	return 0;
}

static inline TASKDIALOG_COMMON_BUTTON_FLAGS ParseCommonButton(const std::string &button) {
	if (button == "ok") return TDCBF_OK_BUTTON;
	if (button == "yes") return TDCBF_YES_BUTTON;
	if (button == "no") return TDCBF_NO_BUTTON;
	if (button == "cancel") return TDCBF_CANCEL_BUTTON;
	if (button == "retry") return TDCBF_RETRY_BUTTON;
	if (button == "close") return TDCBF_CLOSE_BUTTON;
	return 0;
}

static inline std::string ParseCommonButtonID(int button) {
	if (button == IDOK) return "ok";
	if (button == IDYES) return "yes";
	if (button == IDNO) return "no";
	if (button == IDCANCEL) return "cancel";
	if (button == IDRETRY) return "retry";
	if (button == IDCLOSE) return "close";
	return "";
}

duk_ret_t NKWinTaskDialog::Show() {
	HRESULT hr;
	TASKDIALOGCONFIG tdc = { sizeof(TASKDIALOGCONFIG) };
	std::wstring title, mainInstruction, content, expandedInformation, footer, verificationText, expandedControlText, collapsedControlText;
	std::string icon;
	bool isNull;

	tdc.pfCallback = &staticCallback;
	tdc.lpCallbackData = reinterpret_cast<LONG_PTR>(this);

	if (duk_is_object(ctx, 0)) 
		duk_dup(ctx, 0);
	else 
		duk_push_object(ctx);

	title = Utf8ToUtf16(JSGetPropString(ctx, "title", &isNull));
	if (!isNull) tdc.pszWindowTitle = title.c_str();

	mainInstruction = Utf8ToUtf16(JSGetPropString(ctx, "mainInstruction", &isNull));
	if (!isNull) tdc.pszMainInstruction = mainInstruction.c_str();

	content = Utf8ToUtf16(JSGetPropString(ctx, "content", &isNull));
	if (!isNull) tdc.pszContent = content.c_str();

	expandedInformation = Utf8ToUtf16(JSGetPropString(ctx, "expandedInformation", &isNull));
	if (!isNull) tdc.pszExpandedInformation = expandedInformation.c_str();

	expandedControlText = Utf8ToUtf16(JSGetPropString(ctx, "expandedControlText", &isNull));
	if (!isNull) tdc.pszExpandedControlText = expandedControlText.c_str();

	collapsedControlText = Utf8ToUtf16(JSGetPropString(ctx, "collapsedControlText", &isNull));
	if (!isNull) tdc.pszCollapsedControlText = collapsedControlText.c_str();

	footer = Utf8ToUtf16(JSGetPropString(ctx, "footer", &isNull));
	if (!isNull) tdc.pszFooter = footer.c_str();

	verificationText = Utf8ToUtf16(JSGetPropString(ctx, "verificationText", &isNull));
	if (!isNull) tdc.pszVerificationText = verificationText.c_str();

	tdc.pszMainIcon = ParseIcon(JSGetPropString(ctx, "icon"));
	tdc.pszFooterIcon = ParseIcon(JSGetPropString(ctx, "footerIcon"));

	if (JSGetPropBoolean(ctx, "allowDialogCancellation"))  tdc.dwFlags |= TDF_ALLOW_DIALOG_CANCELLATION;
	if (JSGetPropBoolean(ctx, "useCommandLinks"))          tdc.dwFlags |= TDF_USE_COMMAND_LINKS;
	if (JSGetPropBoolean(ctx, "useCommandLinksNoIcon"))    tdc.dwFlags |= TDF_USE_COMMAND_LINKS_NO_ICON;
	if (JSGetPropBoolean(ctx, "enableHyperlinks"))         tdc.dwFlags |= TDF_ENABLE_HYPERLINKS;
	if (JSGetPropBoolean(ctx, "expandFooterArea"))         tdc.dwFlags |= TDF_EXPAND_FOOTER_AREA;
	if (JSGetPropBoolean(ctx, "expandedByDefault"))        tdc.dwFlags |= TDF_EXPANDED_BY_DEFAULT;
	if (JSGetPropBoolean(ctx, "verificationFlagChecked"))  tdc.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
	if (JSGetPropBoolean(ctx, "showProgressBar"))          tdc.dwFlags |= TDF_SHOW_PROGRESS_BAR;
	if (JSGetPropBoolean(ctx, "showMarqueeProgressBar"))   tdc.dwFlags |= TDF_SHOW_MARQUEE_PROGRESS_BAR;
	if (JSGetPropBoolean(ctx, "positionRelativeToWindow")) tdc.dwFlags |= TDF_POSITION_RELATIVE_TO_WINDOW;
	if (JSGetPropBoolean(ctx, "rtlLayout"))                tdc.dwFlags |= TDF_RTL_LAYOUT;
	if (JSGetPropBoolean(ctx, "noDefaultRadioButton"))     tdc.dwFlags |= TDF_NO_DEFAULT_RADIO_BUTTON;
	if (JSGetPropBoolean(ctx, "canBeMinimized"))           tdc.dwFlags |= TDF_CAN_BE_MINIMIZED;
	if (JSGetPropBoolean(ctx, "sizeToContent"))            tdc.dwFlags |= TDF_SIZE_TO_CONTENT;

	TASKDIALOG_BUTTON* buttons = nullptr;
	std::wstring* buttonStrings = nullptr;
	duk_get_prop_string(ctx, -1, "buttons");
	if (duk_is_array(ctx, -1)) {
		int length = duk_get_length(ctx, -1);
		buttons = new TASKDIALOG_BUTTON[length];
		buttonStrings = new std::wstring[length];

		for (int i = 0; i < length; i++) {
			buttonStrings[i] = Utf8ToUtf16(JSGetPropString(ctx, i));
			buttons[i].nButtonID = 1000 + i;
			buttons[i].pszButtonText = buttonStrings[i].c_str();
		}

		tdc.pButtons = buttons;
		tdc.cButtons = length;
	}
	duk_pop(ctx);

	TASKDIALOG_BUTTON* radios = nullptr;
	std::wstring* radioStrings = nullptr;
	duk_get_prop_string(ctx, -1, "radioButtons");
	if (duk_is_array(ctx, -1)) {
		int length = duk_get_length(ctx, -1);
		radios = new TASKDIALOG_BUTTON[length];
		radioStrings = new std::wstring[length];

		for (int i = 0; i < length; i++) {
			radioStrings[i] = Utf8ToUtf16(JSGetPropString(ctx, i));
			radios[i].nButtonID = 2000 + i;
			radios[i].pszButtonText = radioStrings[i].c_str();
		}

		tdc.pRadioButtons = radios;
		tdc.cRadioButtons = length;
	}
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "commonButtons");
	if (duk_is_array(ctx, -1)) {
		int length = duk_get_length(ctx, -1);

		for (int i = 0; i < length; i++) {
			tdc.dwCommonButtons |= ParseCommonButton(JSGetPropString(ctx, i));
		}
	}
	duk_pop(ctx);

	duk_pop(ctx);

	hr = TaskDialogIndirect(&tdc, NULL, NULL, NULL);
	if (buttons) {
		delete[] buttons;
		delete[] buttonStrings;
	}
	if (radios) {
		delete[] radios;
		delete[] radioStrings;
	}

	return 0;
}

duk_ret_t NKWinTaskDialog::PreventClose()
{
	_preventClose = true;
	return 0;
}

void NKWinTaskDialog::setupPrototype(duk_context *ctx) {
	JSEventEmitter::putOnStack(ctx);
	duk_new(ctx, 0);
	
	registerMethod<&NKWinTaskDialog::Show>(ctx, "show", 1);
	registerMethod<&NKWinTaskDialog::PreventClose>(ctx, "preventClose", 0);

	duk_put_prop_string(ctx, -2, "prototype");
}

HRESULT __stdcall NKWinTaskDialog::staticCallback(HWND handle, UINT notification, WPARAM wParam, LPARAM lParam, LONG_PTR data)
{
	NKWinTaskDialog* self = reinterpret_cast<NKWinTaskDialog*>(data);
	_ASSERTE(self);

	return self->callback(handle, notification, wParam, lParam);
}

HRESULT NKWinTaskDialog::callback(HWND handle, UINT notification, WPARAM wParam, LPARAM lParam)
{
	HRESULT result = S_OK;
	JSEventEmitter::putEmitOnStack(ctx);
	duk_push_this(ctx);
	
	switch (notification)
	{
	case TDN_CREATED:
		duk_push_string(ctx, "created");
		duk_call_method(ctx, 1);
		duk_pop(ctx);
		break;

	case TDN_NAVIGATED:
		duk_push_string(ctx, "navigated");
		duk_call_method(ctx, 1);
		duk_pop(ctx);
		break;

	case TDN_BUTTON_CLICKED:
		{
			_preventClose = false;
			duk_push_string(ctx, "buttonClicked");
			std::string commonButtons = ParseCommonButtonID(wParam);
			if (commonButtons.empty()) {
				duk_push_int(ctx, wParam - 1000);
			}
			else {
				duk_push_string(ctx, commonButtons.c_str());
			}
			duk_call_method(ctx, 2);
			duk_pop(ctx);

			if (_preventClose) {
				_preventClose = false;
				result = S_FALSE;
			}
		}
		break;

	case TDN_HYPERLINK_CLICKED:
		{
			std::wstring str = std::wstring(reinterpret_cast<LPCWSTR>(lParam));
			duk_push_string(ctx, "hyperlinkClicked");
			duk_push_string(ctx, Utf16ToUtf8(str).c_str());
			duk_call_method(ctx, 2);
			duk_pop(ctx);
		}
		break;

	case TDN_TIMER:
		duk_push_string(ctx, "timer");
		duk_push_uint(ctx, wParam);
		duk_call_method(ctx, 2);
		duk_pop(ctx);
		break;

	case TDN_DESTROYED:
		duk_push_string(ctx, "destroyed");
		duk_call_method(ctx, 1);
		duk_pop(ctx);
		break;

	case TDN_RADIO_BUTTON_CLICKED:
		duk_push_string(ctx, "radioButtonClicked");
		duk_push_int(ctx, wParam - 2000);
		duk_call_method(ctx, 2);
		duk_pop(ctx);
		break;

	case TDN_DIALOG_CONSTRUCTED:
		duk_push_string(ctx, "dialogConstructed");
		duk_call_method(ctx, 1);
		duk_pop(ctx);
		break;

	case TDN_VERIFICATION_CLICKED:
		duk_push_string(ctx, "verificationClicked");
		duk_push_boolean(ctx, !!wParam);
		duk_call_method(ctx, 2);
		duk_pop(ctx);
		break;

	case TDN_HELP:
		duk_push_string(ctx, "help");
		duk_call_method(ctx, 1);
		duk_pop(ctx);
		break;

	case TDN_EXPANDO_BUTTON_CLICKED:
		duk_push_string(ctx, "expandoButtonClicked");
		duk_push_boolean(ctx, !!wParam);
		duk_call_method(ctx, 2);
		duk_pop(ctx);
		break;
	}
	return result;
}