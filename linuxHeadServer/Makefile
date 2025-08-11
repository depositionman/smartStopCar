CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread -I. -I../tool/agreement -I../tool/memoryPool
LDFLAGS = -lrt -pthread
TARGET = HeadMain
OBJ_DIR = obj
SRCS = HeadMain.cpp \
        threadPool/threadPool.cpp \
        threadPool/baseTask.cpp \
        baseServer/myAddr.cpp \
        baseServer/socketBase.cpp \
        tcpServer/tcpServer.cpp \
        tcpServer/epollServer.cpp \
        client/clientTask.cpp \
        ../tool/agreement/crc.cpp \
        ../tool/memoryPool/memoryPool.cpp
OBJS = $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.cpp=.o)))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LDFLAGS)

$(OBJ_DIR):
	mkdir -p $@

# 通用规则
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 特殊目录规则
$(OBJ_DIR)/threadPool.o: threadPool/threadPool.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/baseTask.o: threadPool/baseTask.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/myAddr.o: baseServer/myAddr.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/socketBase.o: baseServer/socketBase.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/tcpServer.o: tcpServer/tcpServer.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/epollServer.o: tcpServer/epollServer.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/clientTask.o: client/clientTask.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/crc.o: ../tool/agreement/crc.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/memoryPool.o: ../tool/memoryPool/memoryPool.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
