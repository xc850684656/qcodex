#include "mtranscodeinternal.h"
#include "../internal/mutils.h"
#include "../internal/sync_queue.h"

#include <stdatomic.h>

#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>
#include <libavutil/threadmessage.h>
#include <libavutil/eval.h>

// deprecated features
#define FFMPEG_OPT_PSNR 1
#define FFMPEG_OPT_MAP_CHANNEL 1
#define FFMPEG_OPT_MAP_SYNC 1
#define FFMPEG_ROTATION_METADATA 1
#define FFMPEG_OPT_QPHIST 1
#define FFMPEG_OPT_ADRIFT_THRESHOLD 1

enum VideoSyncMethod {
    VSYNC_AUTO = -1,
    VSYNC_PASSTHROUGH,
    VSYNC_CFR,
    VSYNC_VFR,
    VSYNC_VSCFR,
    VSYNC_DROP,
};

#define MAX_STREAMS 1024    /* arbitrary sanity check value */

enum HWAccelID {
    HWACCEL_NONE = 0,
    HWACCEL_AUTO,
    HWACCEL_GENERIC,
};

typedef struct InputFilter {
    struct FilterGraph *graph;
    uint8_t            *name;
} InputFilter;

typedef struct OutputFilter {
    AVFilterContext     *filter;
    struct OutputStream *ost;
    struct FilterGraph  *graph;
    uint8_t             *name;

    /* for filters that are not yet bound to an output stream,
     * this stores the output linklabel, if any */
    uint8_t             *linklabel;

    enum AVMediaType     type;

    /* desired output stream properties */
    int width, height;
    int format;
    int sample_rate;
    AVChannelLayout ch_layout;

    // those are only set if no format is specified and the encoder gives us multiple options
    // They point directly to the relevant lists of the encoder.
    const int *formats;
    const AVChannelLayout *ch_layouts;
    const int *sample_rates;

    /* pts of the last frame received from this filter, in AV_TIME_BASE_Q */
    int64_t last_pts;
} OutputFilter;

typedef struct FilterGraph {
    const AVClass *class;
    int            index;

    AVFilterGraph *graph;

    InputFilter   **inputs;
    int          nb_inputs;
    OutputFilter **outputs;
    int         nb_outputs;
} FilterGraph;

typedef struct Decoder Decoder;

typedef struct InputStream {
    const AVClass *class;

    int file_index;
    int index;

    AVStream *st;
    int discard;             /* true if stream data should be discarded */
    int user_set_discard;
    int decoding_needed;     /* non zero if the packets must be decoded in 'raw_fifo', see DECODING_FOR_* */
#define DECODING_FOR_OST    1
#define DECODING_FOR_FILTER 2

    /**
     * Codec parameters - to be used by the decoding/streamcopy code.
     * st->codecpar should not be accessed, because it may be modified
     * concurrently by the demuxing thread.
     */
    AVCodecParameters *par;
    Decoder *decoder;
    AVCodecContext *dec_ctx;
    const AVCodec *dec;
    const AVCodecDescriptor *codec_desc;

    AVRational framerate_guessed;

    int64_t nb_samples; /* number of samples in the last decoded audio frame before looping */

    AVDictionary *decoder_opts;
    AVRational framerate;               /* framerate forced with -r */
    int top_field_first;

    int autorotate;

    int fix_sub_duration;

    struct sub2video {
        int w, h;
    } sub2video;

    /* decoded data from this stream goes into all those filters
     * currently video and audio only */
    InputFilter **filters;
    int        nb_filters;

    /*
     * Output targets that do not go through lavfi, i.e. subtitles or
     * streamcopy. Those two cases are distinguished by the OutputStream
     * having an encoder or not.
     */
    struct OutputStream **outputs;
    int                nb_outputs;

    int reinit_filters;

    /* hwaccel options */
    enum HWAccelID hwaccel_id;
    enum AVHWDeviceType hwaccel_device_type;
    char  *hwaccel_device;
    enum AVPixelFormat hwaccel_output_format;

    /* stats */
    // number of frames/samples retrieved from the decoder
    uint64_t frames_decoded;
    uint64_t samples_decoded;
    uint64_t decode_errors;
} InputStream;

typedef struct InputFile {
    const AVClass *class;

    int index;

    // input format has no timestamps
    int format_nots;

    AVFormatContext *ctx;
    int eof_reached;      /* true if eof reached */
    int eagain;           /* true if last read attempt returned EAGAIN */
    int64_t input_ts_offset;
    int input_sync_ref;
    /**
     * Effective format start time based on enabled streams.
     */
    int64_t start_time_effective;
    int64_t ts_offset;
    int64_t start_time;   /* user-specified start time in AV_TIME_BASE or AV_NOPTS_VALUE */
    int64_t recording_time;

    /* streams that ffmpeg is aware of;
     * there may be extra streams in ctx that are not mapped to an InputStream
     * if new streams appear dynamically during demuxing */
    InputStream **streams;
    int        nb_streams;

    float readrate;
    int accurate_seek;

    /* when looping the input file, this queue is used by decoders to report
     * the last frame duration back to the demuxer thread */
    AVThreadMessageQueue *audio_duration_queue;
    int                   audio_duration_queue_size;
} InputFile;

