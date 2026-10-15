//------------------------------------------------------------------------------
// File: JpuApi.h
//
// Copyright (c) 2006~2011, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef JPUAPI_H_INCLUDED
#define JPUAPI_H_INCLUDED

#include "jpuconfig.h"
#include "jdi.h"
#include "jputypes.h"

/* _n: number, _s: significance */
#define JPU_CEIL(_s, _n)        (((_n)+(_s-1))&~(_s-1))
#define JPU_FLOOR(_s, _n)       (_n&~(_s-1))
//------------------------------------------------------------------------------
// common struct and definition
//------------------------------------------------------------------------------

typedef enum {
    /* Non-differential, Huffman coding */
    JPEG_BASELINE_DCT,            /**< JPEG baseline profile */
    JPEG_EXTENDED_SEQUENTIAL_DCT, /**< JPEG extended sequential profile */
    /* The others are not supported on CODAJ12 */
} JpgProfile;

typedef enum {
    SET_JPG_SCALE_HOR, /**< This command sets the scale down ratio for picture width. The scale ratio value can be either 1(1/2 scale), 2(1/4 scale) or 3(1/8 scale). */
    SET_JPG_SCALE_VER, /**< This command sets the scale down ratio for picture height. The scale ratio value can be either 1(1/2 scale), 2(1/4 scale) or 3(1/8 scale). */
    SET_JPG_USE_STUFFING_BYTE_FF, /**< This command sets use of stuffing byte, 0xFF. */
    SET_JPG_QUALITY_FACTOR, /**< This command sets use of quality factor. */
    ENC_JPG_GET_HEADER,  /**< This command generates JPEG high-level header syntaxes. */
    ENABLE_LOGGING,      /**< This command activates message logging once JPU_DecOpen() or JPU_EncOpen() is called. */
    DISABLE_LOGGING,     /**< This command deactivates message logging. (default) */
    JPG_CMD_END
} JpgCommand;

typedef enum {
    JPG_RET_SUCCESS, /**< This means that operation was done successfully. */
    JPG_RET_FAILURE, /**< This means that operation was not done successfully. */
    JPG_RET_BIT_EMPTY, /**< This means that bitstream buffer is found to be empty during decode operation. */
    JPG_RET_EOS, /**< This means the end of stream. */
    JPG_RET_INVALID_HANDLE, /**< This means that the given handle for the current API function call was invalid(for example, not initialized yet, improper function call for the given handle, etc.).  */
    JPG_RET_INVALID_PARAM,  /**< This means that the given argument parameters (for example, input data structure) was invalid (not initialized yet or not valid anymore). */
    JPG_RET_INVALID_COMMAND, /**< This means that the given command was invalid (for example, undefined, or not allowed in the given instances). */
    JPG_RET_ROTATOR_OUTPUT_NOT_SET, /**< This means that rotator output buffer was not allocated even though postprocessor (rotation, mirroring, or deringing) is enabled.  */
    JPG_RET_ROTATOR_STRIDE_NOT_SET, /**< This means that rotator stride was not provided even though postprocessor (rotation, mirroring, or deringing) is enabled. */
    JPG_RET_FRAME_NOT_COMPLETE, /**< This means that frame decoding operation was not completed yet, so the given API function call cannot be allowed. */
    JPG_RET_INVALID_FRAME_BUFFER, /**< This means that the given source frame buffer pointers were invalid in encoder (not initialized yet or not valid anymore).  */
    JPG_RET_INSUFFICIENT_FRAME_BUFFERS, /**< This means that the given numbers of frame buffers were not enough for the operations of the given handle. */
    JPG_RET_INVALID_STRIDE, /**< This means that the given stride was invalid (for example, 0, not a multiple of 8 or smaller than picture size). */
    JPG_RET_WRONG_CALL_SEQUENCE, /**< This means that the current API function call was invalid considering the allowed sequences between API functions (for example, missing one crucial function call before  this function call). */
    JPG_RET_CALLED_BEFORE, /**< This means that multiple calls of the current API function for a given instance are invalid. */
    JPG_RET_NOT_INITIALIZED, /**< This means that JPU was not initialized yet. Before calling any API functions, the initialization API function, JPU_Init(), should be called at the beginning. */
    JPG_RET_INSUFFICIENT_RESOURCE, /**< This means that JPU cannot allocate memory due to lack of memory. */
    JPG_RET_INST_CTRL_ERROR, /**< This means there was a fault in instance control. */
    JPG_RET_NOT_SUPPORT, /**< This means that HOST application uses an API option that is not supported in current hardware. */
    JPG_RET_INSUFFICIENT_BITSTREAM_BUFFER, /**< This means that the size of bitstream buffer is not enough for the operations. */
} JpgRet;

