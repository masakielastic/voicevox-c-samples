#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

// VOICEVOX汎用音声生成ツール
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

typedef struct {
    char* text;
    char* output;
    char* player;
    char* search_speaker;
    uint32_t speaker_id;
    float speed_scale;
    float pitch_scale;
    float volume_scale;
    float pre_silence;
    float post_silence;
    bool play_audio;
    bool temp_file;
    bool list_speakers;
    bool show_help;
    bool show_speaker_names;
    bool show_speaker_ids;
    bool show_all_ids;
} VoiceConfig;

// AudioQuery JSON内のパラメータを変更する関数
char* modify_audio_query(const char* original_json, const VoiceConfig* config) {
    size_t json_len = strlen(original_json);
    char* modified_json = malloc(json_len + 2000);
    if (!modified_json) return NULL;
    
    strcpy(modified_json, original_json);
    
    // speed_scale
    char* speed_pos = strstr(modified_json, "\"speed_scale\":");
    if (speed_pos) {
        char* end_pos = strchr(speed_pos + 13, ',');
        if (!end_pos) end_pos = strchr(speed_pos + 13, '}');
        if (end_pos) {
            char new_speed[50];
            snprintf(new_speed, sizeof(new_speed), "\"speed_scale\":%.2f", config->speed_scale);
            size_t new_len = strlen(new_speed);
            memmove(speed_pos + new_len, end_pos, strlen(end_pos) + 1);
            memcpy(speed_pos, new_speed, new_len);
        }
    }
    
    // pitch_scale
    char* pitch_pos = strstr(modified_json, "\"pitch_scale\":");
    if (pitch_pos) {
        char* end_pos = strchr(pitch_pos + 13, ',');
        if (!end_pos) end_pos = strchr(pitch_pos + 13, '}');
        if (end_pos) {
            char new_pitch[50];
            snprintf(new_pitch, sizeof(new_pitch), "\"pitch_scale\":%.2f", config->pitch_scale);
            size_t new_len = strlen(new_pitch);
            memmove(pitch_pos + new_len, end_pos, strlen(end_pos) + 1);
            memcpy(pitch_pos, new_pitch, new_len);
        }
    }
    
    // volume_scale
    char* volume_pos = strstr(modified_json, "\"volume_scale\":");
    if (volume_pos) {
        char* end_pos = strchr(volume_pos + 14, ',');
        if (!end_pos) end_pos = strchr(volume_pos + 14, '}');
        if (end_pos) {
            char new_volume[50];
            snprintf(new_volume, sizeof(new_volume), "\"volume_scale\":%.2f", config->volume_scale);
            size_t new_len = strlen(new_volume);
            memmove(volume_pos + new_len, end_pos, strlen(end_pos) + 1);
            memcpy(volume_pos, new_volume, new_len);
        }
    }
    
    // pre_phoneme_length
    char* pre_pos = strstr(modified_json, "\"pre_phoneme_length\":");
    if (pre_pos) {
        char* end_pos = strchr(pre_pos + 20, ',');
        if (!end_pos) end_pos = strchr(pre_pos + 20, '}');
        if (end_pos) {
            char new_pre[50];
            snprintf(new_pre, sizeof(new_pre), "\"pre_phoneme_length\":%.2f", config->pre_silence);
            size_t new_len = strlen(new_pre);
            memmove(pre_pos + new_len, end_pos, strlen(end_pos) + 1);
            memcpy(pre_pos, new_pre, new_len);
        }
    }
    
    // post_phoneme_length
    char* post_pos = strstr(modified_json, "\"post_phoneme_length\":");
    if (post_pos) {
        char* end_pos = strchr(post_pos + 21, ',');
        if (!end_pos) end_pos = strchr(post_pos + 21, '}');
        if (end_pos) {
            char new_post[50];
            snprintf(new_post, sizeof(new_post), "\"post_phoneme_length\":%.2f", config->post_silence);
            size_t new_len = strlen(new_post);
            memmove(post_pos + new_len, end_pos, strlen(end_pos) + 1);
            memcpy(post_pos, new_post, new_len);
        }
    }
    
    return modified_json;
}