enum forced_keyframes_const {
    FKF_N,
    FKF_N_FORCED,
    FKF_PREV_FORCED_N,
    FKF_PREV_FORCED_T,
    FKF_T,
    FKF_NB
};

#define ABORT_ON_FLAG_EMPTY_OUTPUT        (1 <<  0)
#define ABORT_ON_FLAG_EMPTY_OUTPUT_STREAM (1 <<  1)

enum EncStatsType {
    ENC_STATS_LITERAL = 0,
    ENC_STATS_FILE_IDX,
    ENC_STATS_STREAM_IDX,
    ENC_STATS_FRAME_NUM,
    ENC_STATS_FRAME_NUM_IN,
    ENC_STATS_TIMEBASE,
    ENC_STATS_TIMEBASE_IN,
    ENC_STATS_PTS,
    ENC_STATS_PTS_TIME,
    ENC_STATS_PTS_IN,
    ENC_STATS_PTS_TIME_IN,
    ENC_STATS_DTS,
    ENC_STATS_DTS_TIME,
    ENC_STATS_SAMPLE_NUM,
    ENC_STATS_NB_SAMPLES,
    ENC_STATS_PKT_SIZE,
    ENC_STATS_BITRATE,
    ENC_STATS_AVG_BITRATE,
};

typedef struct EncStatsComponent {
    enum EncStatsType type;

    uint8_t *str;
    size_t   str_len;
} EncStatsComponent;

typedef struct EncStats {
    EncStatsComponent  *components;
    int              nb_components;

    AVIOContext        *io;
} EncStats;

extern const char *const forced_keyframes_const_names[];

typedef enum {
    ENCODER_FINISHED = 1,
    MUXER_FINISHED = 2,
} OSTFinished ;

enum {
    KF_FORCE_SOURCE         = 1,
    KF_FORCE_SOURCE_NO_DROP = 2,
};

typedef struct KeyframeForceCtx {
    int          type;

    int64_t      ref_pts;

    // timestamps of the forced keyframes, in AV_TIME_BASE_Q
    int64_t     *pts;
    int       nb_pts;
    int          index;

    AVExpr      *pexpr;
    double       expr_const_values[FKF_NB];

    int          dropped_keyframe;
} KeyframeForceCtx;

typedef struct Encoder Encoder;

typedef struct OutputStream {
    const AVClass *class;

    enum AVMediaType type;

    int file_index;          /* file index */
    int index;               /* stream index in the output file */

    /**
     * Codec parameters for packets submitted to the muxer (i.e. before
     * bitstream filtering, if any).
     */
    AVCodecParameters *par_in;

    /* input stream that is the source for this output stream;
     * may be NULL for streams with no well-defined source, e.g.
     * attachments or outputs from complex filtergraphs */
    InputStream *ist;

    AVStream *st;            /* stream in the output file */
    /* dts of the last packet sent to the muxing queue, in AV_TIME_BASE_Q */
    int64_t last_mux_dts;

    // the timebase of the packets sent to the muxer
    AVRational mux_timebase;
    AVRational enc_timebase;

    Encoder *enc;
    AVCodecContext *enc_ctx;

    uint64_t nb_frames_dup;
    uint64_t nb_frames_drop;
    int64_t last_dropped;

    /* video only */
    AVRational frame_rate;
    AVRational max_frame_rate;
    enum VideoSyncMethod vsync_method;
    int is_cfr;
    int force_fps;
    int top_field_first;
#if FFMPEG_ROTATION_METADATA
    int rotate_overridden;
#endif
    int autoscale;
    int bitexact;
    int bits_per_raw_sample;
#if FFMPEG_ROTATION_METADATA
    double rotate_override_value;
#endif

    AVRational frame_aspect_ratio;

    KeyframeForceCtx kf;

    /* audio only */
#if FFMPEG_OPT_MAP_CHANNEL
    int *audio_channels_map;             /* list of the channels id to pick from the source stream */
    int audio_channels_mapped;           /* number of channels in audio_channels_map */
#endif

    char *logfile_prefix;
    FILE *logfile;

    OutputFilter *filter;

    AVDictionary *encoder_opts;
    AVDictionary *sws_dict;
    AVDictionary *swr_opts;
    char *apad;
    OSTFinished finished;        /* no more packets should be written for this stream */
    int unavailable;                     /* true if the steram is unavailable (possibly temporarily) */

    // init_output_stream() has been called for this stream
    // The encoder and the bitstream filters have been initialized and the stream
    // parameters are set in the AVStream.
    int initialized;

    int inputs_done;

    const char *attachment_filename;

    int keep_pix_fmt;

    /* stats */
    // number of packets send to the muxer
    atomic_uint_least64_t packets_written;
    // number of frames/samples sent to the encoder
    uint64_t frames_encoded;
    uint64_t samples_encoded;

    /* packet quality factor */
    int quality;

    int sq_idx_encode;
    int sq_idx_mux;

    EncStats enc_stats_pre;
    EncStats enc_stats_post;

    /*
     * bool on whether this stream should be utilized for splitting
     * subtitles utilizing fix_sub_duration at random access points.
     */
    unsigned int fix_sub_duration_heartbeat;
} OutputStream;

