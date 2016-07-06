// ========================================================================
// ========================================================================
// Last updated on 1/7/08; 1/8/08
// ========================================================================

#include <stdint.h>
#include "ffmpeg/CodecContext.h"

#define CODEC_FLAG2_FAST          0x00000001 ///< Allow non spec compliant speedup tricks.
#define CODEC_FLAG_EMU_EDGE        0x4000   ///< Don't draw edges.

using namespace FFWrapper;

// Forward declarations

struct AVCodec;
struct AVFrame;

enum CodecID {
   CODEC_ID_NONE,
   CODEC_ID_MPEG1VIDEO,
   CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
   CODEC_ID_MPEG2VIDEO_XVMC,
   CODEC_ID_H261,
   CODEC_ID_H263,
   CODEC_ID_RV10,
   CODEC_ID_RV20,
   CODEC_ID_MJPEG,
   CODEC_ID_MJPEGB,
   CODEC_ID_LJPEG,
   CODEC_ID_SP5X,
   CODEC_ID_JPEGLS,
   CODEC_ID_MPEG4,
   CODEC_ID_RAWVIDEO,
   CODEC_ID_MSMPEG4V1,
   CODEC_ID_MSMPEG4V2,
   CODEC_ID_MSMPEG4V3,
   CODEC_ID_WMV1,
   CODEC_ID_WMV2,
   CODEC_ID_H263P,
   CODEC_ID_H263I,
   CODEC_ID_FLV1,
   CODEC_ID_SVQ1,
   CODEC_ID_SVQ3,
   CODEC_ID_DVVIDEO,
   CODEC_ID_HUFFYUV,
   CODEC_ID_CYUV,
   CODEC_ID_H264,
   CODEC_ID_INDEO3,
   CODEC_ID_VP3,
   CODEC_ID_THEORA,
   CODEC_ID_ASV1,
   CODEC_ID_ASV2,
   CODEC_ID_FFV1,
   CODEC_ID_4XM,
   CODEC_ID_VCR1,
   CODEC_ID_CLJR,
   CODEC_ID_MDEC,
   CODEC_ID_ROQ,
   CODEC_ID_INTERPLAY_VIDEO,
   CODEC_ID_XAN_WC3,
   CODEC_ID_XAN_WC4,
   CODEC_ID_RPZA,
   CODEC_ID_CINEPAK,
   CODEC_ID_WS_VQA,
   CODEC_ID_MSRLE,
   CODEC_ID_MSVIDEO1,
   CODEC_ID_IDCIN,
   CODEC_ID_8BPS,
   CODEC_ID_SMC,
   CODEC_ID_FLIC,
   CODEC_ID_TRUEMOTION1,
   CODEC_ID_VMDVIDEO,
   CODEC_ID_MSZH,
   CODEC_ID_ZLIB,
   CODEC_ID_QTRLE,
   CODEC_ID_SNOW,
   CODEC_ID_TSCC,
   CODEC_ID_ULTI,
   CODEC_ID_QDRAW,
   CODEC_ID_VIXL,
   CODEC_ID_QPEG,
   CODEC_ID_XVID,
   CODEC_ID_PNG,
   CODEC_ID_PPM,
   CODEC_ID_PBM,
   CODEC_ID_PGM,
   CODEC_ID_PGMYUV,
   CODEC_ID_PAM,
   CODEC_ID_FFVHUFF,
   CODEC_ID_RV30,
   CODEC_ID_RV40,
   CODEC_ID_VC1,
   CODEC_ID_WMV3,
   CODEC_ID_LOCO,
   CODEC_ID_WNV1,
   CODEC_ID_AASC,
   CODEC_ID_INDEO2,
   CODEC_ID_FRAPS,
   CODEC_ID_TRUEMOTION2,
   CODEC_ID_BMP,
   CODEC_ID_CSCD,
   CODEC_ID_MMVIDEO,
   CODEC_ID_ZMBV,
   CODEC_ID_AVS,
   CODEC_ID_SMACKVIDEO,
   CODEC_ID_NUV,
   CODEC_ID_KMVC,
   CODEC_ID_FLASHSV,
   CODEC_ID_CAVS,
   CODEC_ID_JPEG2000,
   CODEC_ID_VMNC,
   CODEC_ID_VP5,
   CODEC_ID_VP6,
   CODEC_ID_VP6F,
   CODEC_ID_TARGA,
   CODEC_ID_DSICINVIDEO,
   CODEC_ID_TIERTEXSEQVIDEO,
   CODEC_ID_TIFF,
   CODEC_ID_GIF,
   CODEC_ID_FFH264,
   CODEC_ID_DXA,
   CODEC_ID_DNXHD,
   CODEC_ID_THP,
   CODEC_ID_SGI,
   CODEC_ID_C93,
   CODEC_ID_BETHSOFTVID,
   CODEC_ID_PTX,
   CODEC_ID_TXD,
   CODEC_ID_VP6A,
   CODEC_ID_AMV,
   CODEC_ID_VB,
   CODEC_ID_PCX,
   CODEC_ID_SUNRAST,

