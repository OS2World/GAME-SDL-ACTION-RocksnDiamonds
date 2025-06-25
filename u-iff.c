#include "u-iff.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

uiff_ctx_t *uiff_new_ctx(FILE *f, uiff_ctx_t *ctx) {
  if(ctx == NULL) ctx = malloc(sizeof(uiff_ctx_t));
  if(ctx) {
    memset(ctx, 0, sizeof(uiff_ctx_t));
    ctx->f = f;
    //ctx->grbegin = 0;
    ctx->grend = LONG_MAX;
    ctx->grsz = INT32_MAX;
  }
  return ctx;
}


uiff_ctx_t *uiff_from_file(FILE *f, ID grID, ID subID) {
  uiff_ctx_t *ctx;

  ctx = calloc(sizeof(uiff_ctx_t), 1);
  if(ctx) {
    rewind(f);
    uiff_new_ctx(f, ctx);
    if(uiff_find_group_ctx(ctx, 0, grID, subID) >= 0) {
    } else {
      free(ctx);
      return NULL;
    }
  }
  return ctx;
}


int32_t uiff_close(uiff_ctx_t *ctx) {
  int ret;

  ret = fclose(ctx->f);
  free(ctx);
  if(ret != 0) return DOS_ERROR;
  return IFF_OKAY;
}


uint32_t read32(FILE *inf) {
  uint32_t u32 = 0;
  int c, i;

  for(i = 0; i < 4; ++i) {
    u32 <<= 8;
    c = fgetc(inf);
    //if(c == EOF) return SHORT_CHUNK;
    u32 |= c;
  }
  //if(u32 & 0x80808080) return -8;
  return u32;
}

uint16_t read16(FILE *inf) {
  uint16_t u16;
  int c;

  c = fgetc(inf);
  u16 = c << 8;
  c = fgetc(inf);
  u16 |= c;
  return u16;
}

int32_t uiff_find_chunk(FILE *inf, long length, ID ckID) {
  ID tmpid;
  int32_t size;
  long maxpos;

  maxpos = ftell(inf) + length;
  while(ftell(inf) < maxpos) {
    tmpid = read32(inf);
    if(feof(inf)) return SHORT_CHUNK;
    size = read32(inf);
    if(size < 0) return BAD_FORM;
    if(feof(inf)) return SHORT_CHUNK;
    if(ckID == 0 || tmpid == ckID) {
      return size;
    }
    fseek(inf, (size + 1) & ~1L, SEEK_CUR);
  }
  return NOT_IFF;
}

int32_t uiff_find_chunk_ctx(uiff_ctx_t *ctx, ID ckID) {
  int32_t size;
  long pos;

  pos = ftell(ctx->f);
  if(pos < ctx->grbgn || pos > ctx->grend) return CLIENT_ERROR; //fseek(ctx->f, ctx->grbgn, SEEK_SET);
  size = uiff_find_chunk(ctx->f, ctx->grsz, ckID);
  if(size >= 0) {
    pos = ftell(ctx->f);
    ctx->ckID = ckID;
    ctx->ckbgn = pos;
    ctx->ckend = pos + size;
    if(ctx->ckend > ctx->grend) {
      //Something went wrong! Chunk end is after group end.
      size = BAD_IFF;
    }
  }
  ctx->cksz = size;
  return size;
}


int32_t uiff_find_chunk_wflags(uiff_ctx_t *ctx, ID ckID, unsigned flags) {
  if(flags & IFF_FIND_REWIND) uiff_rewind_group(ctx);
  return uiff_find_chunk_ctx(ctx, ckID);
}


int32_t uiff_find_group(FILE *inf, long length, ID ckID, ID subID) {
  ID tmpid;
  int32_t size;
  long pos, maxpos;
  int found = 0;

  maxpos = ftell(inf) + length;
  while((pos = ftell(inf)) < maxpos) {
    size = uiff_find_chunk(inf, (maxpos - pos), ckID);
    if(size < 0) return size;
    if(size < 4) return BAD_FORM;
    if(ckID != 0) {
      found = 1;
    } else { //ckID = 0, find any group type
      fseek(inf, -8, SEEK_CUR); //Go back to ID
      tmpid = read32(inf);
      fseek(inf, 4, SEEK_CUR); //Skip size
      if(tmpid == FORM || tmpid == PROP || tmpid == LIST || tmpid == CAT) found = 1;
    }
    if(found) {
      tmpid = read32(inf);
      size -= 4; //Bytes left to read
      if(feof(inf)) return SHORT_CHUNK;
      if(tmpid == subID) {
	return size;
      } else found = 0;
    }
    fseek(inf, size, SEEK_CUR);
  }
  return NOT_IFF;
}

int32_t uiff_find_group_ctx(uiff_ctx_t *ctx, unsigned flags, ID ckID, ID subID) {
  int32_t size;
  long pos;

  if(flags & IFF_FIND_REWIND) uiff_rewind_group(ctx);
  pos = ftell(ctx->f);
  //Am I within a group?
  if(pos < ctx->grbgn || pos > ctx->grend) return CLIENT_ERROR; //fseek(ctx->f, ctx->grbgn, SEEK_SET);
  size = uiff_find_group(ctx->f, ctx->grsz, ckID, subID);
  if(size >= 0) {
    pos = ftell(ctx->f);
    ctx->grID = ckID;
    ctx->grsID = subID;
    ctx->grbgn = pos;
    ctx->grend = pos + size;
    ctx->grsz = size;
    ctx->ckID = 0; //We have no current chunk, set everything to zero/default.
    ctx->ckbgn = 0;
    ctx->ckend = 0;
    ctx->cksz = -1;
  }
  ctx->grsz = size;
  return size;
}

void *uiff_reada_chunk_ctx(uiff_ctx_t *ctx) {
  size_t bytes_read;
  void *ptr = NULL;
  long pos = ftell(ctx->f);

  if(pos >= ctx->grbgn && pos < ctx->grend && ctx->cksz > 0) { //We actually have to read something
    ptr = malloc(ctx->cksz);
    if(ptr) {
      bytes_read = fread(ptr, 1, ctx->cksz, ctx->f);
      if(bytes_read < ctx->cksz) {
	free(ptr);
	return NULL; // This is bad
      }
      ctx->last_data = ptr;
    }
  }
  return ptr;
}

int32_t uiff_skip(uiff_ctx_t *ctx) {
  long pos = ftell(ctx->f);

  if(pos < ctx->ckbgn || pos >= ctx->ckend) return CLIENT_ERROR;
  fseek(ctx->f, ctx->ckend, SEEK_SET);
  return (int32_t)(ctx->ckend - pos);
}

void uiff_rewind_group(uiff_ctx_t *ctx) {
  fseek(ctx->f, ctx->grbgn, SEEK_SET);
}
