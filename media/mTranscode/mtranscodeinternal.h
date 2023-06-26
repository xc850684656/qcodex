#ifndef MTRANSCODEINTERNAL_H
#define MTRANSCODEINTERNAL_H
#ifdef __cplusplus
extern "C" {
#endif

void* createInternalContext();
void releaseInternalContext(void** cp);
void doTranscode(void* context);
void cancelTranscode(void* context);

typedef void (*progress_cbf_init_t)(void* fp, int from, int to, int step);
typedef struct TranscodeCbf {
    progress_cbf_init_t progress_init;
} TranscodeCbf;
void setTranscodeCbf(void* context, const TranscodeCbf* cbf);

#ifdef __cplusplus
}
#endif

#endif // MTRANSCODEINTERNAL_H
