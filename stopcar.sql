-- 创建数据库 stopcar
CREATE DATABASE IF NOT EXISTS stopcar
CHARACTER SET utf8mb4
COLLATE utf8mb4_unicode_ci;

-- 使用 stopcar 数据库
USE stopcar;

-- 临时禁用外键约束
SET FOREIGN_KEY_CHECKS = 0;

-- 删除用户表 user
DROP TABLE IF EXISTS `user`;

-- 创建用户表 user，指定表的字符集为 utf8mb4，排序规则为 utf8mb4_unicode_ci
CREATE TABLE `user` (
    -- 自增主键 id，用于唯一标识每条记录
    id INT AUTO_INCREMENT PRIMARY KEY,
    -- 用户名，可变长度字符串，最大长度 50，不允许为空
    name VARCHAR(50) NOT NULL,
    -- 用户密码，可变长度字符串，最大长度 50，不允许为空
    password VARCHAR(50) NOT NULL,
    -- 用户手机号码，可变长度字符串，最大长度 20，唯一且不允许为空
    phone VARCHAR(20) UNIQUE NOT NULL,
    -- 验证码，可变长度字符串，最大长度 6
    captcha VARCHAR(6)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 初始化一个用户数据
INSERT INTO `user`(name,password,phone) VALUES("admin","e10adc3949ba59abbe56e057f20f883e","15871209593");

-- 删除视频表
DROP TABLE IF EXISTS `video`;

-- 创建视频表
CREATE TABLE `video`(
    -- 自增主键 id，用于唯一标识每条记录
    id INT AUTO_INCREMENT PRIMARY KEY,
    -- 视频名称，可变长度字符串，最大长度100，不允许为空
    filename VARCHAR(100) NOT NULL,
    -- 视频路径，可变长度字符串，最大长度100，不允许为空
    path VARCHAR(100) NOT NULL,
    -- 视频封面路径，可变长度字符串，最大长度100，不允许为空
    coverPath VARCHAR(100) NOT NULL,
    -- 视频总帧数，整形，不允许为空
    totalFrame INT NOT NULL,
    -- 视频已播放帧数，整形，默认值为0
    recordFrame INT DEFAULT 0,
    -- 视频创建的时间，DATETIME，不允许为空
    createTime DATETIME NOT NULL,
    -- 添加 userid 字段用于关联用户表
    userid INT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 删除停车信息表
DROP TABLE IF EXISTS `park_info`;

-- 创建停车信息表
CREATE TABLE `park_info`(
    -- 停车场总空间，整形，不允许为空
    totalSpace INT NOT NULL,
    -- 停车场已占用空间，整形，不允许为空
    occupancySpace INT NOT NULL,
    -- 停车场剩余空间，整形，不允许为空
    remainSpace INT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 初始化停车场数据
INSERT INTO `park_info`(totalSpace,occupancySpace,remainSpace) VALUES(500,0,500);

-- 删除车辆出场表
DROP TABLE IF EXISTS `car_out`;

-- 创建车辆出场表
CREATE TABLE `car_out`(
    -- 自增主键 id，用于唯一标识每条记录
    id INT AUTO_INCREMENT PRIMARY KEY,
    -- 车牌号，可变长度字符串，最大长度20，不允许为空
    carNumber VARCHAR(20) NOT NULL,
    -- 出场时间，DATETIME，不允许为空
    comeOutTime DATETIME NOT NULL,
    -- 出场位置，可变长度字符串，最大长度20，不允许为空
    comeOutLocation VARCHAR(20) NOT NULL,
    -- 出场费用，可变长度字符串，最大长度20，不允许为空
    comeOutFee VARCHAR(20) NOT NULL,
    -- 入场时间，DATETIME，不允许为空
    parkTime DATETIME NOT NULL,
    -- 车牌图片路径，可变长度字符串，最大长度100，不允许为空
    platePath VARCHAR(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 删除车辆表
DROP TABLE IF EXISTS `car`;

-- 创建车辆表
CREATE TABLE `car`(
    -- 自增主键 id，用于唯一标识每条记录
    id INT AUTO_INCREMENT PRIMARY KEY,
    -- 车牌号，可变长度字符串，最大长度20，不允许为空
    carNumber VARCHAR(20) UNIQUE NOT NULL,
    -- VIP状态，整形，不允许为空
    VIPStatus INT NOT NULL,
    -- 停车状态，整形，不允许为空
    parkStatus INT NOT NULL,
    -- 用户id
    userid INT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 删除车辆入场表
DROP TABLE IF EXISTS `car_in`;

-- 创建车辆入场表
CREATE TABLE `car_in`(
    -- 自增主键 id，用于唯一标识每条记录
    id INT AUTO_INCREMENT PRIMARY KEY,
    -- 车牌号，可变长度字符串，最大长度20，不允许为空
    carNumber VARCHAR(20) NOT NULL,
    -- 入场时间，DATETIME，不允许为空
    entryTime DATETIME NOT NULL,
    -- 入场位置，可变长度字符串，最大长度20，不允许为空
    entryLocation VARCHAR(20) NOT NULL,
    -- 车牌图片路径，可变长度字符串，最大长度100，不允许为空
    platePath VARCHAR(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 启用外键约束
SET FOREIGN_KEY_CHECKS = 1;