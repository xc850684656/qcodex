#include "mtranscode.h"
#include "internal/mtranscodeimpl.h"

std::shared_ptr<MTranscode> MTranscode::make(const MOption &opt, const MTranscodeFuncs &funcs)
{
    return std::make_shared<MTranscodeImpl>(opt, funcs);
}