typedef struct OutputFile {
    const AVClass *class;

    int index;

    const AVOutputFormat *format;
    const char           *url;

    OutputStream **streams;
    int         nb_streams;

    SyncQueue *sq_encode;

    int64_t recording_time;  ///< desired length of the resulting file in microseconds == AV_TIME_BASE units
    int64_t start_time;      ///< start time in microseconds == AV_TIME_BASE units

    int shortest;
    int bitexact;
} OutputFile;

typedef struct TranscodeData {
    TranscodeCbf cbf;
    int is_interrupted;

    InputFile   **input_files;
    int        nb_input_files;

    OutputFile   **output_files;
    int         nb_output_files;
} TranscodeData;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ffmpeg_cleanup(TranscodeData* td)
{
    int i;

    if (do_benchmark) {
        int maxrss = getmaxrss() / 1024;
        av_log(NULL, AV_LOG_INFO, "bench: maxrss=%ikB\n", maxrss);
    }

    for (i = 0; i < nb_filtergraphs; i++)
        fg_free(&filtergraphs[i]);
    av_freep(&filtergraphs);

    /* close files */
    for (i = 0; i < nb_output_files; i++)
        of_close(&output_files[i]);

    for (i = 0; i < nb_input_files; i++)
        ifile_close(&input_files[i]);

    if (vstats_file) {
        if (fclose(vstats_file))
            av_log(NULL, AV_LOG_ERROR,
                   "Error closing vstats file, loss of information possible: %s\n",
                   av_err2str(AVERROR(errno)));
    }
    av_freep(&vstats_filename);
    of_enc_stats_close();

    hw_device_free_all();

    av_freep(&filter_nbthreads);

    av_freep(&input_files);
    av_freep(&output_files);

    uninit_opts();

    avformat_network_deinit();

    if (received_sigterm) {
        av_log(NULL, AV_LOG_INFO, "Exiting normally, received signal %d.\n",
               (int) received_sigterm);
    } else if (ret && atomic_load(&transcode_init_done)) {
        av_log(NULL, AV_LOG_INFO, "Conversion failed!\n");
    }
    term_exit();
    ffmpeg_exited = 1;
}

void* createInternalContext()
{
    return calloc(1, sizeof(TranscodeData));
}

void releaseInternalContext(void** cp)
{
    if (cp == NULL || *cp == NULL)
        return;
    free(*cp);
    *cp = NULL;
}

int doTranscode(void* context)
{
    TranscodeData* td = (TranscodeData *)context;

    initDynload();
    setvbuf(stderr,NULL,_IONBF,0); /* win32 runtime needs this */
    av_log_set_flags(AV_LOG_SKIP_REPEATED);

#if CONFIG_AVDEVICE
    avdevice_register_all();
#endif
    avformat_network_init();

    if (td->nb_output_files <= 0 && td->nb_input_files == 0) {
        //show_usage();
        //av_log(NULL, AV_LOG_WARNING, "Use -h to get full help or, even better, run 'man %s'\n", program_name);
        exit_program(1);
    }

    if (nb_output_files <= 0) {
        av_log(NULL, AV_LOG_FATAL, "At least one output file must be specified\n");
        exit_program(1);
    }

    current_time = ti = get_benchmark_time_stamps();
    ret = transcode(&err_rate_exceeded);
    if (ret >= 0 && do_benchmark) {
        int64_t utime, stime, rtime;
        current_time = get_benchmark_time_stamps();
        utime = current_time.user_usec - ti.user_usec;
        stime = current_time.sys_usec  - ti.sys_usec;
        rtime = current_time.real_usec - ti.real_usec;
        av_log(NULL, AV_LOG_INFO,
               "bench: utime=%0.3fs stime=%0.3fs rtime=%0.3fs\n",
               utime / 1000000.0, stime / 1000000.0, rtime / 1000000.0);
    }

    ret = received_nb_signals ? 255 :
              err_rate_exceeded   ?  69 : ret;

    exit_program(ret);
}

void cancelTranscode(void* context)
{
    TranscodeData* td = (TranscodeData *)context;
    td->is_interrupted = 1;
}

TranscodeCbf* getTranscodeCbf(void* context)
{
    TranscodeData* td = (TranscodeData *)context;
    return &td->cbf;
}
