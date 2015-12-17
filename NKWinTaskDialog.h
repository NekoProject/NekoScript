#pragma once
#include "stdafx.h"
#include "JSClass.h"
#include "Helper.h"
#include "NKWinTaskDialog.h"

class NKWinTaskDialog
	: public JSClass<NKWinTaskDialog>
{
public:
	NKWinTaskDialog(duk_context *ctx, void *ptr);
	~NKWinTaskDialog();

	duk_ret_t Show();
	void CreateConfig(TASKDIALOGCONFIG* tdc);

	static void setupPrototype(duk_context *ctx);
};