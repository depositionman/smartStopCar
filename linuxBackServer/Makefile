# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -I. -I./MySQL -I../tool/memoryPool -I../tool/agreement -std=c++11

# Linker flags
LDFLAGS = -L/usr/lib -lmysqlclient -lmysqlcppconn -pthread -lrt -pthread

# Source files
SRCS = BackMain.cpp \
        MySQL/mySql.cpp \
        MySQL/User.cpp \
        MySQL/ParkInfo.cpp \
        MySQL/CarIn.cpp \
		MySQL/CarOut.cpp \
		MySQL/Video.cpp \
		MySQL/Car.cpp \
        ../tool/agreement/crc.cpp \
        ../tool/memoryPool/memoryPool.cpp

# Object files
OBJ_DIR = obj
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.cpp=.o)))

# Executable name
TARGET = BackMain

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# 创建对象目录
$(OBJ_DIR):
	mkdir -p $@

# 通用编译规则
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 特殊目录规则
$(OBJ_DIR)/%.o: ../tool/memoryPool/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# MySQL目录规则
$(OBJ_DIR)/%.o: MySQL/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# CRC目录规则
$(OBJ_DIR)/%.o: ../tool/agreement/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
