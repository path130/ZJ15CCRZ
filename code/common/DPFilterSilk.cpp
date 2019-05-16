#include "DPFilterSilk.h"

#define SKP_int         int                     /* used for counters etc; at least 16 bits */
#define SKP_int64       long long
#define SKP_int32       int
#define SKP_int16       short
#define SKP_int8        signed char

#define SKP_uint        unsigned int            /* used for counters etc; at least 16 bits */
#define SKP_uint64      unsigned long long
#define SKP_uint32      unsigned int
#define SKP_uint16      unsigned short
#define SKP_uint8       unsigned char

#define SKP_bool        unsigned char

/******************/
/* Error messages */
/******************/
#define SKP_SILK_NO_ERROR                               0

/**************************/
/* Encoder error messages */
/**************************/

/* Input length is not a multiplum of 10 ms, or length is longer than the packet length */
#define SKP_SILK_ENC_INPUT_INVALID_NO_OF_SAMPLES        -1

/* Sampling frequency not 8000, 12000, 16000 or 24000 Hertz */
#define SKP_SILK_ENC_FS_NOT_SUPPORTED                   -2

/* Packet size not 20, 40, 60, 80 or 100 ms */
#define SKP_SILK_ENC_PACKET_SIZE_NOT_SUPPORTED          -3

/* Allocated payload buffer too short */
#define SKP_SILK_ENC_PAYLOAD_BUF_TOO_SHORT              -4

/* Loss rate not between 0 and 100 percent */
#define SKP_SILK_ENC_INVALID_LOSS_RATE                  -5

/* Complexity setting not valid, use 0, 1 or 2 */
#define SKP_SILK_ENC_INVALID_COMPLEXITY_SETTING         -6

/* Inband FEC setting not valid, use 0 or 1 */
#define SKP_SILK_ENC_INVALID_INBAND_FEC_SETTING         -7

/* DTX setting not valid, use 0 or 1 */
#define SKP_SILK_ENC_INVALID_DTX_SETTING                -8

/* Internal encoder error */
#define SKP_SILK_ENC_INTERNAL_ERROR                     -9

/**************************/
/* Decoder error messages */
/**************************/

/* Output sampling frequency lower than internal decoded sampling frequency */
#define SKP_SILK_DEC_INVALID_SAMPLING_FREQUENCY         -10

/* Payload size exceeded the maximum allowed 1024 bytes */
#define SKP_SILK_DEC_PAYLOAD_TOO_LARGE                  -11

/* Payload has bit errors */
#define SKP_SILK_DEC_PAYLOAD_ERROR                      -12






/***********************************************/
/* Structure for controlling encoder operation */
/***********************************************/
typedef struct {
	/* I:   Input signal sampling rate in Hertz; 8000/12000/16000/24000                     */
	SKP_int32 API_sampleRate;

	/* I:   Maximum internal sampling rate in Hertz; 8000/12000/16000/24000                 */
	SKP_int32 maxInternalSampleRate;

	/* I:   Number of samples per packet; must be equivalent of 20, 40, 60, 80 or 100 ms    */
	SKP_int packetSize;

	/* I:   Bitrate during active speech in bits/second; internally limited                 */
	SKP_int32 bitRate;                        

	/* I:   Uplink packet loss in percent (0-100)                                           */
	SKP_int packetLossPercentage;

	/* I:   Complexity mode; 0 is lowest; 1 is medium and 2 is highest complexity           */
	SKP_int complexity;

	/* I:   Flag to enable in-band Forward Error Correction (FEC); 0/1                      */
	SKP_int useInBandFEC;

	/* I:   Flag to enable discontinuous transmission (DTX); 0/1                            */
	SKP_int useDTX;
} SKP_SILK_SDK_EncControlStruct;

/**************************************************************************/
/* Structure for controlling decoder operation and reading decoder status */
/**************************************************************************/
typedef struct {
	/* I:   Output signal sampling rate in Hertz; 8000/12000/16000/24000                    */
	SKP_int32 API_sampleRate;

	/* O:   Number of samples per frame                                                     */
	SKP_int frameSize;

	/* O:   Frames per packet 1, 2, 3, 4, 5                                                 */
	SKP_int framesPerPacket;

	/* O:   Flag to indicate that the decoder has remaining payloads internally             */
	SKP_int moreInternalDecoderFrames;

	/* O:   Distance between main payload and redundant payload in packets                  */
	SKP_int inBandFECOffset;
} SKP_SILK_SDK_DecControlStruct;









