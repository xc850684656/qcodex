#include "mtranscode.h"

#include <libavutil/avutil.h>

struct MTranscodeData
{
    ProgInitFunc initFunc;
    ProgStepFunc stepFunc;
    ProgFinishFunc finishFunc;
};

MTranscode::MTranscode()
{
    m_data = new MTranscodeData();
}

MTranscode::~MTranscode()
{
    delete m_data;
}

void MTranscode::setProgressHandler(ProgInitFunc initFunc, ProgStepFunc stepFunc, ProgFinishFunc finishFunc)
{
    m_data->initFunc = initFunc;
    m_data->stepFunc = stepFunc;
    m_data->finishFunc = finishFunc;
}