// 音声ファイルを再生する関数
void play_audio_file(const char* filename, const char* player) {
    if (!player) {
        printf("音声プレイヤーが指定されていません\n");
        return;
    }
    
    printf("音声再生中: %s (プレイヤー: %s)\n", filename, player);
    
    pid_t pid = fork();
    if (pid == 0) {
        execlp(player, player, filename, NULL);
        printf("音声プレイヤーの実行に失敗: %s\n", player);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("再生完了: %s\n", filename);
        } else {
            printf("再生エラー: %s\n", filename);
        }
    } else {
        printf("プロセス作成失敗\n");
    }
}

// 話者名のみを表示
void print_speaker_names() {
    FILE* fp = fopen("metas.json", "r");
    if (!fp) {
        printf("エラー: metas.json が見つかりません\n");
        return;
    }
    
    printf("=== 話者名一覧 ===\n");
    char buffer[65536];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    fclose(fp);
    
    // 各オブジェクトの最初の "name" フィールドのみを取得
    char* pos = buffer;
    while ((pos = strstr(pos, "{\n    \"name\": \"")) != NULL) {
        pos += 15; // "{\n    \"name\": \"" の長さ
        char* end_pos = strchr(pos, '"');
        if (end_pos) {
            char speaker_name[256];
            size_t name_len = end_pos - pos;
            if (name_len >= sizeof(speaker_name)) name_len = sizeof(speaker_name) - 1;
            strncpy(speaker_name, pos, name_len);
            speaker_name[name_len] = '\0';
            printf("%s\n", speaker_name);
        }
        pos = end_pos ? end_pos : pos + 1;
    }
}

// 話者とIDの対応を表示
void print_speaker_ids() {
    FILE* fp = fopen("metas.json", "r");
    if (!fp) {
        printf("エラー: metas.json が見つかりません\n");
        return;
    }
    
    printf("=== 話者とID一覧 ===\n");
    char buffer[65536];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    fclose(fp);
    
    // 簡易的なJSONパース（話者名とIDを抽出）
    char* pos = buffer;
    while ((pos = strstr(pos, "\"name\": \"")) != NULL) {
        pos += 9;
        char* name_end = strchr(pos, '"');
        if (!name_end) break;
        
        char speaker_name[256];
        size_t name_len = name_end - pos;
        if (name_len >= sizeof(speaker_name)) name_len = sizeof(speaker_name) - 1;
        strncpy(speaker_name, pos, name_len);
        speaker_name[name_len] = '\0';
        
        printf("%s: ", speaker_name);
        
        // この話者のIDを探す
        char* styles_pos = strstr(pos, "\"styles\": [");
        if (styles_pos) {
            styles_pos += 11;
            char* styles_end = strstr(styles_pos, "]");
            if (styles_end) {
                bool first_id = true;
                char* id_pos = styles_pos;
                while (id_pos < styles_end && (id_pos = strstr(id_pos, "\"id\": ")) != NULL) {
                    id_pos += 6;
                    
                    // typeがframe_decodeやsingでないかチェック
                    char* type_check = id_pos;
                    while (type_check < styles_end && *type_check != '}') {
                        if (strncmp(type_check, "\"type\": \"", 9) == 0) {
                            type_check += 9;
                            if (strncmp(type_check, "frame_decode", 12) == 0 || 
                                strncmp(type_check, "sing", 4) == 0) {
                                goto next_id;
                            }
                            break;
                        }
                        type_check++;
                    }
                    
                    int id = atoi(id_pos);
                    if (!first_id) printf(", ");
                    printf("%d", id);
                    first_id = false;
                    
                    next_id:
                    while (id_pos < styles_end && *id_pos != '}') id_pos++;
                    if (id_pos < styles_end) id_pos++;
                }
            }
        }
        printf("\n");
        pos = name_end;
    }
}

