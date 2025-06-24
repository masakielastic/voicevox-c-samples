VOICEVOX C API の使用例 (C 言語版)
================================

## 概要

Linux 環境での VOICEVOX C API サンプルコード

## セットアップ

### 1. VOICEVOX のインストール

Linux 版 VOICEVOX をダウンロードし、AppImage を展開

```bash
cd $HOME/.voicevox
./VOICEVOX.AppImage --appimage-extract
```

### 2. 環境変数の設定

VOICEVOX エンジンのパスを環境変数で指定

```bash
# デフォルトパスの場合
export VOICEVOX_DIR="$HOME/.voicevox/squashfs-root/vv-engine"

# カスタムパスの場合
export VOICEVOX_DIR="/path/to/your/voicevox/engine"
```

### 3. 必要ファイル

- `voicevox_core.h`: [公式リポジトリ](https://github.com/VOICEVOX/voicevox_core/blob/0.16.0/crates/voicevox_core_c_api/include/voicevox_core.h)から入手

## ビルドと実行

### まずは音声生成を試してみよう！

```bash
# ヘルプ表示
make help

# 基本的な音声生成+再生
make voice TEXT="こんにちは"

# 話者を変更（四国めたん）
make voice TEXT="今日の夕食は？" OPTIONS="--speaker 2"

# 話者を変更（ずんだもん）
make voice TEXT="カレーなのだ" OPTIONS="--speaker 3"

# 音声ファイル生成のみ
make gen TEXT="会議資料を作成中"

# 話速を変更  
make voice TEXT="早口で話します" OPTIONS="--speed 1.5"

# 音高を変更
make voice TEXT="高い声で話します" OPTIONS="--pitch 0.1"

# 複数パラメータを組み合わせ
make voice TEXT="高く早く" OPTIONS="--pitch 0.1 --speed 1.5"

# 全オプションを確認
make voice-help
```

### 話者を調べる

```bash
# 全話者名一覧
make voice-names

# 特定話者の詳細（ずんだもんの例）
make voice-search NAME="ずんだもん"

# 使用可能なID一覧
make voice-all-ids
```

### 学習用サンプルコード

```bash
# シンプル版を実行
make run

# AudioQuery経由版を実行
make run-audioquery

# 構造体分析版を実行
make run-struct
```

### その他の利用可能なコマンド

```bash
# ビルド関連
make all          # 全サンプルをビルド
make tool         # 汎用ツールのみビルド
make clean        # 生成ファイルを削除

# 詳細ヘルプ
make voice-help      # 汎用ツール詳細ヘルプ
make gen TEXT="テキスト"          # 音声ファイル生成のみ

# 話者検索
make voice-speakers  # 詳細な話者一覧
make voice-ids       # 話者とID対応表示
```

## 環境変数

| 変数名 | 説明 | デフォルト値 |
|--------|------|-------------|
| `VOICEVOX_DIR` | VOICEVOXエンジンのパス | `$HOME/.voicevox/squashfs-root/vv-engine` |
| `PLAYER` | 音声プレイヤー（汎用ツール用） | 未設定（自動再生は有効だがプレイヤー未指定） |

**音声プレイヤーの設定例:**
```bash
export PLAYER=ffplay    # FFmpeg用（推奨）
export PLAYER=aplay     # ALSA用
export PLAYER=paplay    # PulseAudio用
export PLAYER=mpv       # mpv用
```

## ファイル構成

```
voicevox-c-samples/
├── Makefile               # ビルド設定
├── README.md             # このファイル
├── voicevox_core.h       # VOICEVOX C API ヘッダー
├── 01_simple_tts.c       # シンプル音声合成例
├── 02_audioquery_tts.c   # AudioQuery経由音声合成例
├── 03_struct_analysis.c  # 構造体分析例
├── voicevox_tool.c       # 汎用音声生成ツール（推奨）
└── metas.json            # 話者情報データ
```

## 利用可能なVOICEVOX C API関数

### 基本関数（初級者向け）

**必須関数**
- `voicevox_initialize()` - VOICEVOX初期化
- `voicevox_finalize()` - VOICEVOX終了処理
- `voicevox_tts()` - 直接音声合成（推奨）
- `voicevox_wav_free()` - 音声データメモリ解放

**補助関数**
- `voicevox_get_version()` - バージョン取得
- `voicevox_make_default_tts_options()` - TTSオプション初期化
- `voicevox_error_result_to_message()` - エラーメッセージ取得

**使用例**: `01_simple_tts.c`

### 詳細制御関数（上級者向け）

**AudioQuery方式**
- `voicevox_audio_query()` - AudioQuery生成
- `voicevox_synthesis()` - AudioQueryから音声合成
- `voicevox_make_default_audio_query_options()` - AudioQueryオプション初期化
- `voicevox_make_default_synthesis_options()` - 合成オプション初期化
- `voicevox_audio_query_json_free()` - AudioQueryメモリ解放

**低レベル音響処理**
- `voicevox_predict_duration()` - 継続時間予測
- `voicevox_predict_intonation()` - イントネーション予測
- `voicevox_decode()` - 音響デコード
- `voicevox_predict_duration_data_free()` - 継続時間データ解放
- `voicevox_predict_intonation_data_free()` - イントネーションデータ解放
- `voicevox_decode_data_free()` - デコードデータ解放

**使用例**: `02_audioquery_tts.c`, `03_struct_analysis.c`

### モデル管理関数

- `voicevox_load_model()` - 特定話者モデル読み込み
- `voicevox_is_model_loaded()` - モデル読み込み状態確認
- `voicevox_get_metas_json()` - 話者情報取得
- `voicevox_get_supported_devices_json()` - サポートデバイス情報取得
- `voicevox_is_gpu_mode()` - GPU使用状態確認

### 学習推奨順序

1. **まずは音声生成を楽しむ** → `make voice` コマンドで音声生成を試す
2. **基本的な実装を学ぶ** → `01_simple_tts.c` で基本関数を理解
3. **詳細制御を学ぶ** → `02_audioquery_tts.c` でAudioQuery方式を理解
4. **汎用ツールの実装** → `voicevox_tool.c` で全機能統合の実装を学ぶ
5. **低レベル処理** → `03_struct_analysis.c` で構造体詳細を理解

### 音声パラメータ調整

汎用ツール（`voicevox_tool.c`）では以下のパラメータ調整が可能：

- **話速（speed_scale）**: 0.5～2.0 （1.0が標準）
- **音高（pitch_scale）**: -0.15～0.15 （0.0が標準）
- **音量（volume_scale）**: 0.0～2.0 （1.0が標準）
- **開始無音時間（pre_phoneme_length）**: 秒単位
- **終了無音時間（post_phoneme_length）**: 秒単位

**重要な注意点**: AudioQueryのJSONパラメータは**スネークケース**を使用してください。
- ✅ 正しい: `"speed_scale"`, `"pitch_scale"`, `"volume_scale"`
- ❌ 無効: `"speedScale"`, `"pitchScale"`, `"volumeScale"` (キャメルケースは反映されません)

### AudioQuery JSON パラメータ一覧

AudioQueryのJSONには以下のパラメータが含まれています：

#### 音声調整パラメータ
| パラメータ | 型 | 説明 | デフォルト値 | 推奨範囲 |
|-----------|----|----|-------------|----------|
| `speed_scale` | float | 話速の倍率 | 1.0 | 0.5～2.0 |
| `pitch_scale` | float | 音高の調整値 | 0.0 | -0.15～0.15 |
| `intonation_scale` | float | イントネーションの強さ | 1.0 | 0.0～2.0 |
| `volume_scale` | float | 音量の倍率 | 1.0 | 0.0～2.0 |

#### 音声出力設定
| パラメータ | 型 | 説明 | デフォルト値 |
|-----------|----|----|-------------|
| `pre_phoneme_length` | float | 音声開始前の無音時間（秒） | 0.1 |
| `post_phoneme_length` | float | 音声終了後の無音時間（秒） | 0.1 |
| `output_sampling_rate` | int | 出力サンプリングレート（Hz） | 24000 |
| `output_stereo` | bool | ステレオ出力の有無 | false |

#### 音韻・アクセント情報（読み取り専用）
| パラメータ | 型 | 説明 |
|-----------|----|----|
| `accent_phrases` | array | アクセント句の配列（音韻、継続時間、ピッチ情報） |
| `kana` | string | 読み仮名（カタカナ） |

**注意**: `accent_phrases`と`kana`は音声合成エンジンが生成する読み取り専用データです。通常は変更しません。

### 無音時間調整

汎用ツールでは音声ファイル連結時に重要な無音時間を調整できます：

- **pre_phoneme_length**: 音声開始前の無音時間（秒）
- **post_phoneme_length**: 音声終了後の無音時間（秒）

**用途例**:
- `0.0, 0.0`: 無音なし（継ぎ目なし連結）
- `0.1, 0.1`: 標準設定（自然な間）
- `0.5, 0.5`: 長い間（章や段落の区切り）

**ffmpeg連結例**:
```bash
ffmpeg -i section1.wav -i section2.wav -i section3.wav \
       -filter_complex '[0:0][1:0][2:0]concat=n=3:v=0:a=1[out]' \
       -map '[out]' combined_audio.wav
```

### 音声自動再生

汎用ツールでは環境変数`PLAYER`が設定されている場合に音声を自動再生します：

**環境変数の設定例**:
```bash
export PLAYER=aplay      # ALSA用
export PLAYER=paplay     # PulseAudio用
export PLAYER=ffplay     # FFmpeg用（推奨）
export PLAYER=mpv        # mpv用
```

**使用例**:
```bash
# 音声プレイヤーを設定
export PLAYER=ffplay

# 音声生成+再生（デフォルト）
make voice TEXT="こんにちは"

# 音声ファイル生成のみ
make gen TEXT="こんにちは"
```

**機能**:
- 音声ファイル生成後に自動再生（デフォルト）
- 再生完了を待ってから次の音声を処理
- `--quiet`オプションで再生なしモード
- 開発・テスト時の音声確認が効率的

### 汎用音声生成ツール

`voicevox_tool.c` は全ての機能を統合した実用的なコマンドラインツールです：

**基本使用法**:
```bash
# ツールをビルド
make voicevox_tool

# 基本的な音声生成
./voicevox_tool "こんにちは"

# 話者とパラメータ指定
./voicevox_tool "こんにちは" --speaker 1 --speed 1.5 --pitch 0.1

# 生成後に自動再生
./voicevox_tool "こんにちは" --play

# 一時ファイル（再生後削除）
./voicevox_tool "こんにちは" --temp --play
```

**Makefileからの使用**:
```bash
# 基本生成+再生（デフォルト）
make voice TEXT="こんにちは"

# 音声ファイル生成のみ
make gen TEXT="こんにちは"

# パラメータ付き生成+再生
make voice TEXT="こんにちは" OPTIONS="--speaker 1 --speed 1.5"

# ヘルプ表示
make voice-help

# 話者関連コマンド
make voice-speakers     # 詳細な話者一覧
make voice-names        # 話者名のみ一覧
make voice-ids          # 話者とID対応表示
make voice-all-ids      # 使用可能ID一覧
make voice-search NAME="ずんだもん"  # 特定話者検索
```

**利用可能なオプション**:
- `--speaker ID`: 話者指定（全話者対応）
- `--speed VALUE`: 話速調整（0.5-2.0）
- `--pitch VALUE`: 音高調整（-0.15-0.15）
- `--volume VALUE`: 音量調整（0.0-2.0）
- `--pre-silence VALUE`: 開始無音時間
- `--post-silence VALUE`: 終了無音時間
- `--output FILE`: 出力ファイル名
- `--play [PLAYER]`: 生成後に再生
- `--temp`: 一時ファイル（再生後削除）

このツールにより、音声パラメータ調整、無音時間制御、自動再生などの全ての機能をひとつのコマンドで利用できます。

## トラブルシューティング

### ライブラリが見つからない場合

```bash
# VOICEVOX_DIR が正しく設定されているか確認
echo $VOICEVOX_DIR
ls -la $VOICEVOX_DIR/libvoicevox_core.so

# 環境変数を再設定
export VOICEVOX_DIR="/path/to/correct/voicevox/engine"
```

### 辞書パスエラーの場合

辞書ファイルの存在確認：

```bash
ls -la $VOICEVOX_DIR/pyopenjtalk/open_jtalk_dic_utf_8-1.11/
```

## 検証環境

- Chromebook + Debian 12 Bookworm
- VOICEVOX v0.16.0
- gcc (Debian 12.2.0-14) 12.2.0