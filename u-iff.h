#ifndef __MICRO_IFF_H__
#define __MICRO_IFF_H__
#include <stdint.h>
#include <stdio.h>

typedef int32_t IFFP;	/* Status code result from an IFF procedure */
	/* LONG, because must be type compatable with ID for GetChunkHdr.*/
	/* Note that the error codes below are not legal IDs.*/
#define IFF_OKAY   0L	/* Keep going...*/
#define END_MARK  -1L	/* As if there was a chunk at end of group.*/
#define IFF_DONE  -2L	/* clientProc returns this when it has READ enough.
			 * It means return thru all levels. File is Okay.*/
#define DOS_ERROR -3L
#define NOT_IFF   -4L	/* not an IFF file.*/
#define NO_FILE   -5L	/* Tried to open file, DOS didn't find it.*/
#define CLIENT_ERROR -6L /* Client made invalid request, for instance, write
			 * a negative size chunk.*/
#define BAD_FORM  -7L	/* A client read proc complains about FORM semantics;
			 * e.g. valid IFF, but missing a required chunk.*/
#define SHORT_CHUNK -8L	/* Client asked to IFFReadBytes more bytes than left
			 * in the chunk. Could be client bug or bad form.*/
#define BAD_IFF   -9L	/* mal-formed IFF file. [TBD] Expand this into a
			 * range of error codes.*/
#define LAST_ERROR BAD_IFF

// Flags for calling the group find function
#define IFF_FIND_REWIND 1 //!< rewind before searching

/*! \brief Four-character IDentifier builder.*/
#define MakeID(a,b,c,d)  ( (int32_t)(a)<<24L | (int32_t)(b)<<16L | (c)<<8 | (d) )
/* Standard group IDs.  A chunk with one of these IDs contains a
   SubTypeID followed by zero or more chunks.*/
#define FORM MakeID('F','O','R','M')
#define PROP MakeID('P','R','O','P')
#define LIST MakeID('L','I','S','T')
#define CAT  MakeID('C','A','T',' ')
#define FILLER MakeID(' ',' ',' ',' ')
#define JUNK MakeID('J', 'U', 'N', 'K')

typedef int32_t ID;	/*! \brief An ID is four printable ASCII chars
			 *  but stored as a int32_t for efficient copy
			 *  & compare.*/

typedef struct Uiff_CTX {
  FILE *f;
  void *last_data; //!< last data was read here
  ID grID; //!<group id, eg FORM
  ID grsID; //!<group sub id, eg FORM ILBM
  long grbgn; //!<beginning position of group *data* in file
  long grend; //!<begin + grsz
  int32_t grsz; //!<group size

  ID ckID; //!<id of current chunk, eg BMHD
  long ckbgn; //!<beginning of chunk *data*
  long ckend;
  int32_t cksz; //!<chunk size of current chunk
} uiff_ctx_t;

/*! \brief find a chunk in a file
 * 
 * File is positioned on data. If ckID is zero then the next available
 * chunk is found.
 *
 * \param ckID id of the chunk to be found
 * \return chunk size
 */
int32_t uiff_find_chunk(FILE *inf, long length, ID ckID);

/*! \brief find a group in a file
 * 
 * File is positioned on data. If ckID is zero then any of the default
 * group chunks (FORM, LIST, CAT, PROP) is found.
 *
 * \param ckID id of the group chunk, e.g. FORM, etc.
 * \param subID subid of the group, e.g. ILBM, etc.
 * \return chunk size
 */
int32_t uiff_find_group(FILE *inf, long length, ID ckID, ID subID);

/*! \brief group predicate
 *
 */
int uiff_group_p(FILE *inf, ID grID, ID ckID);

/*! \brief read chunk to allocated memory
 *
 * Chunk is read within current context.
 */
void *uiff_reada_chunk_ctx(uiff_ctx_t *inf);

/*! \brief read a 32-bit big endian integer from file
 *
 * \param inf input file
 * \return unsigned integer 32 bit
 */
uint32_t read32(FILE *inf);

/*! \brief read a 16-bit big endian integer from file
 *
 * \param inf input file
 * \return unsigned integer 16 bit
 */
uint16_t read16(FILE *inf);

uiff_ctx_t *uiff_new_ctx(FILE *f, uiff_ctx_t *ctx);

/* \brief Get a context from a file only for true IFF files.
 *
 * This will rewind the file, check if it is an IFF file, and it will
 * return a context with the stream positioned into the main group.
 *
 * \param f a file which is rewound and checked
 * \param grID the group the file shall have, set to zero if any valid IFF file
 * \param subID Which type of IFF file (ILBM, etc.)? Set to zero if any.
 */
uiff_ctx_t *uiff_from_file(FILE *f, ID grID, ID subID);

/*! \brief close an iff context and underlying file
 *
 * \param ctx uiff context
 * \return IFF_OKAY on success
 */
int32_t uiff_close(uiff_ctx_t *ctx);

/*! \brief Find a group chunk
 *
 * If the ckID for the group is equal to zero any type of grouping
 * chunk is found.
 *
 * \param ctx iff context
 * \param flags currently only rewind
 * \param ckID id of the grouping chunk
 * \param subID sub id of the group
 * \return size of the group, is , is < 0 on error
 */
int32_t uiff_find_group_ctx(uiff_ctx_t *ctx, unsigned flags, ID ckID, ID subID);

/*! \brief find a chunk in a group
 *
 * If the ckID for the chunk is equal to zero any type of chunk is
 * found.
 *
 * \param ctx iff context
 * \param ckID id of the chunk
 * \return size of the group, <0 on error
 */
int32_t uiff_find_chunk_ctx(uiff_ctx_t *ctx, ID ckID);

/*! \brief find a chunk in a group with flags
 *
 * If the ckID for the chunk is equal to zero any type of chunk is
 * found.
 *
 * Currently only IFF_FIND_REWIND is supported.
 *
 * \param ctx iff context
 * \param ckID id of the chunk
 * \return size of the group, <0 on error
 */
int32_t uiff_find_chunk_wflags(uiff_ctx_t *ctx, ID ckID, unsigned flags);


/*! \brief skip to end of chunk
 *
 * Will skip to the end of the current chunk.
 *
 * \param ctx micro iff context
 * \return number of bytes skipped or error if < 0
 */
int32_t uiff_skip(uiff_ctx_t *ctx);

/*! \brief rewind to the beginning of the group
 */
void uiff_rewind_group(uiff_ctx_t *ctx);
#endif
