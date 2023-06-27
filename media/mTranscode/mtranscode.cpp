#include "mtranscode.h"
#include "mtranscodeinternal.h"

extern "C" {
static void progress_init(void* fp, int from, int to, int step)
{
    ProgInitFunc* p = (ProgInitFunc*)fp;
    (*p)(from, to, step);
}
static void progress_step(void* fp)
{
    ProgStepFunc* p = (ProgStepFunc*)fp;
    (*p)();
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

    TranscodeCbf *cbf = getTranscodeCbf(m_context);
    cbf->progress_init = progress_init;
    cbf->progress_init_ctx = &m_initFunc;
    cbf->progress_step = progress_step;
    cbf->progress_step_ctx = &m_stepFunc;
}

void MTranscode::run()
{
    int r = doTranscode(m_context);
    if (m_finishFunc)
        m_finishFunc(r);
}

void MTranscode::cancel()
{
    cancelTranscode(m_context);
}