   /* various PCM "codecs" */
   CODEC_ID_PCM_S16LE= 0x10000,
   CODEC_ID_PCM_S16BE,
   CODEC_ID_PCM_U16LE,
   CODEC_ID_PCM_U16BE,
   CODEC_ID_PCM_S8,
   CODEC_ID_PCM_U8,
   CODEC_ID_PCM_MULAW,
   CODEC_ID_PCM_ALAW,
   CODEC_ID_PCM_S32LE,
   CODEC_ID_PCM_S32BE,
   CODEC_ID_PCM_U32LE,
   CODEC_ID_PCM_U32BE,
   CODEC_ID_PCM_S24LE,
   CODEC_ID_PCM_S24BE,
   CODEC_ID_PCM_U24LE,
   CODEC_ID_PCM_U24BE,
   CODEC_ID_PCM_S24DAUD,
   CODEC_ID_PCM_ZORK,
   CODEC_ID_PCM_S16LE_PLANAR,

   /* various ADPCM codecs */
   CODEC_ID_ADPCM_IMA_QT= 0x11000,
   CODEC_ID_ADPCM_IMA_WAV,
   CODEC_ID_ADPCM_IMA_DK3,
   CODEC_ID_ADPCM_IMA_DK4,
   CODEC_ID_ADPCM_IMA_WS,
   CODEC_ID_ADPCM_IMA_SMJPEG,
   CODEC_ID_ADPCM_MS,
   CODEC_ID_ADPCM_4XM,
   CODEC_ID_ADPCM_XA,
   CODEC_ID_ADPCM_ADX,
   CODEC_ID_ADPCM_EA,
   CODEC_ID_ADPCM_G726,
   CODEC_ID_ADPCM_CT,
   CODEC_ID_ADPCM_SWF,
   CODEC_ID_ADPCM_YAMAHA,
   CODEC_ID_ADPCM_SBPRO_4,
   CODEC_ID_ADPCM_SBPRO_3,
   CODEC_ID_ADPCM_SBPRO_2,
   CODEC_ID_ADPCM_THP,
   CODEC_ID_ADPCM_IMA_AMV,
   CODEC_ID_ADPCM_EA_R1,
   CODEC_ID_ADPCM_EA_R3,
   CODEC_ID_ADPCM_EA_R2,
   CODEC_ID_ADPCM_IMA_EA_SEAD,
   CODEC_ID_ADPCM_IMA_EA_EACS,
   CODEC_ID_ADPCM_EA_XAS,

   /* AMR */
   CODEC_ID_AMR_NB= 0x12000,
   CODEC_ID_AMR_WB,

   /* RealAudio codecs*/
   CODEC_ID_RA_144= 0x13000,
   CODEC_ID_RA_288,

   /* various DPCM codecs */
   CODEC_ID_ROQ_DPCM= 0x14000,
   CODEC_ID_INTERPLAY_DPCM,
   CODEC_ID_XAN_DPCM,
   CODEC_ID_SOL_DPCM,

   CODEC_ID_MP2= 0x15000,
   CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
   CODEC_ID_AAC,
#if LIBAVCODEC_VERSION_INT < ((52<<16)+(0<<8)+0)
   CODEC_ID_MPEG4AAC,
#endif
   CODEC_ID_AC3,
   CODEC_ID_DTS,
   CODEC_ID_VORBIS,
   CODEC_ID_DVAUDIO,
   CODEC_ID_WMAV1,
   CODEC_ID_WMAV2,
   CODEC_ID_MACE3,
   CODEC_ID_MACE6,
   CODEC_ID_VMDAUDIO,
   CODEC_ID_SONIC,
   CODEC_ID_SONIC_LS,
   CODEC_ID_FLAC,
   CODEC_ID_MP3ADU,
   CODEC_ID_MP3ON4,
   CODEC_ID_SHORTEN,
   CODEC_ID_ALAC,
   CODEC_ID_WESTWOOD_SND1,
   CODEC_ID_GSM, ///< as in Berlin toast format
   CODEC_ID_QDM2,
   CODEC_ID_COOK,
   CODEC_ID_TRUESPEECH,
   CODEC_ID_TTA,
   CODEC_ID_SMACKAUDIO,
   CODEC_ID_QCELP,
   CODEC_ID_WAVPACK,
   CODEC_ID_DSICINAUDIO,
   CODEC_ID_IMC,
   CODEC_ID_MUSEPACK7,
   CODEC_ID_MLP,
   CODEC_ID_GSM_MS, /* as found in WAV */
   CODEC_ID_ATRAC3,
   CODEC_ID_VOXWARE,
   CODEC_ID_APE,
   CODEC_ID_NELLYMOSER,
   CODEC_ID_MUSEPACK8,