// 全ての使用可能IDを表示
void print_all_ids() {
    FILE* fp = fopen("metas.json", "r");
    if (!fp) {
        printf("エラー: metas.json が見つかりません\n");
        return;
    }
    
    printf("=== 使用可能ID一覧 ===\n");
    char buffer[65536];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    fclose(fp);
    
    int ids[1000];
    int id_count = 0;
    
    char* pos = buffer;
    while ((pos = strstr(pos, "\"id\": ")) != NULL) {
        pos += 6;
        
        // typeがframe_decodeやsingでないかチェック
        char* type_check = pos;
        char* brace_end = strchr(pos, '}');
        if (brace_end) {
            while (type_check < brace_end) {
                if (strncmp(type_check, "\"type\": \"", 9) == 0) {
                    type_check += 9;
                    if (strncmp(type_check, "frame_decode", 12) == 0 || 
                        strncmp(type_check, "sing", 4) == 0) {
                        goto next_id2;
                    }
                    break;
                }
                type_check++;
            }
        }
        
        int id = atoi(pos);
        if (id_count < 1000) {
            ids[id_count++] = id;
        }
        
        next_id2:
        while (*pos && *pos != '}') pos++;
        if (*pos) pos++;
    }
    
    // ソート
    for (int i = 0; i < id_count - 1; i++) {
        for (int j = i + 1; j < id_count; j++) {
            if (ids[i] > ids[j]) {
                int temp = ids[i];
                ids[i] = ids[j];
                ids[j] = temp;
            }
        }
    }
    
    // 表示
    for (int i = 0; i < id_count; i++) {
        if (i > 0) printf(", ");
        printf("%d", ids[i]);
    }
    printf("\n");
}

// 特定の話者を検索
void search_speaker(const char* search_term) {
    FILE* fp = fopen("metas.json", "r");
    if (!fp) {
        printf("エラー: metas.json が見つかりません\n");
        return;
    }
    
    printf("=== '%s' を含む話者の検索結果 ===\n", search_term);
    char buffer[65536];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, fp);
    buffer[len] = '\0';
    fclose(fp);
    
    // 各話者オブジェクトを個別に処理
    char* pos = buffer;
    while ((pos = strstr(pos, "{\n    \"name\": \"")) != NULL) {
        pos += 15; // "{\n    \"name\": \"" の長さ
        
        // 話者名の終端を見つける
        char* name_end = strchr(pos, '"');
        if (!name_end) break;
        
        char speaker_name[256];
        size_t name_len = name_end - pos;
        if (name_len >= sizeof(speaker_name)) name_len = sizeof(speaker_name) - 1;
        strncpy(speaker_name, pos, name_len);
        speaker_name[name_len] = '\0';
        
        // この話者オブジェクトの終端を見つける
        char* speaker_end = pos;
        int brace_count = 1;
        while (*speaker_end && brace_count > 0) {
            if (*speaker_end == '{') brace_count++;
            else if (*speaker_end == '}') brace_count--;
            speaker_end++;
        }
        
        // 検索語が含まれているかチェック
        if (strstr(speaker_name, search_term)) {
            printf("\n話者名: %s\n", speaker_name);
            printf("スタイル:\n");
            
            // この話者内でスタイルを探す
            char* search_area = pos;
            char* styles_pos = strstr(search_area, "\"styles\": [");
            if (styles_pos && styles_pos < speaker_end) {
                styles_pos += 11;
                char* styles_end = strstr(styles_pos, "]");
                if (styles_end && styles_end < speaker_end) {
                    char* style_pos = styles_pos;
                    while (style_pos < styles_end && (style_pos = strstr(style_pos, "{")) != NULL && style_pos < styles_end) {
                        style_pos++;
                        char* style_end = strchr(style_pos, '}');
                        if (!style_end || style_end > styles_end) break;
                        
                        char* style_name_pos = strstr(style_pos, "\"name\": \"");
                        char* id_pos = strstr(style_pos, "\"id\": ");
                        char* type_pos = strstr(style_pos, "\"type\": \"");
                        
                        if (style_name_pos && id_pos && 
                            style_name_pos < style_end && id_pos < style_end &&
                            style_name_pos > style_pos && id_pos > style_pos) {
                            
                            // typeチェック（frame_decodeとsingを除外）
                            if (type_pos && type_pos < style_end && type_pos > style_pos) {
                                type_pos += 9;
                                if (strncmp(type_pos, "frame_decode", 12) == 0 || 
                                    strncmp(type_pos, "sing", 4) == 0) {
                                    style_pos = style_end;
                                    continue;
                                }
                            }
                            
                            style_name_pos += 9;
                            char* style_name_end = strchr(style_name_pos, '"');
                            id_pos += 6;
                            
                            if (style_name_end && style_name_end < style_end) {
                                char style_name[128];
                                size_t sname_len = style_name_end - style_name_pos;
                                if (sname_len >= sizeof(style_name)) sname_len = sizeof(style_name) - 1;
                                strncpy(style_name, style_name_pos, sname_len);
                                style_name[sname_len] = '\0';
                                
                                int id = atoi(id_pos);
                                printf("  ID %d: %s\n", id, style_name);
                            }
                        }
                        style_pos = style_end;
                    }
                }
            }
        }
        
        // 次の話者オブジェクトに移動
        pos = speaker_end;
    }
}

