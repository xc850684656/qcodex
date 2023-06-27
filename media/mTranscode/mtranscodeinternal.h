#ifndef MTRANSCODEINTERNAL_H
#define MTRANSCODEINTERNAL_H
#ifdef __cplusplus
extern "C" {
#endif

void* createInternalContext();
void releaseInternalContext(void** cp);
int doTranscode(void* context);
void cancelTranscode(void* context);

typedef void (*progress_cbf_init_t)(void*, int, int, int);
typedef void (*progress_cbf_step_t)(void*);

typedef struct TranscodeCbf {
    progress_cbf_init_t progress_init;
    void * progress_init_ctx;
    progress_cbf_step_t progress_step;
    void * progress_step_ctx;
} TranscodeCbf;
TranscodeCbf* getTranscodeCbf(void* context);

#ifdef __cplusplus
}
#endif

#endif // MTRANSCODEINTERNAL_H