#ifdef __cplusplus
extern "C"
{
#endif

#define SILK_MAX_FRAMES_PER_PACKET  5

	/* Struct for TOC (Table of Contents) */
	typedef struct {
		SKP_int     framesInPacket;                             /* Number of 20 ms frames in packet     */
		SKP_int     fs_kHz;                                     /* Sampling frequency in packet         */
		SKP_int     inbandLBRR;                                 /* Does packet contain LBRR information */
		SKP_int     corrupt;                                    /* Packet is corrupt                    */
		SKP_int     vadFlags[     SILK_MAX_FRAMES_PER_PACKET ]; /* VAD flag for each frame in packet    */
		SKP_int     sigtypeFlags[ SILK_MAX_FRAMES_PER_PACKET ]; /* Signal type for each frame in packet */
	} SKP_Silk_TOC_struct;

	/****************************************/
	/* Encoder functions                    */
	/****************************************/

	/***********************************************/
	/* Get size in bytes of the Silk encoder state */
	/***********************************************/
	SKP_int SKP_Silk_SDK_Get_Encoder_Size( 
		SKP_int32                           *encSizeBytes   /* O:   Number of bytes in SILK encoder state           */
		);

	/*************************/
	/* Init or reset encoder */
	/*************************/
	SKP_int SKP_Silk_SDK_InitEncoder(
		void                                *encState,      /* I/O: State                                           */
		SKP_SILK_SDK_EncControlStruct       *encStatus      /* O:   Encoder Status                                  */
		);

	/***************************************/
	/* Read control structure from encoder */
	/***************************************/
	SKP_int SKP_Silk_SDK_QueryEncoder(
		const void                          *encState,      /* I:   State                                           */
		SKP_SILK_SDK_EncControlStruct       *encStatus      /* O:   Encoder Status                                  */
		);

	/**************************/
	/* Encode frame with Silk */
	/**************************/
	SKP_int SKP_Silk_SDK_Encode( 
		void                                *encState,      /* I/O: State                                           */
		const SKP_SILK_SDK_EncControlStruct *encControl,    /* I:   Control status                                  */
		const SKP_int16                     *samplesIn,     /* I:   Speech sample input vector                      */
		SKP_int                             nSamplesIn,     /* I:   Number of samples in input vector               */
		SKP_uint8                           *outData,       /* O:   Encoded output vector                           */
		SKP_int16                           *nBytesOut      /* I/O: Number of bytes in outData (input: Max bytes)   */
		);

	/****************************************/
	/* Decoder functions                    */
	/****************************************/

	/***********************************************/
	/* Get size in bytes of the Silk decoder state */
	/***********************************************/
	SKP_int SKP_Silk_SDK_Get_Decoder_Size( 
		SKP_int32                           *decSizeBytes   /* O:   Number of bytes in SILK decoder state           */
		);

	/*************************/
	/* Init or Reset decoder */
	/*************************/
	SKP_int SKP_Silk_SDK_InitDecoder( 
		void                                *decState       /* I/O: State                                           */
		);

	/******************/
	/* Decode a frame */
	/******************/
	SKP_int SKP_Silk_SDK_Decode(
		void*                               decState,       /* I/O: State                                           */
		SKP_SILK_SDK_DecControlStruct*      decControl,     /* I/O: Control Structure                               */
		SKP_int                             lostFlag,       /* I:   0: no loss, 1 loss                              */
		const SKP_uint8                     *inData,        /* I:   Encoded input vector                            */
		const SKP_int                       nBytesIn,       /* I:   Number of input bytes                           */
		SKP_int16                           *samplesOut,    /* O:   Decoded output speech vector                    */
		SKP_int16                           *nSamplesOut    /* I/O: Number of samples (vector/decoded)              */
		);

	/***************************************************************/
	/* Find Low Bit Rate Redundancy (LBRR) information in a packet */
	/***************************************************************/
	void SKP_Silk_SDK_search_for_LBRR(
		const SKP_uint8                     *inData,        /* I:   Encoded input vector                            */
		const SKP_int16                     nBytesIn,       /* I:   Number of input Bytes                           */
		SKP_int                             lost_offset,    /* I:   Offset from lost packet                         */
		SKP_uint8                           *LBRRData,      /* O:   LBRR payload                                    */
		SKP_int16                           *nLBRRBytes     /* O:   Number of LBRR Bytes                            */
		);

	/**************************************/
	/* Get table of contents for a packet */
	/**************************************/
	void SKP_Silk_SDK_get_TOC(
		const SKP_uint8                     *inData,        /* I:   Encoded input vector                            */
		const SKP_int16                     nBytesIn,       /* I:   Number of input bytes                           */
		SKP_Silk_TOC_struct                 *Silk_TOC       /* O:   Table of contents                               */
		);

	/**************************/
	/* Get the version number */
	/**************************/
	/* Return a pointer to string specifying the version */ 
	const char *SKP_Silk_SDK_get_version();

#ifdef __cplusplus
}
#endif


