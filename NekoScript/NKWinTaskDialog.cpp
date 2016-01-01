#include "stdafx.h"
#include "Helper.h"
#include "JSEventEmitter.h"
#include "NKWinTaskDialog.h"

NKWinTaskDialog::NKWinTaskDialog(duk_context *ctx, void *ptr) : JSClass(ctx, ptr) {
	_preventClose = false;
}

NKWinTaskDialog::~NKWinTaskDialog() {
	_ASSERTE(_buttonsRuntime == nullptr);
	_ASSERTE(_radiosRuntime == nullptr);
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

void NKWinTaskDialog::CreateConfig() {
	bool isNull;

	_ASSERTE(_buttonsRuntime == nullptr);
	_ASSERTE(_radiosRuntime == nullptr);

	memset(&_tdc, 0, sizeof(TASKDIALOGCONFIG));
	_tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	_tdc.hInstance = (HINSTANCE) GetModuleHandle(NULL);
	_tdc.pfCallback = &staticCallback;
	_tdc.lpCallbackData = reinterpret_cast<LONG_PTR>(this);

	if (duk_is_object(ctx, 0))
		duk_dup(ctx, 0);
	else
		duk_push_object(ctx);

#define TextField(JSName, NativeName) \
	do {\
		_##JSName = Utf8ToUtf16(JSGetPropString(ctx, #JSName, &isNull)); \
		if (!isNull) _tdc.NativeName = _##JSName.c_str();\
	} while(0)

	TextField(title,                pszWindowTitle);
	TextField(mainInstruction,      pszMainInstruction);
	TextField(content,              pszContent);
	TextField(expandedInformation,  pszExpandedInformation);
	TextField(expandedControlText,  pszExpandedControlText);
	TextField(collapsedControlText, pszCollapsedControlText);
	TextField(footer,               pszFooter);
	TextField(verificationText,     pszVerificationText);

#undef TextField
#define FlagField(JSName, NativeName) \
	do {\
		if (JSGetPropBoolean(ctx, #JSName))\
			_tdc.dwFlags |= NativeName;\
	} while(0)
	
	FlagField(allowDialogCancellation,  TDF_ALLOW_DIALOG_CANCELLATION);
	FlagField(useCommandLinks,          TDF_USE_COMMAND_LINKS);
	FlagField(useCommandLinksNoIcon,    TDF_USE_COMMAND_LINKS_NO_ICON);
	FlagField(enableHyperlinks,         TDF_ENABLE_HYPERLINKS);
	FlagField(expandFooterArea,         TDF_EXPAND_FOOTER_AREA);
	FlagField(expandedByDefault,        TDF_EXPANDED_BY_DEFAULT);
	FlagField(verificationFlagChecked,  TDF_VERIFICATION_FLAG_CHECKED);
	FlagField(showProgressBar,          TDF_SHOW_PROGRESS_BAR);
	FlagField(showMarqueeProgressBar,   TDF_SHOW_MARQUEE_PROGRESS_BAR);
	FlagField(positionRelativeToWindow, TDF_POSITION_RELATIVE_TO_WINDOW);
	FlagField(rtlLayout,                TDF_RTL_LAYOUT);
	FlagField(noDefaultRadioButton,     TDF_NO_DEFAULT_RADIO_BUTTON);
	FlagField(canBeMinimized,           TDF_CAN_BE_MINIMIZED);
	FlagField(sizeToContent,            TDF_SIZE_TO_CONTENT);

#undef FlagField

	_tdc.pszMainIcon = ParseIcon(JSGetPropString(ctx, "icon"));
	_tdc.pszFooterIcon = ParseIcon(JSGetPropString(ctx, "footerIcon"));

	duk_get_prop_string(ctx, -1, "commonButtons");
	if (duk_is_array(ctx, -1)) {
		int length = duk_get_length(ctx, -1);

		for (int i = 0; i < length; i++) {
			_tdc.dwCommonButtons |= ParseCommonButton(JSGetPropString(ctx, i));
		}
	}
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "buttons");
	if (duk_is_array(ctx, -1)) {
		int length = duk_get_length(ctx, -1);
		_buttons.clear();
		_buttons.resize(length);
		_buttonsRuntime = new TASKDIALOG_BUTTON[length];

		for (int i = 0; i < length; i++) {
			_buttons[i] = Utf8ToUtf16(JSGetPropString(ctx, i));
			_buttonsRuntime[i].nButtonID = 1000 + i;
			_buttonsRuntime[i].pszButtonText = _buttons[i].c_str();
		}
		_tdc.pButtons = _buttonsRuntime;
		_tdc.cButtons = length;
	}
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "radioButtons");
	if (duk_is_array(ctx, -1)) {
		int length = duk_get_length(ctx, -1);
		_radios.clear();
		_radios.resize(length);
		_radiosRuntime = new TASKDIALOG_BUTTON[length];

		for (int i = 0; i < length; i++) {
			_radios[i] = Utf8ToUtf16(JSGetPropString(ctx, i));
			_buttonsRuntime[i].nButtonID = 2000 + i;
			_buttonsRuntime[i].pszButtonText = _radios[i].c_str();
		}

		_tdc.pRadioButtons = _buttonsRuntime;
		_tdc.cRadioButtons = length;
	}
	duk_pop(ctx);

	duk_pop(ctx);
}


void NKWinTaskDialog::FreeRuntime() {
	if (_buttonsRuntime) {
		delete[] _buttonsRuntime;
		_buttonsRuntime = 0;
	}

	if (_radiosRuntime) {
		delete[] _radiosRuntime;
		_radiosRuntime = 0;
	}
}

duk_ret_t NKWinTaskDialog::Show() {
	if (_hwnd) {
		SetForegroundWindow(_hwnd);
		return 0;
	}

	HRESULT hr;
	CreateConfig();
	hr = TaskDialogIndirect(&_tdc, NULL, NULL, NULL);
	FreeRuntime();

	if (hr != S_OK) JSThrowWin32Error(ctx, hr, "TaskDialogIndirect");

	return 0;
}

duk_ret_t NKWinTaskDialog::Navigate() {
	if (!_hwnd) {
		return Show();
	}

	LRESULT result;
	CreateConfig();
	result = SendMessage(_hwnd, TDM_NAVIGATE_PAGE, 0, (LPARAM) &_tdc);
	FreeRuntime();

	if (result != S_OK) JSThrowWin32Error(ctx, result, "SendMessage TDM_NAVIGATE_PAGE");

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

	registerMethod<&Show>(ctx, "show", 1);
	registerMethod<&Navigate>(ctx, "navigate", 1);
	registerMethod<&PreventClose>(ctx, "preventClose", 0);

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
		_hwnd = nullptr;

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
		_hwnd = handle;
		FreeRuntime();

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