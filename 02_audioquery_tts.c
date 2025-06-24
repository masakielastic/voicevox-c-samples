#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>

// AudioQuery経由での音声合成を試行
typedef uint32_t VoicevoxResultCode;
typedef struct {
    uint32_t acceleration_mode;
    uint16_t cpu_num_threads;
    bool load_all_models;
    const char* open_jtalk_dict_dir;
} VoicevoxInitializeOptionsManual;

typedef struct {
    bool kana;
} VoicevoxAudioQueryOptionsManual;

typedef struct {
    bool enable_interrogative_upspeak;
} VoicevoxSynthesisOptionsManual;

int main() {
    printf("=== VOICEVOX AudioQuery版 ===\n");
    
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
    
    // 関数ポインタを取得
    VoicevoxResultCode (*init_func)(VoicevoxInitializeOptionsManual) = dlsym(lib, "voicevox_initialize");
    VoicevoxResultCode (*audio_query_func)(const char*, uint32_t, VoicevoxAudioQueryOptionsManual, char**) = dlsym(lib, "voicevox_audio_query");
    VoicevoxResultCode (*synthesis_func)(const char*, uint32_t, VoicevoxSynthesisOptionsManual, uintptr_t*, uint8_t**) = dlsym(lib, "voicevox_synthesis");
    void (*finalize_func)(void) = dlsym(lib, "voicevox_finalize");
    void (*wav_free_func)(uint8_t*) = dlsym(lib, "voicevox_wav_free");
    void (*json_free_func)(char*) = dlsym(lib, "voicevox_audio_query_json_free");
    const char* (*version_func)(void) = dlsym(lib, "voicevox_get_version");
    
    if (!init_func || !audio_query_func || !synthesis_func || !finalize_func) {
        printf("必要な関数が取得できません\n");
        dlclose(lib);
        return 1;
    }
    printf("関数ポインタ取得成功\n");
    
    // バージョン確認
    if (version_func) {
        printf("VOICEVOXバージョン: %s\n", version_func());
    }
    
    // 辞書パスを指定して初期化
    char dict_path[512];
    snprintf(dict_path, sizeof(dict_path), "%s/pyopenjtalk/open_jtalk_dic_utf_8-1.11", voicevox_dir);
    
    VoicevoxInitializeOptionsManual opts = {
        .acceleration_mode = 0,
        .cpu_num_threads = 0,
        .load_all_models = true,  // 全モデルロード
        .open_jtalk_dict_dir = dict_path
    };
    
    printf("初期化実行中（辞書パス指定）...\n");
    VoicevoxResultCode result = init_func(opts);
    printf("初期化結果: %d\n", result);
    
    if (result == 0) {
        printf("初期化成功！\n");
        
        // AudioQuery生成
        printf("AudioQuery生成中...\n");
        const char* text = "こんにちは";
        uint32_t speaker_id = 3;
        VoicevoxAudioQueryOptionsManual aq_opts = { .kana = false };
        
        char* audio_query_json = NULL;
        result = audio_query_func(text, speaker_id, aq_opts, &audio_query_json);
        printf("AudioQuery結果: %d\n", result);
        
        if (result == 0 && audio_query_json != NULL) {
            printf("AudioQuery成功！\n");
            printf("AudioQuery JSON (最初の100文字): %.100s\n", audio_query_json);
            
            // 音声合成
            printf("音声合成実行中...\n");
            VoicevoxSynthesisOptionsManual syn_opts = { .enable_interrogative_upspeak = true };
            
            uint8_t* wav_data = NULL;
            uintptr_t wav_length = 0;
            
            result = synthesis_func(audio_query_json, speaker_id, syn_opts, &wav_length, &wav_data);
            printf("音声合成結果: %d, サイズ: %zu\n", result, wav_length);
            
            if (result == 0 && wav_data != NULL) {
                printf("音声合成成功！ファイル保存中...\n");
                FILE* fp = fopen("audioquery_output.wav", "wb");
                if (fp) {
                    fwrite(wav_data, 1, wav_length, fp);
                    fclose(fp);
                    printf("保存完了: audioquery_output.wav\n");
                }
                
                if (wav_free_func) {
                    wav_free_func(wav_data);
                }
            } else {
                printf("音声合成失敗: %d\n", result);
            }
            
            // AudioQuery JSON解放
            if (json_free_func && audio_query_json) {
                json_free_func(audio_query_json);
            }
            
        } else {
            printf("AudioQuery失敗: %d\n", result);
        }
        
        finalize_func();
        printf("終了処理完了\n");
    } else {
        printf("初期化失敗: %d\n", result);
    }
    
    dlclose(lib);
    printf("=== 完了 ===\n");
    return (result == 0) ? 0 : 1;
}