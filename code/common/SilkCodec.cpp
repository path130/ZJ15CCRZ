#include "roomlib.h"
#include <string.h>
#include <stdlib.h>
#include  <stdio.h>
#define MAX_BYTES_PER_FRAME     250 // Equals peak bitrate of 100 kbps 
#define FRAME_LENGTH_MS         20
#define MAX_API_FS_KHZ          24//48
#define MAX_INPUT_FRAMES        1
typedef struct {
    /* I:   Output signal sampling rate in Hertz; 8000/12000/16000/24000                    */
    int API_sampleRate;

    /* O:   Number of samples per frame                                                     */
    int frameSize;

    /* O:   Frames per packet 1, 2, 3, 4, 5                                                 */
    int framesPerPacket;

    /* O:   Flag to indicate that the decoder has remaining payloads internally             */
    int moreInternalDecoderFrames;

    /* O:   Distance between main payload and redundant payload in packets                  */
    int inBandFECOffset;
} SKP_SILK_SDK_DecControlStruct;


typedef struct
{
	int samplerate;
	UINT64 sample_count;
	UINT64 frame_sample;
	UINT64 sync_seq;
	int plc_count;
	BOOL plc;
	short pcmbuff[ FRAME_LENGTH_MS * MAX_API_FS_KHZ * MAX_INPUT_FRAMES ];
	void * psDec;
	SKP_SILK_SDK_DecControlStruct decControl;
} SilkDecState;

typedef struct {
    /* I:   Input signal sampling rate in Hertz; 8000/12000/16000/24000                     */
    int API_sampleRate;

    /* I:   Maximum internal sampling rate in Hertz; 8000/12000/16000/24000                 */
    int maxInternalSampleRate;

    /* I:   Number of samples per packet; must be equivalent of 20, 40, 60, 80 or 100 ms    */
    int packetSize;

    /* I:   Bitrate during active speech in bits/second; internally limited                 */
    int bitRate;                        

    /* I:   Uplink packet loss in percent (0-100)                                           */
    int packetLossPercentage;
    
    /* I:   Complexity mode; 0 is lowest; 1 is medium and 2 is highest complexity           */
    int complexity;

    /* I:   Flag to enable in-band Forward Error Correction (FEC); 0/1                      */
    int useInBandFEC;

    /* I:   Flag to enable discontinuous transmission (DTX); 0/1                            */
    int useDTX;
} SKP_SILK_SDK_EncControlStruct;

typedef struct SilkEncState
{
	int samplerate;
	int bitrate;
	int ptime;
	DWORD ts;

	BYTE payload[ MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES ];
	short pcmbuff[ FRAME_LENGTH_MS * MAX_API_FS_KHZ * MAX_INPUT_FRAMES ];

	void *psEnc;
	SKP_SILK_SDK_EncControlStruct encControl;

} SilkEncState;

extern "C" int SKP_Silk_SDK_Get_Encoder_Size(int	*encSizeBytes);   /* O:   Number of bytes in SILK encoder state           */
extern "C" int SKP_Silk_SDK_InitEncoder(
    void                                *encState,      /* I/O: State                                           */
    SKP_SILK_SDK_EncControlStruct       *encStatus      /* O:   Encoder Status                                  */
);
extern "C" int SKP_Silk_SDK_Encode( 
    void                                *encState,      /* I/O: State                                           */
    const SKP_SILK_SDK_EncControlStruct *encControl,    /* I:   Control status                                  */
    const short                     *samplesIn,     /* I:   Speech sample input vector                      */
    int                             nSamplesIn,     /* I:   Number of samples in input vector               */
    BYTE                           *outData,       /* O:   Encoded output vector                           */
    short                           *nBytesOut      /* I/O: Number of bytes in outData (input: Max bytes)   */
);



extern "C" int SKP_Silk_SDK_Get_Decoder_Size(int *decSizeBytes);   /* O:   Number of bytes in SILK decoder state           */
extern "C" int SKP_Silk_SDK_InitDecoder(void *decState);       /* I/O: State                                           */
extern "C" int SKP_Silk_SDK_Decode(
    void*                               decState,       /* I/O: State                                           */
    SKP_SILK_SDK_DecControlStruct*      decControl,     /* I/O: Control Structure                               */
    int                             lostFlag,       /* I:   0: no loss, 1 loss                              */
    const BYTE                     *inData,        /* I:   Encoded input vector                            */
    const int                       nBytesIn,       /* I:   Number of input bytes                           */
    short                           *samplesOut,    /* O:   Decoded output speech vector                    */
    short                           *nSamplesOut    /* I/O: Number of samples (vector/decoded)              */
);


static BOOL Encoder_Open(SilkEncState * s)
{
	int encSizeBytes;
	int ret;

	if (s->psEnc != NULL)
		return FALSE;

	// Create Encoder
	ret = SKP_Silk_SDK_Get_Encoder_Size (&encSizeBytes);
	if (ret)
	{
		printf ("[SILK] Get_Encoder_Size() returned %d\r\n", ret);
		return FALSE;
	}

	s->psEnc = malloc (encSizeBytes);
	if (s->psEnc == NULL)
	{
		printf ("[SILK] Encoder_Open() malloc %d bytes failed\r\n", encSizeBytes);
		return FALSE;
	}
	memset(s->psEnc,0,encSizeBytes);
	return TRUE;
}