// 話者一覧を表示
void print_speakers() {
    printf("=== 利用可能な話者一覧 ===\n");
    printf("ずんだもん:\n");
    printf("  ID 1: あまあま\n");
    printf("  ID 3: ノーマル\n");
    printf("  ID 5: セクシー\n");
    printf("  ID 7: ツンツン\n");
    printf("  ID 22: ささやき\n");
    printf("  ID 38: ヒソヒソ\n");
    printf("  ID 75: ヘロヘロ\n");
    printf("  ID 76: なみだめ\n\n");
    
    printf("四国めたん:\n");
    printf("  ID 0: あまあま\n");
    printf("  ID 2: ノーマル\n");
    printf("  ID 4: セクシー\n");
    printf("  ID 6: ツンツン\n");
    printf("  ID 36: ささやき\n");
    printf("  ID 37: ヒソヒソ\n\n");
    
    printf("九州そら:\n");
    printf("  ID 15: あまあま\n");
    printf("  ID 16: ノーマル\n");
    printf("  ID 17: セクシー\n");
    printf("  ID 18: ツンツン\n");
    printf("  ID 19: ささやき\n\n");
    
    printf("春日部つむぎ: ID 8\n");
    printf("雨晴はう: ID 10\n");
    printf("波音リツ: ID 9, 65\n");
    printf("玄野武宏: ID 11, 39, 40, 41\n");
    printf("白上虎太郎: ID 12, 32, 33, 34, 35\n");
    printf("青山龍星: ID 13, 81-86\n");
    printf("冥鳴ひまり: ID 14\n");
    printf("もち子さん: ID 20, 66, 77-80\n");
    printf("剣崎雌雄: ID 21\n");
    printf("ホワイトシーユーエル: ID 23-26\n");
    printf("後鬼: ID 27, 28, 87, 88\n");
    printf("ナンバーセブン: ID 29-31\n");
    printf("ちび式じい: ID 42\n");
    printf("櫻歌ミコ: ID 43-45\n");
    printf("小夜: ID 46\n");
    printf("ナースロボタイプティー: ID 47-50\n");
    printf("聖騎士紅桜: ID 51\n");
    printf("雀松朱司: ID 52\n");
    printf("麒ヶ島宗麟: ID 53\n");
    printf("春歌ナナ: ID 54\n");
    printf("猫使アル: ID 55-57\n");
    printf("猫使ビィ: ID 58-60\n");
    printf("中国うさぎ: ID 61-64\n");
    printf("栗田まろん: ID 67\n");
    printf("あいえるたん: ID 68\n");
    printf("満別花丸: ID 69-73\n");
    printf("琴詠ニア: ID 74\n");
    printf("ボイドール: ID 89\n");
    printf("ぞん子: ID 90-93\n");
    printf("中部つるぎ: ID 94-98\n");
    printf("離途: ID 99\n");
    printf("黒沢冴白: ID 100\n");
    printf("\n詳細な話者情報とスタイル一覧:\n");
    printf("  全話者名: cat metas.json | jq -r '.[].name'\n");
    printf("  特定話者検索: cat metas.json | jq '.[] | select(.name | contains(\"ずんだもん\"))'\n");
    printf("  話者とID一覧: cat metas.json | jq -r '.[] | \"\\(.name): \\([.styles[] | select(has(\"type\") | not) | .id] | join(\", \"))\"'\n");
    printf("  使用可能ID: cat metas.json | jq -r '[.[] | .styles[] | select(has(\"type\") | not) | .id] | sort | join(\", \")'\n");
}

