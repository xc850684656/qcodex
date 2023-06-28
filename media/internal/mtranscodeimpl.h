#ifndef MTRANSCODEIMPL_H
#define MTRANSCODEIMPL_H

#include <mtranscode.h>

#include <atomic>

class MTranscodeImpl : public MTranscode
{
public:
    MTranscodeImpl(const MOption& opt, const MTranscodeFuncs& funcs);
    virtual ~MTranscodeImpl();
    void run() override;
    void cancel() override;
private:
    bool parseOption();
    void initDynload();

    struct BenchmarkTimeStamps {
        int64_t real_usec;
        int64_t user_usec;
        int64_t sys_usec;
    };
    BenchmarkTimeStamps get_benchmark_time_stamps(void);
    int transcode(int *err_rate_exceeded);
private:
    MOption m_opt;
    MTranscodeFuncs m_funcs;
    BenchmarkTimeStamps current_time;
    std::atomic<bool> transcode_init_done = false;
    int stdin_interaction = 1;
};

#endif // MTRANSCODEIMPL_H
