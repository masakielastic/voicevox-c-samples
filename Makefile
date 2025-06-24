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

TARGET_TOOL = voicevox_tool
SOURCE_TOOL = voicevox_tool.c


# ビルドルール
$(TARGET_SIMPLE): $(SOURCE_SIMPLE)
	$(CC) $(CFLAGS) -o $@ $< -ldl

$(TARGET_AUDIOQUERY): $(SOURCE_AUDIOQUERY)
	$(CC) $(CFLAGS) -o $@ $< -ldl

$(TARGET_STRUCT): $(SOURCE_STRUCT)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS)

$(TARGET_TOOL): $(SOURCE_TOOL)
	$(CC) $(CFLAGS) -o $@ $< -ldl


# 全てビルド
all: $(TARGET_SIMPLE) $(TARGET_AUDIOQUERY) $(TARGET_STRUCT) $(TARGET_TOOL)

# 実行時にライブラリパスを設定
run: $(TARGET_SIMPLE)
	@echo "=== シンプル版実行（辞書パス指定） ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_SIMPLE)
	@if [ -n "$(PLAYER)" ]; then \
		echo "音声再生中: direct_output.wav"; \
		$(PLAYER) direct_output.wav; \
	fi

run-simple: $(TARGET_SIMPLE)
	@echo "=== シンプル版実行 ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_SIMPLE)
	@if [ -n "$(PLAYER)" ]; then \
		echo "音声再生中: direct_output.wav"; \
		$(PLAYER) direct_output.wav; \
	fi

run-audioquery: $(TARGET_AUDIOQUERY)
	@echo "=== AudioQuery版実行 ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_AUDIOQUERY)
	@if [ -n "$(PLAYER)" ]; then \
		echo "音声再生中: audioquery_output.wav"; \
		$(PLAYER) audioquery_output.wav; \
	fi

run-struct: $(TARGET_STRUCT)
	@echo "=== 構造体分析版実行 ==="
	LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_STRUCT)

# 汎用ツールコマンド
voice: $(TARGET_TOOL)
	@if [ -z "$(TEXT)" ]; then echo "使用法: make voice TEXT=\"テキスト\" [OPTIONS]"; exit 1; fi
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) "$(TEXT)" --play $(OPTIONS)

gen: $(TARGET_TOOL)
	@if [ -z "$(TEXT)" ]; then echo "使用法: make gen TEXT=\"テキスト\" [OPTIONS]"; exit 1; fi
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) "$(TEXT)" $(OPTIONS)

voice-help: $(TARGET_TOOL)
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) --help

voice-speakers: $(TARGET_TOOL)
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) --list-speakers

voice-names: $(TARGET_TOOL)
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) --speaker-names

voice-ids: $(TARGET_TOOL)
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) --speaker-ids

voice-all-ids: $(TARGET_TOOL)
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) --all-ids

voice-search: $(TARGET_TOOL)
	@if [ -z "$(NAME)" ]; then echo "使用法: make voice-search NAME=\"話者名\""; exit 1; fi
	@LD_LIBRARY_PATH=$(VOICEVOX_DIR) ./$(TARGET_TOOL) --search "$(NAME)"

# クリーンアップ
clean:
	rm -f $(TARGET_SIMPLE) $(TARGET_AUDIOQUERY) $(TARGET_STRUCT) $(TARGET_TOOL) *.wav

# ヘルプ
help:
	@echo "=== VOICEVOX C API サンプル ==="
	@echo ""
	@echo "🎵 まずは音声生成を試してみましょう！"
	@echo "  make voice TEXT=\"こんにちは\"                          # 基本音声生成+再生"
	@echo "  make voice TEXT=\"今日の夕食は？\" OPTIONS=\"--speaker 2\"  # 話者変更（四国めたん）"
	@echo "  make voice TEXT=\"カレーなのだ\" OPTIONS=\"--speaker 3\"    # 話者変更（ずんだもん）"
	@echo "  make gen TEXT=\"会議資料を作成中\"                      # ファイル生成のみ"
	@echo "  make voice TEXT=\"早口で話します\" OPTIONS=\"--speed 1.5\"   # 話速変更"
	@echo "  make voice TEXT=\"高い声で話します\" OPTIONS=\"--pitch 0.1\" # 音高変更"
	@echo "  make voice TEXT=\"高く早く\" OPTIONS=\"--pitch 0.1 --speed 1.5\"    # 音高+話速"
	@echo "  make voice-help                                     # 全オプション確認"
	@echo ""
	@echo "🔍 話者を調べる:"
	@echo "  make voice-names                                    # 全話者名一覧"
	@echo "  make voice-search NAME=\"ずんだもん\"                  # ずんだもんの詳細"
	@echo "  make voice-speakers                                 # 詳細な話者一覧"
	@echo "  make voice-all-ids                                  # 使用可能ID一覧"
	@echo ""
	@echo "📚 学習用サンプルコード:"
	@echo "  make run                                            # シンプル版実行"
	@echo "  make run-audioquery                                 # AudioQuery版実行"
	@echo "  make run-struct                                     # 構造体分析版実行"
	@echo ""
	@echo "🔧 開発者向け:"
	@echo "  make all                                            # 全サンプルをビルド"
	@echo "  make tool                                           # 汎用ツールのみビルド"
	@echo "  make clean                                          # 生成ファイルを削除"
	@echo "  make voice-help                                     # 汎用ツール詳細ヘルプ"
	@echo "  make gen TEXT=\"テキスト\"                             # 音声ファイル生成のみ"
	@echo ""
	@echo "⚙️  環境変数:"
	@echo "  export VOICEVOX_DIR=/path/to/voicevox/engine        # VOICEVOXエンジンパス"
	@echo "  export PLAYER=ffplay                               # 音声プレイヤー設定"
	@echo ""
	@echo "📖 詳細な使い方やソースコード解説: README.md をご覧ください"

# エイリアス
simple: $(TARGET_SIMPLE)
audioquery: $(TARGET_AUDIOQUERY)
struct: $(TARGET_STRUCT)
tool: $(TARGET_TOOL)

.PHONY: all run run-simple run-audioquery run-struct clean help simple audioquery struct tool voice gen voice-help voice-speakers voice-names voice-ids voice-all-ids voice-search