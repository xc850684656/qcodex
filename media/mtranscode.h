#ifndef MTRANSCODE_H
#define MTRANSCODE_H
#include "mTranscode_global.h"
#include "option/moption.h"

#include <memory>
#include <functional>

typedef std::function<void(int,int,int)> ProgInitFunc; // 进度描述（int-最小值, int-最大值, int-步进值）
typedef std::function<void(void)> ProgStepFunc; // 进度推进一个步进单位
typedef std::function<void(int)> ProgFinishFunc; // 完成事件(int-退出码id)

struct MTranscodeFuncs
{
    ProgInitFunc progInit;
    ProgStepFunc progStep;
    ProgFinishFunc progFinish;
};

class MTRANSCODE_EXPORT MTranscode
{
private:
    ProgInitFunc m_initFunc;
    ProgStepFunc m_stepFunc;
    ProgFinishFunc m_finishFunc;
public:
    virtual ~MTranscode(){}

    static std::shared_ptr<MTranscode> make(const MOption& opt, const MTranscodeFuncs& funcs);

    // 执行转码
    virtual void run() = 0;

    // 中断/退出
    virtual void cancel() = 0;
};

#endif // MTRANSCODE_H