void print_usage(const char* program_name) {
    printf("ボイスボックス汎用音声生成ツール\n\n");
    printf("使用法: %s \"テキスト\" [オプション]\n\n", program_name);
    printf("オプション:\n");
    printf("  --speaker ID         Speaker ID (デフォルト: 3)\n");
    printf("  --speed VALUE        話速 (0.5-2.0, デフォルト: 1.0)\n");
    printf("  --pitch VALUE        音高 (-0.15-0.15, デフォルト: 0.0)\n");
    printf("  --volume VALUE       音量 (0.0-2.0, デフォルト: 1.0)\n");
    printf("  --pre-silence VALUE  開始無音 (秒, デフォルト: 0.1)\n");
    printf("  --post-silence VALUE 終了無音 (秒, デフォルト: 0.1)\n");
    printf("  --output FILE        出力ファイル名 (デフォルト: output.wav)\n");
    printf("  --play [PLAYER]      生成後に再生\n");
    printf("  --temp              一時ファイル（再生後削除）\n");
    printf("  --list-speakers     話者一覧表示\n");
    printf("  --speaker-names     話者名のみ表示\n");
    printf("  --speaker-ids       話者とID対応表示\n");
    printf("  --all-ids           使用可能ID一覧表示\n");
    printf("  --search NAME       特定話者を検索\n");
    printf("  --help              このヘルプを表示\n\n");
    printf("使用例:\n");
    printf("  %s \"こんにちは\"                           # 基本生成（再生なし）\n", program_name);
    printf("  %s \"こんにちは\" --play                    # 生成+再生\n", program_name);
    printf("  %s \"こんにちは\" --speaker 1 --speed 1.5   # 話者・速度指定+再生\n", program_name);
    printf("  %s \"こんにちは\" --temp                    # 一時再生\n", program_name);
    printf("  %s \"元気です\" --speaker 1 --pitch 0.1 --volume 1.2  # 複数パラメータ\n", program_name);
    printf("\n話者検索例:\n");
    printf("  %s --speaker-names                        # 全話者名\n", program_name);
    printf("  %s --speaker-ids                          # 話者とID対応\n", program_name);
    printf("  %s --search ずんだもん                     # 話者検索\n", program_name);
    printf("  %s --all-ids                              # 使用可能ID\n", program_name);
}

