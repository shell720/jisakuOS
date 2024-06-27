#pragma once

#include <cstddef>

// 静的に確保するページディレクトリの個数。この定数はSetupIdentityPageMapで使用される
// 1つのページディレクトリには、512個の 2MiBページを設定できるので
// kPageDirectoryCount x 1GiBの仮想アドレスがマッピングされることになる．
const size_t kPageDirectoryCount = 64;

// アイデンティティページテーブル：仮想アドレス = 物理アドレス
void SetupIdentityPageTable();

void InitializePaging();