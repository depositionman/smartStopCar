#ifndef __VIDEO_H
#define __VIDEO_H

#include "mySql.h"
#include <string>
#include <vector> 
#include "../agreement/agreement.h"  
#include <cstring>

typedef struct VideoInfo {
    std::string filename;
    std::string path;
    int totalFrame;
    int currentFrame;
}VideoInfo;

class Video : public MySql {
public:
    Video(std::string databaseName);
    ~Video() {}
    void addVideo(std::string filename, std::string path, int totalFrame, std::string coverPath, std::string createTime, int userid); // 添加视频
    void deleteVideoById(int id); // 根据ID删除视频
    bool findVideoById(int id); // 根据ID查找视频
    std::string getVideoPathById(int id); // 根据ID获取视频路径
    std::vector<VIDEO_ITEM> getVideosByUserId(int userid, int limit = 15); // 根据用户ID获取视频列表
    std::vector<VIDEO_ITEM> getVideosByUserIdAndPage(int userid, int page, const std::string& date);
    VideoInfo getVideoInfoByUserIdAndFilename(int userid, const std::string& filename);
    void updateCurrentFrameByUserIdAndFilename(int userid, const std::string& filename, int currentFrame);
private:
    std::string databaseName;
};

#endif 