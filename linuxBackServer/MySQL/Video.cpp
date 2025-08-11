#include "Video.h"
#include <iostream>
#include <sstream>
#include <string>

Video::Video(std::string databaseName) : MySql(databaseName) {}

void Video::addVideo(std::string filename, std::string path, int totalFrame, std::string coverPath, std::string createTime, int userid) {
    std::ostringstream oss;
    oss << "INSERT INTO video (filename, path, totalFrame, coverPath, createTime, userid) VALUES ('"
        << filename << "', '" << path << "', " << totalFrame << ", '" << coverPath << "', '" << createTime << "', " << userid << ")";
    executeUpdate(oss.str());
}

void Video::deleteVideoById(int id) {
    std::ostringstream oss;
    oss << "DELETE FROM video WHERE id=" << id;
    executeUpdate(oss.str());
}

bool Video::findVideoById(int id) {
    std::ostringstream oss;
    oss << "SELECT * FROM video WHERE id=" << id;
    int rowCount = 0;
    executeQuery(oss.str(), [&rowCount](sql::ResultSet* res) {
        rowCount = res->rowsCount();
    });
    return rowCount > 0;
}

std::string Video::getVideoPathById(int id) {
    std::ostringstream oss;
    oss << "SELECT path FROM video WHERE id=" << id;
    std::string path = "";
    executeQuery(oss.str(), [&path](sql::ResultSet* res) {
        if (res->next()) {
            path = res->getString("path");
        }
    });
    return path;
}

std::vector<VIDEO_ITEM> Video::getVideosByUserId(int userid, int limit) {
    std::ostringstream oss;
    oss << "SELECT filename, coverPath FROM video WHERE userid=" << userid << " LIMIT " << limit;
    std::vector<VIDEO_ITEM> videoList;
    executeQuery(oss.str(), [&videoList](sql::ResultSet* res) {
        while (res->next()) {
            VIDEO_ITEM item;
            // 使用std::string的赋值操作来避免缓冲区问题
            strcpy(item.videoName,res->getString("filename").c_str());
            strcpy(item.coverPath,res->getString("coverPath").c_str());
            videoList.push_back(item);
        }
    });
    return videoList;
} 

std::vector<VIDEO_ITEM> Video::getVideosByUserIdAndPage(int userid, int page, const std::string& date) {
    // 每页显示 15 个视频
    const int limit = 15;
    // 计算偏移量
    int offset = (page - 1) * limit;

    std::ostringstream oss;
    oss << "SELECT filename, coverPath FROM video WHERE userid=" << userid;

    if (!date.empty()) {
        if (date.length() == 10) { // 格式为 "%04d-%02d-%02d"
            oss << " AND DATE(createTime) = '" << date << "'";
        } else if (date.length() == 7) { // 格式为 "%04d-%02d"
            oss << " AND DATE_FORMAT(createTime, '%Y-%m') = '" << date << "'";
        }
    }

    oss << " LIMIT " << limit << " OFFSET " << offset;

    std::vector<VIDEO_ITEM> videoList;
    executeQuery(oss.str(), [&videoList](sql::ResultSet* res) {
        while (res->next()) {
            VIDEO_ITEM item;
            strcpy(item.videoName,res->getString("filename").c_str());
            strcpy(item.coverPath,res->getString("coverPath").c_str());
            videoList.push_back(item);
        }
    });
    return videoList;
}

VideoInfo Video::getVideoInfoByUserIdAndFilename(int userid, const std::string& filename) {
    std::ostringstream oss;
    oss << "SELECT filename, path, totalFrame, recordFrame FROM video WHERE userid=" << userid << " AND filename='" << filename << "'";

    VideoInfo info;
    info.filename = "";
    info.path = "";
    info.totalFrame = 0;
    info.currentFrame = 0;

    executeQuery(oss.str(), [&info](sql::ResultSet* res) {
        if (res->next()) {
            info.filename = res->getString("filename");
            info.path = res->getString("path");
            info.totalFrame = res->getInt("totalFrame");
            info.currentFrame = res->getInt("recordFrame");
        }
    });

    return info;
}

void Video::updateCurrentFrameByUserIdAndFilename(int userid, const std::string& filename, int currentFrame) {
    std::ostringstream oss;
    oss << "UPDATE video SET recordFrame = " << currentFrame
        << " WHERE userid = " << userid
        << " AND filename = '" << filename << "'";
    executeUpdate(oss.str());
}