int generate_voice(const VoiceConfig* config, void* lib) {
    // 関数ポインタを取得
    VoicevoxResultCode (*audio_query_func)(const char*, uint32_t, VoicevoxAudioQueryOptionsManual, char**) = dlsym(lib, "voicevox_audio_query");
    VoicevoxResultCode (*synthesis_func)(const char*, uint32_t, VoicevoxSynthesisOptionsManual, uintptr_t*, uint8_t**) = dlsym(lib, "voicevox_synthesis");
    void (*wav_free_func)(uint8_t*) = dlsym(lib, "voicevox_wav_free");
    void (*json_free_func)(char*) = dlsym(lib, "voicevox_audio_query_json_free");
    
    printf("音声生成中: %s\n", config->text);
    printf("Speaker ID: %u\n", config->speaker_id);
    printf("パラメータ: 話速=%.2f, 音高=%.2f, 音量=%.2f\n", 
           config->speed_scale, config->pitch_scale, config->volume_scale);
    printf("無音時間: 開始=%.2f秒, 終了=%.2f秒\n", config->pre_silence, config->post_silence);
    
    // AudioQuery生成
    VoicevoxAudioQueryOptionsManual aq_opts = { .kana = false };
    char* audio_query_json = NULL;
    VoicevoxResultCode result = audio_query_func(config->text, config->speaker_id, aq_opts, &audio_query_json);
    
    if (result != 0 || !audio_query_json) {
        printf("オーディオクエリー生成失敗: %d\n", result);
        return 1;
    }
    
    // パラメータを変更
    char* modified_json = modify_audio_query(audio_query_json, config);
    if (!modified_json) {
        printf("ジェーソン修正失敗\n");
        if (json_free_func) json_free_func(audio_query_json);
        return 1;
    }
    
    // 音声合成
    VoicevoxSynthesisOptionsManual syn_opts = { .enable_interrogative_upspeak = true };
    uint8_t* wav_data = NULL;
    uintptr_t wav_length = 0;
    
    result = synthesis_func(modified_json, config->speaker_id, syn_opts, &wav_length, &wav_data);
    
    if (result == 0 && wav_data) {
        const char* filename = config->temp_file ? "temp_voice.wav" : config->output;
        FILE* fp = fopen(filename, "wb");
        if (fp) {
            fwrite(wav_data, 1, wav_length, fp);
            fclose(fp);
            printf("保存完了: %s (サイズ: %zu bytes)\n", filename, wav_length);
            
            // 再生
            if (config->play_audio && config->player) {
                play_audio_file(filename, config->player);
            }
            
            // 一時ファイル削除
            if (config->temp_file) {
                unlink(filename);
                printf("一時ファイル削除完了\n");
            }
        } else {
            printf("ファイル保存失敗: %s\n", filename);
        }
        
        if (wav_free_func) wav_free_func(wav_data);
    } else {
        printf("音声合成失敗: %d\n", result);
    }
    
    // メモリ解放
    free(modified_json);
    if (json_free_func) json_free_func(audio_query_json);
    
    return (result == 0) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    VoiceConfig config = {
        .text = NULL,
        .output = "output.wav",
        .player = NULL,
        .search_speaker = NULL,
        .speaker_id = 3,
        .speed_scale = 1.0f,
        .pitch_scale = 0.0f,
        .volume_scale = 1.0f,
        .pre_silence = 0.1f,
        .post_silence = 0.1f,
        .play_audio = false,
        .temp_file = false,
        .list_speakers = false,
        .show_help = false,
        .show_speaker_names = false,
        .show_speaker_ids = false,
        .show_all_ids = false
    };

    static struct option long_options[] = {
        {"speaker", required_argument, 0, 's'},
        {"speed", required_argument, 0, 'S'},
        {"pitch", required_argument, 0, 'p'},
        {"volume", required_argument, 0, 'v'},
        {"pre-silence", required_argument, 0, 'P'},
        {"post-silence", required_argument, 0, 'O'},
        {"output", required_argument, 0, 'o'},
        {"play", optional_argument, 0, 'l'},
        {"temp", no_argument, 0, 't'},
        {"list-speakers", no_argument, 0, 'L'},
        {"speaker-names", no_argument, 0, 'N'},
        {"speaker-ids", no_argument, 0, 'I'},
        {"all-ids", no_argument, 0, 'A'},
        {"search", required_argument, 0, 'R'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "s:S:p:v:P:O:o:l::tLNIAR:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 's': config.speaker_id = atoi(optarg); break;
            case 'S': config.speed_scale = atof(optarg); break;
            case 'p': config.pitch_scale = atof(optarg); break;
            case 'v': config.volume_scale = atof(optarg); break;
            case 'P': config.pre_silence = atof(optarg); break;
            case 'O': config.post_silence = atof(optarg); break;
            case 'o': config.output = optarg; break;
            case 'l': 
                config.play_audio = true;
                if (optarg) config.player = optarg;
                break;
            case 't': config.temp_file = true; break;
            case 'L': config.list_speakers = true; break;
            case 'N': config.show_speaker_names = true; break;
            case 'I': config.show_speaker_ids = true; break;
            case 'A': config.show_all_ids = true; break;
            case 'R': config.search_speaker = optarg; break;
            case 'h': config.show_help = true; break;
            default: print_usage(argv[0]); return 1;
        }
    }

    if (config.show_help) {
        print_usage(argv[0]);
        return 0;
    }

    if (config.list_speakers) {
        print_speakers();
        return 0;
    }
    
    if (config.show_speaker_names) {
        print_speaker_names();
        return 0;
    }
    
    if (config.show_speaker_ids) {
        print_speaker_ids();
        return 0;
    }
    
    if (config.show_all_ids) {
        print_all_ids();
        return 0;
    }
    
    if (config.search_speaker) {
        search_speaker(config.search_speaker);
        return 0;
    }

    if (optind >= argc) {
        printf("エラー: テキストが指定されていません\n");
        print_usage(argv[0]);
        return 1;
    }

    config.text = argv[optind];
    
    
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
    void (*finalize_func)(void) = dlsym(lib, "voicevox_finalize");
    
    if (!init_func || !finalize_func) {
        printf("必要な関数が取得できません\n");
        dlclose(lib);
        return 1;
    }
    
    // 初期化
    char dict_path[512];
    snprintf(dict_path, sizeof(dict_path), "%s/pyopenjtalk/open_jtalk_dic_utf_8-1.11", voicevox_dir);
    
    VoicevoxInitializeOptionsManual opts = {
        .acceleration_mode = 0,
        .cpu_num_threads = 0,
        .load_all_models = true,
        .open_jtalk_dict_dir = dict_path
    };
    
    VoicevoxResultCode result = init_func(opts);
    if (result != 0) {
        printf("初期化失敗: %d\n", result);
        dlclose(lib);
        return 1;
    }
    
    // 音声生成
    int ret = generate_voice(&config, lib);
    
    finalize_func();
    dlclose(lib);
    
    return ret;
}