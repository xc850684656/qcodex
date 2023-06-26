#include "mtranscode.h"
#include "mtranscodeinternal.h"

extern "C" {
static void progress_init(void* fp, int from, int to, int step)
{
    ProgInitFunc* p = (ProgInitFunc*)fp;
    (*p)(from, to, step);
}
}

MTranscode::MTranscode()
{
    m_context = createInternalContext();
}

MTranscode::~MTranscode()
{
    releaseInternalContext(&m_context);
}

void MTranscode::setProgressHandler(ProgInitFunc initFunc, ProgStepFunc stepFunc, ProgFinishFunc finishFunc)
{
    m_initFunc = initFunc;
    m_stepFunc = stepFunc;
    m_finishFunc = finishFunc;

    TranscodeCbf cbf = {0};
    cbf.progress_init = progress_init;
    setTranscodeCbf(m_context, &cbf);
}

void MTranscode::run()
{
    doTranscode(m_context);
}

void MTranscode::cancel()
{

}
