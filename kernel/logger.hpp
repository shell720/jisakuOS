// カーネルロガーの実装

#pragma once

enum LogLevel{
    kError = 3,
    kWarn = 4,
    kInfo = 6,
    kDebug = 7,
};

// グローバルなログ優先度のしきい値をlevelに設定する
// 以降のLogの呼び出しでは、ここで設定した優先度以上のログのみ記録される
void SetLogLevel(LogLevel level);

// 指定された優先度以上のログを記録する．
// @param level  ログの優先度。しきい値以上の優先度のログのみが記録される
// @param format  書式文字列。printk と互換あり
int Log(LogLevel level, const char* format, ...);