typedef enum {
    ENCODE_STATE_NEW_FRAME = 0,  /**< The state indicates the start of a new frame. It appears for slice encoding. */
    ENCODE_STATE_FRAME_DONE = 0, /**< The state indicates the completion of encoding a frame. */
    ENCODE_STATE_SLICE_DONE = 1  /**< The state indicates the completion of encoding a slice. */
} EncodeState;

typedef enum {
    MIRDIR_NONE, /**< No mirroring */
    MIRDIR_VER,  /**< Vertical mirroring */
    MIRDIR_HOR,  /**< Horizontal mirroring */
    MIRDIR_HOR_VER /**< Horizontal and veritical mirroring */
} JpgMirrorDirection;

typedef enum {
    FORMAT_420 = 0, /**< YUV420 format */
    FORMAT_422 = 1, /**< YUV422 format */
    FORMAT_440 = 2, /**< YUV440 format */
    FORMAT_444 = 3, /**< YUV444 format */
    FORMAT_400 = 4, /**< YUV400 format */
    FORMAT_MAX
} FrameFormat;

typedef enum {
    CBCR_SEPARATED = 0, /**< Cb data and Cr data are located in each separate plane. */
    CBCR_INTERLEAVE, /**< Chroma data are interleaved in Cb and Cr order. */
    CRCB_INTERLEAVE  /**< Chroma data are interleaved in Cr and Cb order. */
} CbCrInterLeave;

typedef enum {
    PACKED_FORMAT_NONE,
    PACKED_FORMAT_422_YUYV,  /**< This is YUV422 packed format with YUYV component order. */
    PACKED_FORMAT_422_UYVY,  /**< This is YUV422 packed format with UYVY component order. */
    PACKED_FORMAT_422_YVYU,  /**< This is YUV422 packed format with YVYU component order. */
    PACKED_FORMAT_422_VYUY,  /**< This is YUV422 packed format with VYUY component order. */
    PACKED_FORMAT_444,       /**< This is YUV444 packed format.  */
    PACKED_FORMAT_MAX
} PackedFormat;

typedef enum {
    O_FMT_NONE,
    O_FMT_420,
    O_FMT_422,
    O_FMT_444,
    O_FMT_MAX
} OutputFormat;

/* Assume that pixel endianness is big-endian.
 * b0 is low address in a framebuffer.
 * b1 is high address in a framebuffer.
 * pixel consists of b0 and b1.
 * RIGHT JUSTIFICATION: (default)
 * lsb         msb
 * |----|--------|
 * |0000| pixel  |
 * |----|--------|
 * | b0   |   b1 |
 * |-------------|
 * LEFT JUSTIFICATION:
 * lsb         msb
 * |--------|----|
 * | pixel  |0000|
 * |--------|----|
 * | b0   |   b1 |
 * |-------------|
 */
enum JPU_PIXEL_JUSTIFICATION {
    PIXEL_16BIT_MSB_JUSTIFIED,
    PIXEL_16BIT_LSB_JUSTIFIED,
    PIXEL_16BIT_JUSTIFICATION_MAX,
};

typedef enum {
    INT_JPU_DONE = 0,  /**< This interrupt indicates the completion of encoding/decoding a frame. */
    INT_JPU_ERROR = 1, /**< This interrupt indicates occurrence of error. */
    INT_JPU_BIT_BUF_EMPTY = 2, /**< This interrupt indicates JPU encounters empty bitstream buffer while decoding. */
    INT_JPU_BIT_BUF_FULL = 2,  /**< This interrupt indicates full state of bitstream buffer while encoding. */     /* For encoder */
    INT_JPU_SLICE_DONE = 9,    /**< This interrupt indicates the completion of encoding a slice. */     /* For encoder */
}InterruptJpu;

typedef enum {
    JPG_TBL_NORMAL,
} JpgTableMode;

typedef enum {
    ENC_HEADER_MODE_NORMAL,  /**< Generation of the whole JPEG header syntaxes  */
    ENC_HEADER_MODE_SOS_ONLY /**< Generation of scan header syntax only  */
} JpgEncHeaderMode;

