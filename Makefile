# Makefile for SDCC project with automatic library discovery
CC = sdcc
CFLAGS = --model-medium --opt-code-size

# 定义库名称（只需要在这里添加）
LIBRARIES = User System

# 单片机型号
MCU = STC8G1K08A
# 串口设备
SERIAL_PORT = /dev/ttyUSB0

# 生成包含目录路径
INCLUDES = $(foreach lib, $(LIBRARIES), -I $(lib)/Inc)

# 查找所有源文件
SRCS = $(foreach lib, $(LIBRARIES), $(wildcard $(lib)/Src/*.c))
# 获取所有.c文件的基本名称（用于检查重名）
BASENAMES = $(foreach src, $(SRCS), $(notdir $(src)))

# 检查是否有重复的源文件名
DUPLICATES = $(filter $(words $(BASENAMES)),$(words $(sort $(BASENAMES))))
ifneq ($(DUPLICATES),$(words $(BASENAMES)))
    $(warning 警告: 发现重复的源文件名！)
    $(warning 重复文件: $(foreach base,$(sort $(foreach b,$(BASENAMES),$(if $(filter 2,$(words $(filter $(b),$(BASENAMES)))),$(b)))),$(base)))
endif

# 生成对象文件列表（保持目录结构）
OBJS = $(patsubst %.c,%.rel,$(SRCS))

# 项目名称
TARGET = pwm

# 构建目录
BUILD_DIR = build

# 将对象文件放在build目录中
OBJS_BUILD = $(patsubst %.rel,$(BUILD_DIR)/%.rel,$(OBJS))

# 默认目标
all: check_libs $(BUILD_DIR) $(BUILD_DIR)/$(TARGET).hex
	@echo "构建完成: $(BUILD_DIR)/$(TARGET).hex"

# 检查库目录是否存在
check_libs:
	@for lib in $(LIBRARIES); do \
		if [ ! -d "$$lib/Inc" ] || [ ! -d "$$lib/Src" ]; then \
			echo "错误: 库 $$lib 结构不正确！需要 $$lib/Inc 和 $$lib/Src 目录"; \
			exit 1; \
		fi; \
	done

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 编译规则：所有.c文件 -> build/目录下的.rel文件
$(BUILD_DIR)/%.rel: %.c
	@echo "编译: $<"
	@mkdir -p $(dir $@)
	@$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ 2>&1 | head -20

# 链接
$(BUILD_DIR)/$(TARGET).ihx: $(OBJS_BUILD)
	@echo "链接: $(words $(OBJS_BUILD)) 个对象文件..."
	@cd $(BUILD_DIR) && $(CC) $(CFLAGS) $(patsubst $(BUILD_DIR)/%,%,$(OBJS_BUILD)) -o $(TARGET).ihx

# 生成HEX
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).ihx
	@echo "生成HEX文件..."
	@cd $(BUILD_DIR) && packihx $(TARGET).ihx > $(TARGET).hex

# 下载到单片机
flash: $(BUILD_DIR)/$(TARGET).hex
	@echo "下载到 $(MCU) via $(SERIAL_PORT)..."
	stcgal -p $(SERIAL_PORT) -t 22168 -o program_eeprom_split=12288 -a $(BUILD_DIR)/$(TARGET).hex

# 编译但不链接（调试用）
compile: $(OBJS_BUILD)
	@echo "编译完成，生成 $(words $(OBJS_BUILD)) 个.rel文件"

# 显示项目信息
info:
	@echo "========================================"
	@echo "项目: $(TARGET)"
	@echo "编译器: $(CC)"
	@echo "库列表: $(LIBRARIES)"
	@echo "包含目录: $(INCLUDES)"
	@echo "源文件 ($(words $(SRCS)) 个):"
	@for src in $(sort $(SRCS)); do echo "  $$src"; done
	@echo "对象文件 ($(words $(OBJS_BUILD)) 个):"
	@for obj in $(sort $(OBJS_BUILD)); do echo "  $$obj"; done
	@echo "构建目录: $(BUILD_DIR)"
	@echo "========================================"

# 清理
clean:
	@echo "清理构建文件..."
	@rm -rf $(BUILD_DIR)
	@rm -f *.ihx *.hex *.rel *.lk *.map *.mem *.rst *.asm *.lst *.sym *.cdb

# 帮助信息
help:
	@echo "可用命令:"
	@echo "  make           - 构建整个项目"
	@echo "  make clean     - 清理所有生成文件"
	@echo "  make flash     - 下载到单片机"
	@echo "  make info      - 显示项目信息"
	@echo "  make compile   - 只编译不链接"
	@echo "  make help      - 显示此帮助信息"

.PHONY: all clean flash info help check_libs compile