static BOOL Encoder_Close(SilkEncState * s)
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
static BOOL Encoder_Start (SilkEncState * s, int SampleRate, int BitRate, int FramesPerPayload)
{
	int ret;

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
		printf ("[SILK] Encoder_Start() returned %d\r\n", ret);
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


static short Encoder_Stop(SilkEncState * s)
{
	if (s->psEnc == NULL)
		return FALSE;
	return TRUE;
}

static BOOL Encoder_Run (SilkEncState * s, short *SampleBuffer, int SampleNumber, BYTE *Payload, int *pnBytes)
{
	int ret;
	short nBytes;
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
			printf ("[SILK] Encoder_Run() returned %d\r\n", ret);
			return FALSE;
		}
		SampleBuffer += SampleNumber;
//		printf ("[SILK] len %d\r\n", nBytes);
	}

	*pnBytes = nBytes;
	return TRUE;
}

static BOOL Decoder_Open(SilkDecState * s)
{
	int decSizeBytes;
	int ret;

	if (s->psDec != NULL)
		return FALSE;

	// Create Decoder
	ret = SKP_Silk_SDK_Get_Decoder_Size (&decSizeBytes);
	if (ret)
	{
		printf ("[SILK] Get_Decoder_Size() returned %d\r\n", ret);
		return FALSE;
	}

	s->psDec = malloc (decSizeBytes);
	if (s->psDec == NULL)
	{
		printf ("[SILK] Decoder_Open() malloc %d bytes failed\r\n", decSizeBytes);
		return FALSE;
	}
	memset(s->psDec,0,decSizeBytes);
	return TRUE;
}


static BOOL Decoder_Close(SilkDecState * s)
{
	if (s->psDec != NULL)
	{
		free (s->psDec);
		s->psDec = NULL;
	}
	return TRUE;
}

static BOOL Decoder_Start(SilkDecState * s, int SampleRate)
{
	int ret;

	if (s->psDec == NULL)
		return FALSE;

	// Check parameter
	if (SampleRate != 8000 && SampleRate != 16000)
		return FALSE;

	// Reset Decoder
	ret = SKP_Silk_SDK_InitDecoder(s->psDec);
	if (ret)
	{
		printf ("[SILK] Decoder_Start() returned %d\r\n", ret);
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


static BOOL Decoder_Stop(SilkDecState * s)
{
	if (s->psDec == NULL)
		return FALSE;
	return TRUE;
}

static BOOL Decoder_Run (SilkDecState * s, BOOL bPacketLost, BYTE *PayloadBuf, int PayloadSize, short *PcmBuf, int *pnSamples)
{
	int  ret;
	short *SamplePtr;
	short  SampleCnt;
	short  nSamples;
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
				printf ("[SILK] Decoder_Run() corrupt stream\r\n");
			}

			// Decode 20 ms
			ret = SKP_Silk_SDK_Decode (s->psDec, &s->decControl, 0, PayloadBuf, PayloadSize, SamplePtr, &nSamples);
			if (ret)
			{
				printf ("[SILK] Decoder_Run() returned %d\r\n", ret);
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
				printf ("[SILK] Decoder_Run() returned %d\r\n", ret);
				return FALSE;
			}
			SamplePtr += nSamples;
			SampleCnt += nSamples;
		}
	}

	*pnSamples = SampleCnt;
	return TRUE;
}

void silkenc_uninit(int idec)
{
	SilkEncState *s=(SilkEncState*)idec;
	if (s==NULL)
		return;

	Encoder_Stop(s);
	Encoder_Close(s);

	free(s);
}

int silkenc_init(void)
{
	SilkEncState *s = (SilkEncState*) malloc(sizeof(SilkEncState));
	memset(s,0,sizeof(SilkEncState));
	s->samplerate = 8000;
	s->bitrate = 8000;
	s->ptime = 40;
	s->ts = 0;

	s->psEnc = NULL;
	memset(&s->encControl, 0, sizeof(SKP_SILK_SDK_EncControlStruct));

	if (Encoder_Open(s) == FALSE)
	{
		printf("silk coder Encoder_Open error\n");
		return 0;
	}

	if(!Encoder_Start(s, 8000, 8000, MAX_INPUT_FRAMES))
	{
		printf("silk encoder Encoder_Start error\n");
		return 0;
	}
	return (int)s;
}

int silkenc_process(int idec, BYTE* encbuf, int* enclen, short* pcmbuf, int SampleNumber)
{
	return Encoder_Run((SilkEncState *)idec, pcmbuf, SampleNumber, encbuf, enclen);
}

int silkdec_init(void)
{
	SilkDecState *s = (SilkDecState *)malloc(sizeof(SilkDecState));
	memset(s,0,sizeof(SilkDecState));
	s->psDec = NULL;
	memset(&s->decControl, 0, sizeof(SKP_SILK_SDK_DecControlStruct));

	if(!Decoder_Open(s))
	{
		printf("silk decoder Decoder_Open error\n");
		return 0;
	}

	if(!Decoder_Start(s,8000))
	{
		printf("silk decoder Decoder_Start error\n");
		return 0;
	}

	return (int)s;
}

void silkdec_uninit(int idec)
{
	SilkDecState *s=(SilkDecState*)idec;
	if (s==NULL)
		return;

	Decoder_Stop(s);
	Decoder_Close(s);

	free(s);
}

int silkdec_process(int idec, char* encbuf, int enclen, short* pcmbuf)
{
	SilkDecState *s=(SilkDecState*)idec;
	int   nSamples = FRAME_LENGTH_MS * MAX_API_FS_KHZ * MAX_INPUT_FRAMES;

	Decoder_Run(s, FALSE, (BYTE*)encbuf, enclen, pcmbuf, &nSamples);
//	printf("dec %d product %d\r\n", enclen, nSamples);
	return nSamples;
}



