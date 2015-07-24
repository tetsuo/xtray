#include "next.h"

void next (callback_t callback, const void* ctx) {
  callback(ctx);
}
