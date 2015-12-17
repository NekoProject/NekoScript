#pragma once
#include "stdafx.h"
#include "JSClass.h"
#include "Helper.h"
#include "NKWinTaskDialog.h"

class NKWinTaskDialog
	: public JSClass<NKWinTaskDialog>
{
public:
	NKWinTaskDialog(duk_context *ctx);
	~NKWinTaskDialog();

	duk_ret_t Show();

	static void setupPrototype(duk_context *ctx);
};