   /* subtitle codecs */
   CODEC_ID_DVD_SUBTITLE= 0x17000,
   CODEC_ID_DVB_SUBTITLE,
   CODEC_ID_TEXT,  ///< raw UTF-8 text
   CODEC_ID_XSUB,
   CODEC_ID_SSA,

   CODEC_ID_MPEG2TS= 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
    * stream (only used by libavformat) */
};

extern "C" 
{
   int avcodec_open(AVCodecContext* avctx, AVCodec* codec);
   int avcodec_decode_video(
      AVCodecContext *avctx, AVFrame *picture,
      int* got_picture_ptr,uint8_t* buf, int buf_size);
   AVCodec* avcodec_find_decoder(enum CodecID id);
   int avcodec_close(AVCodecContext *avctx);

   int get_AVCC_flags(const AVCodecContext* ic);
   void set_AVCC_flags(AVCodecContext* ic,int flags);
   int get_AVCC_flags2(const AVCodecContext* ic);
   void set_AVCC_flags2(AVCodecContext* ic,int flags2);

   enum CodecID get_codec_ID(AVCodecContext* avcc_ptr);
   void set_debug_mv(AVCodecContext* context_ptr,int dmv);
   void set_debug(AVCodecContext* context_ptr,int dbg);
   void set_workaround_bugs(AVCodecContext* context_ptr,int wab);
   int get_lowres(AVCodecContext* context_ptr);
   void set_idct_algo(AVCodecContext* context_ptr,int idct_algo);
   void set_skip_frame(AVCodecContext* context_ptr,enum AVDiscard skip_frame);
   void set_skip_idct(AVCodecContext* context_ptr,enum AVDiscard skip_idct);
   void set_skip_loop_filter(
      AVCodecContext* context_ptr,enum AVDiscard skip_loop_filter);
   void set_error_resilience(
      AVCodecContext* context_ptr,int error_resilience);
   void set_error_concealment(
      AVCodecContext* context_ptr,int error_concealment);
   void set_thread_count(AVCodecContext* context_ptr,int thread_count);
}

// ========================================================================

CodecContext::CodecContext()
   : m_Context(0)
{
}

CodecContext::~CodecContext()
{
   close();
}

int CodecContext::open_nothrow(
   AVCodecContext* p_Context,
   const CodecOptions& p_Options) throw()
{
   close();
    
   // Shamelessly ripped from ffplay.c stream_component_open()
   AVCodec* codec;

   codec = avcodec_find_decoder(get_codec_ID(p_Context));

   set_debug_mv(p_Context,p_Options.get_debug_mv());
   set_debug(p_Context,p_Options.get_debug());
   set_workaround_bugs(p_Context,p_Options.get_workaround_bugs());
   set_idct_algo(p_Context,p_Options.get_idct_algo());

//   int lowres;
   if(get_lowres(p_Context) == p_Options.get_lowres())
   {
      set_AVCC_flags(p_Context,get_AVCC_flags(p_Context) | 
                     CODEC_FLAG_EMU_EDGE);
   }
   if(p_Options.get_fast())
   {
      set_AVCC_flags2(p_Context,get_AVCC_flags2(p_Context) |
                      CODEC_FLAG_EMU_EDGE);
   }

   set_skip_frame(p_Context,p_Options.get_skip_frame());
   set_skip_idct(p_Context,p_Options.get_skip_idct());
   set_skip_loop_filter(p_Context,p_Options.get_skip_loop_filter());
   set_error_resilience(p_Context,p_Options.get_error_resilience());
   set_error_concealment(p_Context,p_Options.get_error_concealment());

   if(!codec || avcodec_open(p_Context, codec) < 0)
   {
      return -1;
   }

   int thread_count = p_Options.get_thread_count();
#ifdef HAVE_THREADS
   if(thread_count > 1)
   {
      avcodec_thread_init(p_Context, thread_count);
   }
#endif

   set_thread_count(p_Context,thread_count);
    
   m_Context = p_Context;

   return 0;
}

void CodecContext::close() throw()
{
   if (m_Context)
   {
      avcodec_close(m_Context);
      m_Context = 0;
   }
}

