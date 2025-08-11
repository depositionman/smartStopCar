#!/bin/bash
echo "=== 创建停车场的数据库 ==="
export MYSQL_PASSWORD=pASSWORD_123
password=$MYSQL_PASSWORD

mysql -u root -p$password < ./stopcar.sql
if [ $? -ne 0 ]; then
    echo "执行 MySQL 脚本时出现错误"
    # 可以在这里添加更详细的错误处理逻辑，比如记录日志等
fi

unset MYSQL_PASSWORD