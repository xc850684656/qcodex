#ifndef MTRANSCODE_H
#define MTRANSCODE_H
#include "moption.h"

#include <functional>

typedef std::function<void(int,int,int)> ProgInitFunc; // 进度描述（int-最小值, int-最大值, int-步进值）
typedef std::function<void(void)> ProgStepFunc; // 进度推进一个步进单位
typedef std::function<void(int)> ProgFinishFunc; // 完成事件(int-退出码id)

struct MTranscodeData;
class MTRANSCODE_EXPORT MTranscode
{
private:
    MOption m_option;
    MTranscodeData* m_data = nullptr;
public:
    MTranscode();
    ~MTranscode();

    // 参数配置
    MOption& option() { return m_option; }
    const MOption& option() const { return m_option; }

    // 事件回调
    void setProgressHandler(ProgInitFunc initFunc, ProgStepFunc stepFunc, ProgFinishFunc finishFunc);

    // 执行转码

    // 中断/退出
};

#endif // MTRANSCODE_H
