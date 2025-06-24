#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>

// ヘッダーファイルを使わずに関数ポインタで直接呼び出し
// 構造体定義の問題を回避

typedef uint32_t VoicevoxResultCode;
typedef struct {
    uint32_t acceleration_mode;
    uint16_t cpu_num_threads;
    bool load_all_models;
    const char* open_jtalk_dict_dir;
} VoicevoxInitializeOptionsManual;

typedef struct {
    bool kana;
} VoicevoxTtsOptionsManual;

int main() {
    printf("=== VOICEVOX 直接呼び出し版 ===\n");
    
    // ライブラリを動的ロード
    const char* voicevox_dir = getenv("VOICEVOX_DIR");
    if (!voicevox_dir) {
        voicevox_dir = "/home/masakielastic/.voicevox/squashfs-root/vv-engine";
    }
    
    char lib_path[512];
    snprintf(lib_path, sizeof(lib_path), "%s/libvoicevox_core.so", voicevox_dir);
    void* lib = dlopen(lib_path, RTLD_LAZY);
    if (!lib) {
        printf("ライブラリロードエラー: %s\n", dlerror());
        return 1;
    }
    printf("ライブラリロード成功\n");
    
    // 関数ポインタを取得
    VoicevoxResultCode (*init_func)(VoicevoxInitializeOptionsManual) = dlsym(lib, "voicevox_initialize");
    VoicevoxResultCode (*tts_func)(const char*, uint32_t, VoicevoxTtsOptionsManual, uintptr_t*, uint8_t**) = dlsym(lib, "voicevox_tts");
    void (*finalize_func)(void) = dlsym(lib, "voicevox_finalize");
    void (*wav_free_func)(uint8_t*) = dlsym(lib, "voicevox_wav_free");
    const char* (*version_func)(void) = dlsym(lib, "voicevox_get_version");
    
    if (!init_func || !tts_func || !finalize_func) {
        printf("必要な関数が取得できません\n");
        dlclose(lib);
        return 1;
    }
    printf("関数ポインタ取得成功\n");
    
    // バージョン確認
    if (version_func) {
        const char* version = version_func();
        printf("VOICEVOXバージョン: %s\n", version);
    }
    
    // 初期化オプションを手動設定（辞書パスを指定）
    char dict_path[512];
    snprintf(dict_path, sizeof(dict_path), "%s/pyopenjtalk/open_jtalk_dic_utf_8-1.11", voicevox_dir);
    
    VoicevoxInitializeOptionsManual opts = {
        .acceleration_mode = 0,        // CPU
        .cpu_num_threads = 0,          // 自動
        .load_all_models = true,       // 全モデルロード
        .open_jtalk_dict_dir = dict_path  // 辞書パス指定
    };
    
    printf("初期化実行中...\n");
    VoicevoxResultCode result = init_func(opts);
    printf("初期化結果: %d\n", result);
    
    if (result == 0) {  // VOICEVOX_RESULT_OK = 0
        printf("初期化成功！\n");
        
        // TTS実行
        printf("TTS実行中...\n");
        const char* text = "こんにちは";
        uint32_t speaker_id = 3;
        VoicevoxTtsOptionsManual tts_opts = { .kana = false };
        
        uint8_t* wav_data = NULL;
        uintptr_t wav_length = 0;
        
        result = tts_func(text, speaker_id, tts_opts, &wav_length, &wav_data);
        printf("TTS結果: %d, サイズ: %zu\n", result, wav_length);
        
        if (result == 0 && wav_data != NULL) {
            printf("TTS成功！ファイル保存中...\n");
            FILE* fp = fopen("direct_output.wav", "wb");
            if (fp) {
                fwrite(wav_data, 1, wav_length, fp);
                fclose(fp);
                printf("保存完了: direct_output.wav\n");
            }
            
            // WAVデータ解放
            if (wav_free_func) {
                wav_free_func(wav_data);
            }
        } else {
            printf("TTS失敗: %d\n", result);
        }
        
        // 終了処理
        finalize_func();
        printf("終了処理完了\n");
    } else {
        printf("初期化失敗: %d\n", result);
    }
    
    dlclose(lib);
    printf("=== 完了 ===\n");
    return (result == 0) ? 0 : 1;
}