enum JPU_DOWNSCALE_VAL {
    JPG_SCALE_DOWN_NONE,     /**< No scaling */
    JPG_SCALE_DOWN_ONE_HALF, /**< 1/2 scale*/
    JPG_SCALE_DOWN_ONE_QUARTER, /**< 1/4 scale */
    JPG_SCALE_DOWN_ONE_EIGHTS,  /**< 1/8 scale */
    JPG_SCALE_DOWN_MAX
};

enum JPU_PRODUCT_ID {
    PRODUCT_ID_CODAJ12 = 12,
};

typedef struct {
    PhysicalAddress bufY;  /**< It indicates the base address for Y component in the physical address space when linear map is used. */
    PhysicalAddress bufCb; /**< It indicates the base address for Cb component in the physical address space when linear map is used. */
    PhysicalAddress bufCr; /**< It indicates the base address for Cr component in the physical address space when linear map is used. */
    Uint32          stride; /**< A horizontal stride for luma frame buffer */    /* luma stride */
    Uint32          strideC;/**< A horizontal stride for chroma frame buffer */    /* chroma stride */
    Uint32          fbLumaHeight; /**< A height for luma frame buffer */
    Uint32          fbChromaHeight;/**< A height for chroma frame buffer */
    FrameFormat     format; /**< <<jpuapi_h_OutputFormat>> */
    /**
    @verbatim
    Bitstream buffer endianess

    @* 0 : little endian format
    @* 1 : big endian format
    @* 2 : 32 bits little endian format
    @* 3 : 32 bits big endian format
    @endverbatim
    */
    Uint32          endian;
    Uint32          fbsize;  /* frame buffer size */
} FrameBuffer;

struct JpgInst;
typedef struct JpgInst *JpgHandle;

//------------------------------------------------------------------------------
// decode struct and definition
//------------------------------------------------------------------------------

typedef struct JpgInst JpgDecInst;
typedef JpgDecInst *JpgDecHandle;

/* JPU Capabilities */
typedef struct {
    Uint32  productId;     /**< Product ID number, 12 */
    Uint32  revisoin;      /**< Revision number of hardware */
    BOOL    support12bit;  /**< It indicates whether to support an extended sequential JPEG, 12bit. */
} JPUCap;

typedef struct {
    PhysicalAddress bitstreamBuffer; 		/**< The start address of bit stream buffer from which the decoder can get the next bitstream. This address must be 8-byte aligned.*/
    Uint32          bitstreamBufferSize;	/**< The virtual address according to bitstreamBuffer. This address is needed when the host wants to access bitstream.*/
    BYTE*           pBitStream;				/**< The size of the buffer pointed by bitstreamBuffer in byte. This value must be a multiple of 1024. The maximum size is 16383 x 1024 bytes.*/
    /**
    @verbatim
    Bitstream buffer endianess

    @* 0 : little endian format
    @* 1 : big endian format
    @* 2 : 32 bits little endian format
    @* 3 : 32 bits big endian format
    @endverbatim
    */
    Uint32          streamEndian;
    /**
    @verbatim
    Frame buffer endianess

    * 0 : little endian format
    * 1 : big endian format
    * 2 : 32 bits little endian format
    * 3 : 32 bits big endian format
    @endverbatim
    */
    Uint32          frameEndian;
    /**
    @verbatim
    @* 0: CbCr data is written in separate frame memories (chroma separate mode)
    @* 1: CbCr data is interleaved in chroma memory. (chroma interleave mode)
    @endverbatim
    */
    CbCrInterLeave  chromaInterleave;
    PackedFormat    packedFormat;			/**< This is for packed image format. It is valid only when source image format is written in YUV444 or RGB24. */
    BOOL            roiEnable;				/**< This enables the ROI (Region of Interest). */
    Uint32          roiOffsetX;				/**< This is ROI offset for width. */
    Uint32          roiOffsetY;				/**< This is ROI offset for height. */
    Uint32          roiWidth;				/**< This is ROI width from RoiOffsetX. */
    Uint32          roiHeight;				/**< This is ROI width from RoiOffsetY. */
    Uint32          pixelJustification;     /**< It specifies pixel justification of source image for 12bit encoding mode. */ /* avaliable in 16bit pixel. */
    Uint32          intrEnableBit;          /**< It specifies an interrupt enable bit. For the meaning of each bit, refer to <<jpuapi_h_InterruptJpu>>. */
    Uint32          rotation;              /**< It rotates decoded images in 0, 90, 180, or 270 degree */
    JpgMirrorDirection  mirror;            /**< <<jpuapi_h_JpgMirrorDirection>> */
    FrameFormat     outputFormat;          /**< <<jpuapi_h_OutputFormat>> */
} JpgDecOpenParam;

