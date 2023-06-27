#ifndef MTRANSCODE_H
#define MTRANSCODE_H
#include "mTranscode_global.h"
#include "../option/moption.h"

#include <functional>

typedef std::function<void(int,int,int)> ProgInitFunc; // 进度描述（int-最小值, int-最大值, int-步进值）
typedef std::function<void(void)> ProgStepFunc; // 进度推进一个步进单位
typedef std::function<void(int)> ProgFinishFunc; // 完成事件(int-退出码id)

class MTRANSCODE_EXPORT MTranscode
{
private:
    void* m_context = nullptr;
    ProgInitFunc m_initFunc;
    ProgStepFunc m_stepFunc;
    ProgFinishFunc m_finishFunc;
public:
    MTranscode();
    ~MTranscode();

    // 参数配置
    MOption* option();

    // 事件回调
    void setProgressHandler(ProgInitFunc initFunc, ProgStepFunc stepFunc, ProgFinishFunc finishFunc);

    // 执行转码
    void run();

    // 中断/退出
    void cancel();
};

#endif // MTRANSCODE_H
