#!/bin/bash
# 增强版清理脚本
echo "=== 清理共享内存和信号量 ==="
sudo rm -f /dev/shm/stopCar_shm
sudo rm -f /dev/shm/sem.stopCar_sem

echo "=== 最终资源状态 ==="
ipcs -a 