typedef struct {
    /**
    @verbatim
    Horizontal picture size in pixels

    This width value will be used while allocating decoder frame buffers. In some
    cases, this returned value, the display picture width declared on stream header,
    should be modified before allocating frame buffers. When picture width is not a
    multiple of 16, picture width for buffer allocation should be re-calculated from
    the declared display width as follows:

    picBufWidth = {( picWidth + 15)/16}* 16,

    where picBufWidth is the horizontal picture buffer width. When picWidth is a
    multiple of 16, picWidth = picBufWidth.
    */
    int         picWidth;
    /**
    @verbatim
    Vertical picture size in pixels

    This height value will be used while allocating decoder frame buffers. In some
    cases, this returned value, the display picture height declared on stream
    header, should be modified before allocating frame buffers. When picture height
    is not a multiple of 16, picture height for buffer allocation should be
    re-calculated from the declared display height as follows:

    picBufHeight = {( picHeight + 15)/16}* 16,

    where picBufHeight is the vertical picture buffer height. When picHeight is a
    multiple of 16, picHeight = picBufHeight.
    @endverbatim
    */
    int         picHeight;
    /**
    This is the minimum number of frame buffers required for decoding. Applications
    must allocate at least as many as this number of frame buffers and register the
    number of buffers to the JPU using JPU_DecRegisterFrameBuffer() before decoding
    pictures.
    */
    int         minFrameBufferCount;
    FrameFormat sourceFormat;  /**< <<jpuapi_h_FrameFormat>> */
    int         ecsPtr;        /**< This is an entropy-coded segment pointer of the first frame. */
    int         roiFrameWidth; /**< This is width of ROI region. */
    int         roiFrameHeight;/**< This is height of ROI region. */
    int         roiFrameOffsetX; /**< This is x position of ROI region. */
    int         roiFrameOffsetY; /**< This is y position of ROI region. */
    Uint32      bitDepth;  /**< 8 for baseline sequential, 12 for extended sequential */
} JpgDecInitialInfo;

typedef struct {
    int scaleDownRatioWidth;  /**< The ratio of frame width to scale down */
    /**
    @verbatim
    The ratio of frame height to scale down. It might have one of the following values:

    @* 0 : no scale
    @* 1 : (1/2 scale)
    @* 2 : (1/4 scale)
    @* 3 : (1/8 scale)
    @endverbatim
    */
    int scaleDownRatioHeight;
} JpgDecParam;

typedef struct {
    /**
    This is the frame buffer index of a picture to be displayed this time among
    frame buffers which were registered using JPU_DecRegisterFrameBuffer(). Frame
    data to be displayed this time will be stored into the frame buffer specified by
    this index. When delay in display does not exist, this index will always be the
    same with indexFrameDecoded.
    At the end of sequence decoding, if there is no more output for display, then
    this value will have -1 (0xFFFF). So by checking this index, host applications
    can easily know whether sequence decoding has been finished or not.
    */
    int         indexFrameDisplay;
    int         numOfErrMBs;        /**< The number of erroneous macroblocks while decoding a picture */
    /**
    @verbatim
    @* 0 : incomplete finish of decoding process
    @* 1 : complete finish of decoding process
    @endverbatim
    */
    int	        decodingSuccess;
    int         decPicHeight;       /**< The JPU reports the height of a decoded picture in pixel unit after decoding one frame. This value will be updated, when sequence information for resolution is changed. */
    int         decPicWidth;        /**< The JPU reports the width of a decoded picture in pixel unit after decoding one frame. This value will be updated, when sequence information for resolution is changed. */
    int         consumedByte;       /**< This means how many bytes are consumed in a bitstream. */
    int         bytePosFrameStart;  /**< This means the position of an SOI maker in bitstream buffer. */
    int         ecsPtr;             /**< This is a pointer to an entropy-coded segment of current frame. */
    Uint32      frameCycle;         /**< This variable reports the number of clock cycles taken to decode one frame. */ /* clock cycle */
    Uint32      rdPtr;              /**< The position of read pointer in the bitstream buffer after completion of frame decoding */
    Uint32      wrPtr;              /**< The position of write pointer in the bitstream buffer after completion of frame decoding */
} JpgDecOutputInfo;



//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------

typedef struct JpgInst JpgEncInst;
typedef JpgEncInst *JpgEncHandle;


