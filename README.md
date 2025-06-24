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

### 基本的な使い方

```bash
# ヘルプ表示
make help

# シンプル版をビルド・実行（推奨）
make run

# AudioQuery経由版を実行
make run-audioquery

# 構造体分析版を実行
make run-struct
```

### 利用可能なコマンド

```bash
# ビルド
make              # シンプル版をビルド・実行
make simple       # シンプル版をビルド
make audioquery   # AudioQuery版をビルド
make struct       # 構造体分析版をビルド
make all          # 複数をビルド

# 実行
make run          # シンプル版を実行
make run-simple   # シンプル版を実行
make run-audioquery # AudioQuery版を実行
make run-struct   # 構造体分析版を実行

# その他
make clean        # 生成ファイルを削除
```

## 環境変数

| 変数名 | 説明 | デフォルト値 |
|--------|------|-------------|
| `VOICEVOX_DIR` | VOICEVOXエンジンのパス | `$HOME/.voicevox/squashfs-root/vv-engine` |

## ファイル構成

```
voicevox-c-samples/
├── Makefile               # ビルド設定
├── README.md             # このファイル
├── voicevox_core.h       # VOICEVOX C API ヘッダー
├── 01_simple_tts.c       # シンプル音声合成例
├── 02_audioquery_tts.c   # AudioQuery経由音声合成例
└── 03_struct_analysis.c  # 構造体分析例
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

1. **基本関数** → `01_simple_tts.c`
2. **AudioQuery方式** → `02_audioquery_tts.c`  
3. **低レベル処理** → `03_struct_analysis.c`

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