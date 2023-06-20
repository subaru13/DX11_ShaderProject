#pragma once

#ifdef _M_IX86
static_assert(false, "Not compatible with x86 version");
#endif // _M_IX86

//ウィンドウの幅
static CONST LONG		SCREEN_WIDTH{ 1280 };
//ウィンドウの高さ
static CONST LONG		SCREEN_HEIGHT{ 720 };