typedef struct {
    PhysicalAddress bitstreamBuffer;     /**< The start address of bit stream buffer into which encoder puts bit streams. This address must be a multiple of 4, namely, 4 byte-aligned. */
    Uint32          bitstreamBufferSize; /**< The size of the buffer in bytes pointed by bitstreamBuffer. This value must be a multiple of 1024. The maximum size is 16383 x 1024 bytes. */
    Uint32          picWidth;            /**< The width of a picture to be encoded in pixels */
    Uint32          picHeight;           /**< The height of a picture to be encoded in pixels */
    FrameFormat     sourceFormat;        /**< <<jpuapi_h_FrameFormat>> */
    Uint32          restartInterval;     /**< The number of MCU in the restart interval */
    /**
    @verbatim
    Bitstream buffer endianess

    @* 0 : little endian format
    @* 1 : big endian format
    @* 2 : 32 bits little endian format
    @* 3 : 32 bits big endian format
    @endverbatim
    */
	Uint32          streamEndian;
    /**
    @verbatim
    Frame buffer endianess

    @* 0 : little endian format
    @* 1 : big endian format
    @* 2 : 32 bits little endian format
    @* 3 : 32 bits big endian format
    @endverbatim
    */
    Uint32          frameEndian;
    /**
    @verbatim
    @* 0 : CbCr data is written in separate frame memories (chroma separate mode)
    @* 1 : CbCr data is interleaved in chroma memory. (chroma interleave mode)
    @endverbatim
    */
    CbCrInterLeave  chromaInterleave;
    BYTE            huffVal[8][256];     /**< A list of the 8-bit symbol values in Huffman tables. */
    BYTE            huffBits[8][256];    /**< A 16-byte list giving number of codes for each code length from 1 to 16 in Huffman tables. */
    short           qMatTab[4][64];      /**< Quantization tables */
    BOOL            jpg12bit;            /**< It enables 12bit encoding mode. */
    BOOL            q_prec0;             /**< Luma quantization tables for 12bit encoding mode */
    BOOL            q_prec1;             /**< Chroma quantization tables for 12bit encoding mode */
    PackedFormat    packedFormat;        /**< It specifies a packed image format. It is valid only when source image format is written in YUV444 or RGB24. */
    Uint32          pixelJustification;  /**< It specifies pixel justification of source image for 12bit encoding mode. */
    Uint32          tiledModeEnable;     /**< It enables use of tiled mode. */
    Uint32          sliceHeight;         /**< It specifies a slice height for slice encoding. */
    Uint32          intrEnableBit;       /**< It specifies an interrupt enable bit. For the meaning of each bit, refer to <<jpuapi_h_InterruptJpu>>. */
    BOOL            sliceInstMode;
    Uint32          rotation;            /**< It rotates source images in 0, 90, 180, or 270 degree before encoding. */
    Uint32          mirror;              /**< <<jpuapi_h_JpgMirrorDirection>> */
} JpgEncOpenParam;



typedef struct {
    FrameBuffer * sourceFrame; /**< This member must represent the frame buffer containing source image to be encoded. */
} JpgEncParam;


typedef struct {
    /**
    The Physical address of the starting point of newly encoded picture stream
    If dynamic buffer allocation is enabled in line-buffer mode, this value will be
    identical with the specified picture stream buffer address by host applications.
    */
    PhysicalAddress bitstreamBuffer;
    Uint32 bitstreamSize;            /**< The byte size of encoded bitstream */
    PhysicalAddress streamRdPtr;     /**< A read pointer of bitstream buffer */
    PhysicalAddress streamWrPtr;     /**< A write pointer of bitstream buffer */
    Uint32 encodedSliceYPos;         /**< A vertical position of the currently encoded slice */
    EncodeState encodeState;         /**< A result of the current encoding, completion of encoding the whole picture or completion of encoding a specific slice */
    Uint32  frameCycle;              /**< This variable reports the number of clock cycles taken to decode one frame. */ /* clock cycle */
} JpgEncOutputInfo;