// Define codec specific settings
#define MAX_BYTES_PER_FRAME     250 // Equals peak bitrate of 100 kbps 
#define MAX_INPUT_FRAMES        5
#define FRAME_LENGTH_MS         20
#define MAX_API_FS_KHZ          24//48

#define PRINTFERR    printf



typedef struct SilkDecState{
	int samplerate;
	SKP_uint64 sample_count;
	SKP_uint64 frame_sample;
	SKP_uint16 sync_seq;
	int plc_count;
	SKP_bool plc;

	SKP_int16 pcmbuff[FRAME_LENGTH_MS * MAX_API_FS_KHZ * MAX_INPUT_FRAMES];

	void * psDec;
	SKP_SILK_SDK_DecControlStruct decControl;

} SilkDecState;


static SKP_bool Decoder_Open(SilkDecState * s)
{
	SKP_int32 decSizeBytes;
	SKP_int32 ret;

	if (s->psDec != NULL)
		return FALSE;

	// Create Decoder
	ret = SKP_Silk_SDK_Get_Decoder_Size (&decSizeBytes);
	if (ret)
	{
		PRINTFERR ("[SILK] Get_Decoder_Size() returned %d\r\n", ret);
		return FALSE;
	}

	s->psDec = malloc (decSizeBytes);
	if (s->psDec == NULL)
	{
		PRINTFERR ("[SILK] Decoder_Open() malloc %d bytes failed\r\n", decSizeBytes);
		return FALSE;
	}

	return TRUE;
}


static SKP_bool Decoder_Close(SilkDecState * s)
{
	if (s->psDec != NULL)
	{
		free (s->psDec);
		s->psDec = NULL;
	}
	return TRUE;
}


//=============================================================================
// SampleRate -- 8000 or 16000 Hz
//=============================================================================
static SKP_bool Decoder_Start(SilkDecState * s, int SampleRate)
{
	SKP_int32 ret;

	if (s->psDec == NULL)
		return FALSE;

	// Check parameter
	if (SampleRate != 8000 && SampleRate != 16000)
		return FALSE;

	// Reset Decoder
	ret = SKP_Silk_SDK_InitDecoder(s->psDec);
	if (ret)
	{
		PRINTFERR ("[SILK] Decoder_Start() returned %d\r\n", ret);
		return FALSE;
	}

	// Set Decoder parameters
	s->decControl.API_sampleRate  = SampleRate;
	s->decControl.frameSize       = 0;
	s->decControl.framesPerPacket = 0;
	s->decControl.inBandFECOffset = 0;
	s->decControl.moreInternalDecoderFrames = 0;

	return TRUE;
}


static SKP_bool Decoder_Stop(SilkDecState * s)
{
	if (s->psDec == NULL)
		return FALSE;
	return TRUE;
}


static SKP_bool Decoder_Reset(SilkDecState * s)
{
	SKP_int32 ret;

	if (s->psDec == NULL)
		return FALSE;

	// Reset Decoder
	ret = SKP_Silk_SDK_InitDecoder(s->psDec);
	if (ret)
	{
		PRINTFERR ("[SILK] Decoder_Reset() returned %d\r\n", ret);
		return FALSE;
	}

	return TRUE;
}


