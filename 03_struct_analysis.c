#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "voicevox_core.h"

// 構造体の詳細分析
int main() {
    printf("=== VOICEVOX 構造体分析版 ===\n");
    
    // 詳細な構造体分析
    printf("構造体詳細分析:\n");
    printf("  VoicevoxInitializeOptions:\n");
    printf("    サイズ: %zu bytes\n", sizeof(VoicevoxInitializeOptions));
    printf("    アライメント: %zu bytes\n", _Alignof(VoicevoxInitializeOptions));
    
    printf("  VoicevoxTtsOptions:\n");
    printf("    サイズ: %zu bytes\n", sizeof(VoicevoxTtsOptions));
    printf("    アライメント: %zu bytes\n", _Alignof(VoicevoxTtsOptions));
    
    printf("  VoicevoxSynthesisOptions:\n");
    printf("    サイズ: %zu bytes\n", sizeof(VoicevoxSynthesisOptions));
    printf("    アライメント: %zu bytes\n", _Alignof(VoicevoxSynthesisOptions));
    
    // デフォルトオプションの中身を確認
    printf("\nデフォルトオプション値確認:\n");
    VoicevoxInitializeOptions init_opts = voicevox_make_default_initialize_options();
    printf("  VoicevoxInitializeOptions の値:\n");
    
    // バイト単位で表示
    unsigned char* bytes = (unsigned char*)&init_opts;
    printf("    バイト列: ");
    for (size_t i = 0; i < sizeof(init_opts); i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
    
    // TTSオプションも確認
    VoicevoxTtsOptions tts_opts = voicevox_make_default_tts_options();
    printf("  VoicevoxTtsOptions の値:\n");
    bytes = (unsigned char*)&tts_opts;
    printf("    バイト列: ");
    for (size_t i = 0; i < sizeof(tts_opts); i++) {
        printf("%02x ", bytes[i]);
    }
    printf("\n");
    
    // より安全な初期化の試行
    printf("\n安全な初期化テスト:\n");
    
    // 最小限の構造体で試行
    printf("  ケース1: デフォルトオプションそのまま\n");
    // これは避ける（セグフォルトの原因）
    
    printf("  ケース2: 全ゼロ初期化\n");
    VoicevoxInitializeOptions zero_opts;
    memset(&zero_opts, 0, sizeof(zero_opts));
    printf("    準備完了（実行はしない）\n");
    
    printf("  ケース3: 8バイト構造体として手動設定\n");
    // 8バイトが何を意味するか推測
    // 可能性1: 64bit値1つ
    // 可能性2: 32bit値2つ  
    // 可能性3: その他の組み合わせ
    
    uint64_t manual_opts_64 = 0;  // 64bit値として
    printf("    64bit値として準備: 0x%016lx\n", manual_opts_64);
    
    struct {
        uint32_t field1;
        uint32_t field2;
    } manual_opts_32x2 = {0, 0};  // 32bit値2つとして
    printf("    32bit×2として準備: {0x%08x, 0x%08x}\n", 
           manual_opts_32x2.field1, manual_opts_32x2.field2);
    
    printf("\n=== 分析完了 ===\n");
    printf("直接呼び出し版を試してください。\n");
    
    return 0;
}