typedef struct {
    PhysicalAddress paraSet;   /**< The physical address of bitstream buffer where header bitstream is saved  */
    BYTE *pParaSet;            /**< The virtual address of bitstream buffer where header bitstream is saved  */
    int size;                  /**< The allocation size of buffer for saving header bitstream */
    /**
    @verbatim
    Header encoding mode

    @* ENC_HEADER_MODE_NORMAL: generates all sorts of headers.
    @* ENC_HEADER_MODE_SOS_ONLY: generates only SOS header (not any other such as SOI, APP, DRI, DQT, DHT, and SOF headers).
    @endverbatim
    */
    int headerMode;
    /**
    @verbatim
    Quantization table merge option

    @* 0: do not merge quantization table
    @* 1: merge quantization table
    @endverbatim
    */
    int quantMode;
    /**
    @verbatim
    Huffman table merge option

    @* 0: do not merge huffman table
    @* 1: merge huffman table
    @endverbatim
    */
    int huffMode;
    int disableAPPMarker;      /**< It disables generating an App Marker in encoding header. */
    int enableSofStuffing;     /**< It enables to fill 0xff at the end of header if it is not 8-byte aligned after encoding the header. (defalut: 0x00) */
} JpgEncParamSet;



#ifdef __cplusplus
extern "C" {
#endif


Uint32      JPU_IsInit(void);
Int32       JPU_InterruptSortOut(Int32* instIndex, Uint32* reason);
Int32       JPU_WaitInterrupt(JpgHandle handle, int timeout);
int         JPU_IsBusy(JpgHandle handle);
Uint32      JPU_GetStatus(JpgHandle handle);
void        JPU_ClrStatus(JpgHandle handle, Uint32 val);
void        JPU_ClrStatusEx(Int32 instIdx, Uint32 val);
void        JPU_DisableInterrput(void);
void        JPU_EnableInterrput(Uint32 val);

JpgRet      JPU_Init(unsigned long phy_base, unsigned int reg_range);
JpgRet      JPU_DeInit(void);
int         JPU_GetOpenInstanceNum(void);
JpgRet      JPU_GetVersionInfo(
    Uint32 *apiVersion,
    Uint32 *hwRevision,
    Uint32 *hwProductId);

// function for decode
JpgRet JPU_DecOpen(JpgDecHandle *, JpgDecOpenParam *);
JpgRet JPU_DecClose(JpgDecHandle);
JpgRet JPU_DecGetInitialInfo(
    JpgDecHandle handle,
    JpgDecInitialInfo *info);
JpgRet JPU_DecRegisterFrameBuffer(
    JpgDecHandle handle,
    FrameBuffer *bufArray,
    int num,
    int stride);
JpgRet JPU_DecGetBitstreamBuffer(
    JpgDecHandle handle,
    PhysicalAddress *prdPrt,
    PhysicalAddress *pwrPtr,
    int *size );
JpgRet JPU_DecUpdateBitstreamBuffer(
    JpgDecHandle handle,
    int size);
JpgRet JPU_HWReset(void);
JpgRet JPU_SWReset(JpgHandle handle);
JpgRet JPU_DecStartOneFrame(
    JpgDecHandle handle,
    JpgDecParam *param );
JpgRet JPU_DecGetOutputInfo(
    JpgDecHandle handle,
    JpgDecOutputInfo *info);
JpgRet JPU_DecGiveCommand(
    JpgDecHandle handle,
    JpgCommand cmd,
    void *parameter);
JpgRet JPU_DecSetRdPtr(
    JpgDecHandle handle,
    PhysicalAddress addr,
    BOOL updateWrPtr);
JpgRet JPU_DecSetRdPtrEx(
    JpgDecHandle handle,
    PhysicalAddress addr,
    BOOL updateWrPtr);

// function for encode
JpgRet JPU_EncOpen(JpgEncHandle *, JpgEncOpenParam *);
JpgRet JPU_EncClose(JpgEncHandle);
JpgRet JPU_EncGetBitstreamBuffer(
    JpgEncHandle handle,
    PhysicalAddress *prdPrt,
    PhysicalAddress *pwrPtr,
    int *size);
JpgRet JPU_EncUpdateBitstreamBuffer(
    JpgEncHandle handle,
    int size);
JpgRet JPU_EncStartOneFrame(
    JpgEncHandle handle,
    JpgEncParam *param );
JpgRet JPU_EncGetOutputInfo(
    JpgEncHandle handle,
    JpgEncOutputInfo *info);
JpgRet JPU_DecGetOutputInfoEx(
    JpgDecHandle handle,
    JpgDecOutputInfo *info,
    Int32 pic_status);
JpgRet JPU_EncGiveCommand(
    JpgEncHandle handle,
    JpgCommand cmd,
    void *parameter);

JpgRet JPU_EncGetOutputInfoEx(
    JpgEncHandle handle,
    JpgEncOutputInfo *info,
    int reason
);


#ifdef __cplusplus
}
#endif

#endif
