#include "mtranscodeimpl.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
}

//////////////////////////////////////////////////////////////////////////////////
// structs
struct InputStream
{

};

//////////////////////////////////////////////////////////////////////////////////
MTranscodeImpl::MTranscodeImpl(const MOption& opt, const MTranscodeFuncs& funcs)
    : m_opt(opt), m_funcs(funcs)
{

}

MTranscodeImpl::~MTranscodeImpl()
{

}

void MTranscodeImpl::run()
{
    initDynload();

    //avdevice_register_all();
    avformat_network_init();

    if (!parseOption())
        return;
    int ret, err_rate_exceeded;
    BenchmarkTimeStamps ti;

    current_time = ti = get_benchmark_time_stamps();
    ret = transcode(&err_rate_exceeded);
    if (ret >= 0 && m_opt.global.do_benchmark) {
        int64_t utime, stime, rtime;
        current_time = get_benchmark_time_stamps();
        utime = current_time.user_usec - ti.user_usec;
        stime = current_time.sys_usec  - ti.sys_usec;
        rtime = current_time.real_usec - ti.real_usec;
        av_log(NULL, AV_LOG_INFO,
               "bench: utime=%0.3fs stime=%0.3fs rtime=%0.3fs\n",
               utime / 1000000.0, stime / 1000000.0, rtime / 1000000.0);
    }
}

void MTranscodeImpl::cancel()
{

}

bool MTranscodeImpl::parseOption()
{
    return true;
}

void MTranscodeImpl::initDynload()
{
#if HAVE_SETDLLDIRECTORY && defined(_WIN32)
    /* Calling SetDllDirectory with the empty string (but not NULL) removes the
     * current working directory from the DLL search path as a security pre-caution. */
    SetDllDirectory("");
#endif
}

MTranscodeImpl::BenchmarkTimeStamps MTranscodeImpl::get_benchmark_time_stamps()
{
    BenchmarkTimeStamps time_stamps = { av_gettime_relative() };
#if HAVE_GETRUSAGE
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);
    time_stamps.user_usec =
        (rusage.ru_utime.tv_sec * 1000000LL) + rusage.ru_utime.tv_usec;
    time_stamps.sys_usec =
        (rusage.ru_stime.tv_sec * 1000000LL) + rusage.ru_stime.tv_usec;
#elif HAVE_GETPROCESSTIMES
    HANDLE proc;
    FILETIME c, e, k, u;
    proc = GetCurrentProcess();
    GetProcessTimes(proc, &c, &e, &k, &u);
    time_stamps.user_usec =
        ((int64_t)u.dwHighDateTime << 32 | u.dwLowDateTime) / 10;
    time_stamps.sys_usec =
        ((int64_t)k.dwHighDateTime << 32 | k.dwLowDateTime) / 10;
#else
    time_stamps.user_usec = time_stamps.sys_usec = 0;
#endif
    return time_stamps;
}

int MTranscodeImpl::transcode(int *err_rate_exceeded)
{
    int ret = 0, i;
    InputStream *ist;
    int64_t timer_start;
    //print_stream_maps();

    *err_rate_exceeded = 0;
    atomic_store(&transcode_init_done, true);

    if (stdin_interaction) {
        av_log(NULL, AV_LOG_INFO, "Press [q] to stop, [?] for help\n");
    }

    timer_start = av_gettime_relative();

    return ret;
}