static SKP_bool Decoder_Run (SilkDecState * s, SKP_bool bPacketLost, SKP_uint8 *PayloadBuf, int PayloadSize, SKP_int16 *PcmBuf, int *pnSamples)
{
	SKP_int32  ret;
	SKP_int16 *SamplePtr;
	SKP_int16  SampleCnt;
	SKP_int16  nSamples;
	int  frames, i;

	// Check parameter
	if (PcmBuf == NULL || pnSamples == NULL)
		return FALSE;
	if (PayloadBuf == NULL && bPacketLost == FALSE)
		return FALSE;
	if (PayloadBuf != NULL && PayloadSize <= 0)
		return FALSE;
	if (*pnSamples < FRAME_LENGTH_MS * MAX_API_FS_KHZ * MAX_INPUT_FRAMES)
		return FALSE;

	*pnSamples = 0;
	if (s->psDec == NULL)
		return FALSE;

	SamplePtr = PcmBuf;
	SampleCnt = 0;

	if (bPacketLost == FALSE)
	{
		// No Loss: Decode all frames in the packet
		frames = 0;
		do
		{
			// Hack for corrupt stream that could generate too many frames
			if (frames >= MAX_INPUT_FRAMES)
			{
				frames = 0;
				SamplePtr = PcmBuf;
				SampleCnt = 0;
				PRINTFERR ("[SILK] Decoder_Run() corrupt stream\r\n");
			}

			// Decode 20 ms
			ret = SKP_Silk_SDK_Decode (s->psDec, &s->decControl, 0, PayloadBuf, PayloadSize, SamplePtr, &nSamples);
			if (ret)
			{
				PRINTFERR ("[SILK] Decoder_Run() returned %d\r\n", ret);
				return FALSE;
			}

			frames ++;
			SamplePtr += nSamples;
			SampleCnt += nSamples;
			// Until last 20 ms frame of packet has been decoded
		} while (s->decControl.moreInternalDecoderFrames);
	}
	else
	{
		// Loss: Decode enough frames to cover one packet duration
		for (i = 0; i < s->decControl.framesPerPacket; i++)
		{
			// Generate 20 ms
			ret = SKP_Silk_SDK_Decode (s->psDec, &s->decControl, 1, PayloadBuf, PayloadSize, SamplePtr, &nSamples);
			if (ret)
			{
				PRINTFERR ("[SILK] Decoder_Run() returned %d\r\n", ret);
				return FALSE;
			}
			SamplePtr += nSamples;
			SampleCnt += nSamples;
		}
	}

	*pnSamples = SampleCnt;
	return TRUE;
}




typedef struct SilkEncState{
	int samplerate;
	int bitrate;
	int ptime;

	SKP_uint8 payload[ MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES ];
	SKP_int16 pcmbuff[ FRAME_LENGTH_MS * MAX_API_FS_KHZ * MAX_INPUT_FRAMES ];

	void *psEnc;
	SKP_SILK_SDK_EncControlStruct encControl;

} SilkEncState;


static SKP_bool Encoder_Open(SilkEncState * s)
{
	SKP_int32 encSizeBytes;
	SKP_int32 ret;

	if (s->psEnc != NULL)
		return FALSE;

	// Create Encoder
	ret = SKP_Silk_SDK_Get_Encoder_Size (&encSizeBytes);
	if (ret)
	{
		PRINTFERR ("[SILK] Get_Encoder_Size() returned %d\r\n", ret);
		return FALSE;
	}

	s->psEnc = malloc (encSizeBytes);
	if (s->psEnc == NULL)
	{
		PRINTFERR ("[SILK] Encoder_Open() malloc %d bytes failed\r\n", encSizeBytes);
		return FALSE;
	}

	return TRUE;
}


static SKP_bool Encoder_Close(SilkEncState * s)
{
	if (s->psEnc != NULL)
	{
		free (s->psEnc);
		s->psEnc = NULL;
	}
	return TRUE;
}


//=============================================================================
// SampleRate -- 8000 or 16000 Hz
// BitRate    -- 5000 ~~ 100000 bps
// FramesPerPayload -- 1 ~~ 5 frames per payload
//=============================================================================
static SKP_bool Encoder_Start (SilkEncState * s, int SampleRate, int BitRate, int FramesPerPayload)
{
	SKP_int32 ret;

	if (s->psEnc == NULL)
		return FALSE;

	// Check parameter
	if (SampleRate != 8000   && SampleRate != 16000)      return FALSE;
	if (BitRate < 5000       || BitRate > 100000)         return FALSE;
	if (FramesPerPayload < 1 || FramesPerPayload > 5)     return FALSE;

	// Reset Encoder
	ret = SKP_Silk_SDK_InitEncoder (s->psEnc, &s->encControl);
	if (ret)
	{
		PRINTFERR ("[SILK] Encoder_Start() returned %d\r\n", ret);
		return FALSE;
	}

	// Set Encoder parameters
	s->encControl.API_sampleRate        = SampleRate;
	s->encControl.maxInternalSampleRate = SampleRate;
	s->encControl.packetSize            = FramesPerPayload * SampleRate * FRAME_LENGTH_MS / 1000;
	s->encControl.packetLossPercentage  = 0;
	s->encControl.useInBandFEC          = FALSE;
	s->encControl.useDTX                = FALSE;
	s->encControl.complexity            = 0;
	s->encControl.bitRate               = BitRate;

	return TRUE;
}


static SKP_bool Encoder_Stop(SilkEncState * s)
{
	if (s->psEnc == NULL)
		return FALSE;
	return TRUE;
}


