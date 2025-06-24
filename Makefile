# VOICEVOX v0.16.0 C API サンプルのMakefile（辞書パス対応版）

# 環境変数またはデフォルトパス
VOICEVOX_DIR ?= $(HOME)/.voicevox/squashfs-root/vv-engine

# コンパイル設定
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -DVOICEVOX_LINK_ONNXRUNTIME
LDFLAGS = -L$(VOICEVOX_DIR) -lvoicevox_core -lonnxruntime
INCLUDES = -I.

# ターゲット
TARGET_SIMPLE = 01_simple_tts
SOURCE_SIMPLE = 01_simple_tts.c

TARGET_AUDIOQUERY = 02_audioquery_tts
SOURCE_AUDIOQUERY = 02_audioquery_tts.c

TARGET_STRUCT = 03_struct_analysis
SOURCE_STRUCT = 03_struct_analysis.c


# ビルドルール
$(TARGET_SIMPLE): $(SOURCE_SIMPLE)
	$(CC) $(CFLAGS) -o $@ $< -ldl

$(TARGET_AUDIOQUERY): $(SOURCE_AUDIOQUERY)
	$(CC) $(CFLAGS) -o $@ $< -ldl

$(TARGET_STRUCT): $(SOURCE_STRUCT)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)


# 全てビルド
all: $(TARGET_SIMPLE) $(TARGET_AUDIOQUERY)

# 実行時にライブラリパスを設定
run: $(TARGET_SIMPLE)
	@echo "=== シンプル版実行（辞書パス指定） ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_SIMPLE)

run-simple: $(TARGET_SIMPLE)
	@echo "=== シンプル版実行 ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_SIMPLE)

run-audioquery: $(TARGET_AUDIOQUERY)
	@echo "=== AudioQuery版実行 ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_AUDIOQUERY)

run-struct: $(TARGET_STRUCT)
	@echo "=== 構造体分析版実行 ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_STRUCT)

# クリーンアップ
clean:
	rm -f $(TARGET_SIMPLE) $(TARGET_AUDIOQUERY) $(TARGET_STRUCT) *.wav

# ヘルプ
help:
	@echo "使用方法（辞書パス対応版）:"
	@echo "  make              - シンプル版をビルド・実行（推奨）"
	@echo "  make simple       - シンプル版をビルド"
	@echo "  make audioquery   - AudioQuery版をビルド"
	@echo "  make struct       - 構造体分析版をビルド"
	@echo "  make all          - simple, audioqueryをビルド"
	@echo "  make run          - シンプル版を実行（推奨）"
	@echo "  make run-simple   - シンプル版を実行"
	@echo "  make run-audioquery - AudioQuery版を実行"
	@echo "  make run-struct   - 構造体分析版を実行"
	@echo "  make clean        - 生成ファイルを削除"
	@echo ""
	@echo "環境変数:"
	@echo "  VOICEVOX_DIR  - VOICEVOXエンジンのパス (デフォルト: $(VOICEVOX_DIR))"
	@echo "  例: export VOICEVOX_DIR=/path/to/your/voicevox/engine"
	@echo ""
	@echo "推奨実行順序:"
	@echo "  1. make run         # シンプル版（辞書パス指定）"
	@echo "  2. make run-audioquery # AudioQuery経由版"
	@echo "  3. make run-struct  # 構造体分析版"

# エイリアス
simple: $(TARGET_SIMPLE)
audioquery: $(TARGET_AUDIOQUERY)
struct: $(TARGET_STRUCT)

.PHONY: all run run-simple run-audioquery run-struct clean help simple audioquery struct