static SKP_bool Encoder_Reset(SilkEncState * s)
{
	SKP_int32 ret;
	SKP_SILK_SDK_EncControlStruct tmpControl;

	if (s->psEnc == NULL)
		return FALSE;

	tmpControl = s->encControl;
	if (s->encControl.packetSize == 0 || s->encControl.maxInternalSampleRate == 0 ||
		s->encControl.bitRate    == 0 || s->encControl.API_sampleRate == 0 )
		return FALSE;

	// Reset Encoder
	ret = SKP_Silk_SDK_InitEncoder (s->psEnc, &s->encControl);
	if (ret)
	{
		PRINTFERR ("[SILK] Encoder_Reset() returned %d\r\n", ret);
		return FALSE;
	}

	s->encControl = tmpControl;
	return TRUE;
}

static SKP_bool Encoder_Run (SilkEncState * s, SKP_int16 *SampleBuffer, int SampleNumber, SKP_uint8 *Payload, int *pnBytes)
{
	SKP_int32 ret;
	SKP_int16 nBytes;
	int i;

	// Check parameter
	if (SampleBuffer == NULL || Payload == NULL || pnBytes == NULL)
		return FALSE;
	if (SampleNumber != s->encControl.packetSize)
		return FALSE;
	if (*pnBytes < (MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES))
		return FALSE;

	*pnBytes = 0;
	if (s->psEnc == NULL)
		return FALSE;

	// Silk Encoder
	SampleNumber = s->encControl.API_sampleRate * FRAME_LENGTH_MS / 1000;
	for (i = 0; i < s->encControl.packetSize / SampleNumber; i++)
	{
		nBytes = MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES;
		ret = SKP_Silk_SDK_Encode (s->psEnc, &s->encControl, SampleBuffer, SampleNumber, Payload, &nBytes);
		if (ret)
		{
			PRINTFERR ("[SILK] Encoder_Run() returned %d\r\n", ret);
			return FALSE;
		}
		SampleBuffer += SampleNumber;
	}

	*pnBytes = nBytes;
	return TRUE;
}


/**************************/
/*						  */
/**************************/

HANDLE SilkEncCreate()
{
	SilkEncState* pEncoder = new SilkEncState();
	if(pEncoder == NULL)
		return NULL;

	memset(pEncoder, 0, sizeof(SilkEncState));

	pEncoder->samplerate = 8000;
	pEncoder->bitrate = 16000;
	pEncoder->ptime = 40;
	pEncoder->psEnc = NULL;
	memset(&pEncoder->encControl, 0, sizeof(SKP_SILK_SDK_EncControlStruct));

	Encoder_Open(pEncoder);
	Encoder_Start(pEncoder, pEncoder->samplerate, pEncoder->bitrate, pEncoder->ptime/FRAME_LENGTH_MS);
	return pEncoder;
}

int SilkEncRun(HANDLE hEncoder, char* pInBuf, int nInLen, char* pOutBuf)
{
	int paySize = 1250;
	Encoder_Run((SilkEncState*)hEncoder, (short*)pInBuf, nInLen >> 1, (BYTE*)pOutBuf, &paySize);
	return paySize;
}

void SilkEncDestroy(HANDLE hEncoder)
{
	if(hEncoder)
	{
		SilkEncState* p = (SilkEncState*)hEncoder;
		Encoder_Stop(p);
		Encoder_Close(p);
		delete p;
	}
}

HANDLE SilkDecCreate()
{
	SilkDecState* pDecoder = new SilkDecState();
	if(pDecoder == NULL)
		return NULL;

	memset(pDecoder, 0, sizeof(SilkDecState));
	pDecoder->samplerate = 8000;
	pDecoder->sample_count = 0;
	pDecoder->sync_seq = 0;
	pDecoder->plc = TRUE;
	pDecoder->plc_count = 0;
	pDecoder->psDec = NULL;
	memset(&pDecoder->decControl, 0, sizeof(SKP_SILK_SDK_DecControlStruct));

	Decoder_Open(pDecoder);
	Decoder_Start(pDecoder, pDecoder->samplerate);
	return pDecoder;
}

int SilkDecRun(HANDLE pDecoder, char* pInBuf, int nInLen, char* pOutBuf)
{
	int pcmSize = 2400;
	int ret = Decoder_Run((SilkDecState*)pDecoder, FALSE, (BYTE*)pInBuf, nInLen, (short*)pOutBuf, &pcmSize);
	return pcmSize;
}

void SilkDecDestroy(HANDLE pDecoder)
{
	if(pDecoder)
	{
		SilkDecState* p = (SilkDecState*)pDecoder;
		Decoder_Stop(p);
		Decoder_Close(p);
		delete p;
	}
}