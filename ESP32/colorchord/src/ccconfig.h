#ifndef _CCCONFIG_H
#define _CCCONFIG_H

#include <c_types.h>

#define HPABUFFSIZE 512

#define CCEMBEDDED
#define NUM_LIN_LEDS 144
#define DFREQ 16000
#define LUXETRON 0
// #define memcpy ets_memcpy
// #define memset ets_memset

#define ROOT_NOTE_OFFSET	CCS.gROOT_NOTE_OFFSET
#define DFTIIR				CCS.gDFTIIR
#define FUZZ_IIR_BITS  		CCS.gFUZZ_IIR_BITS
#define MAXNOTES  12 //MAXNOTES cannot be changed dynamically.
#define FILTER_BLUR_PASSES	CCS.gFILTER_BLUR_PASSES
#define SEMIBITSPERBIN		CCS.gSEMIBITSPERBIN
#define MAX_JUMP_DISTANCE	CCS.gMAX_JUMP_DISTANCE
#define MAX_COMBINE_DISTANCE CCS.gMAX_COMBINE_DISTANCE
#define AMP_1_IIR_BITS		CCS.gAMP_1_IIR_BITS
#define AMP_2_IIR_BITS		CCS.gAMP_2_IIR_BITS
#define MIN_AMP_FOR_NOTE	CCS.gMIN_AMP_FOR_NOTE
#define MINIMUM_AMP_FOR_NOTE_TO_DISAPPEAR CCS.gMINIMUM_AMP_FOR_NOTE_TO_DISAPPEAR
#define NOTE_FINAL_AMP		CCS.gNOTE_FINAL_AMP
#define NERF_NOTE_PORP		CCS.gNERF_NOTE_PORP
#define USE_NUM_LIN_LEDS	CCS.gUSE_NUM_LIN_LEDS
#define COLORCHORD_OUTPUT_DRIVER	CCS.gCOLORCHORD_OUTPUT_DRIVER
#define COLORCHORD_ACTIVE	CCS.gCOLORCHORD_ACTIVE
#define INITIAL_AMP	CCS.gINITIAL_AMP
#define LED_DRIVER_MODE		CCS.gLED_DRIVER_MODE

//We are not enabling these for the ESP8266 port.
#define LIN_WRAPAROUND 0 
#define SORT_NOTES 0


struct CCSettings
{
	uint16_t gSETTINGS_KEY;
	uint16_t gROOT_NOTE_OFFSET; //Set to define what the root note is.  0 = A.
	uint16_t gDFTIIR;                            //=6
	uint16_t gFUZZ_IIR_BITS;                     //=1
	uint16_t gFILTER_BLUR_PASSES;                //=2
	uint16_t gSEMIBITSPERBIN;                    //=3
	uint16_t gMAX_JUMP_DISTANCE;                 //=4
	uint16_t gMAX_COMBINE_DISTANCE;              //=7
	uint16_t gAMP_1_IIR_BITS;                    //=4
	uint16_t gAMP_2_IIR_BITS;                    //=2
	uint16_t gMIN_AMP_FOR_NOTE;                  //=80
	uint16_t gMINIMUM_AMP_FOR_NOTE_TO_DISAPPEAR; //=64
	uint16_t gNOTE_FINAL_AMP;                    //=12
	uint16_t gNERF_NOTE_PORP;                    //=15
	uint16_t gUSE_NUM_LIN_LEDS;                  // = NUM_LIN_LEDS
	uint16_t gCOLORCHORD_ACTIVE;
	uint16_t gCOLORCHORD_OUTPUT_DRIVER;
	uint16_t gLED_DRIVER_MODE;
	uint16_t gINITIAL_AMP;
};

#define CCCONFIG_ADDRESS 0xAF000

// extern struct CCSettings CCS;
struct CCSettings CCS;


#endif

//Copyright 2015 <>< Charles Lohr under the ColorChord License.

#ifndef _DFT32_H
#define _DFT32_H

#ifdef ICACHE_FLASH
// #include <c_types.h> //If on ESP8266
#else
#include <stdint.h>
#endif

//A 32-bit version of the DFT used for ColorChord.
//This header makes it convenient to use for an embedded system.
//The 32-bit DFT avoids some bit shifts, however it uses slightly
//more RAM and it uses a lot of 32-bit arithmatic. 
//
//This is basically a clone of "ProgressiveIntegerSkippy" and changes
//made here should be backported there as well.

//You can # define these to be other things elsewhere.

// Will used simple approximation of norm rather than
//   sum squares and approx sqrt
#ifndef APPROXNORM
#define APPROXNORM 1
#endif

#ifndef OCTAVES
#define OCTAVES  5
#endif

#ifndef FIXBPERO
#define FIXBPERO 24
#endif

//Don't configure this.
#define FIXBINS  (FIXBPERO*OCTAVES)
#define BINCYCLE (1<<OCTAVES)

//You may increase this past 5 but if you do, the amplitude of your incoming
//signal must decrease.  Increasing this value makes responses slower.  Lower
//values are more responsive.
#ifndef DFTIIR
#define DFTIIR 6
#endif

//Everything the integer one buys, except it only calculates 2 octaves worth of
//notes per audio frame.
//This is sort of working, but still have some quality issues.
//It would theoretically be fast enough to work on an AVR.
//NOTE: This is the only DFT available to the embedded port of ColorChord
#ifndef CCEMBEDDED
void DoDFTProgressive32( float * outbins, float * frequencies, int bins,
	const float * databuffer, int place_in_data_buffer, int size_of_data_buffer,
	float q, float speedup );
#endif

//It's actually split into a few functions, which you can call on your own:
int SetupDFTProgressive32();  //Call at start. Returns nonzero if error.
void UpdateBins32( const uint16_t * frequencies );

//Call this to push on new frames of sound. 
//Though it accepts an int16, it actually only takes -4095 to +4095. (13-bit)
//Any more and you will exceed the accumulators and it will cause an overflow.
void PushSample32( int16_t dat );

#ifndef CCEMBEDDED
//ColorChord regular uses this to pass in floats.
void UpdateBinsForDFT32( const float * frequencies ); //Update the frequencies
#endif

//This takes the current sin/cos state of ColorChord and output to
//embeddedbins32.
void UpdateOutputBins32();

//Whenever you need to read the bins, you can do it from here.
//These outputs are limited to 0..~2047, this makes it possible
//for you to process with uint16_t's more easily.
//This is updated every time the DFT hits the octavecount, or 1/32 updates.
extern uint16_t embeddedbins32[];  //[FIXBINS]


#endif

//Copyright 2015 <>< Charles Lohr under the ColorChord License.

#include <string.h>

#ifndef CCEMBEDDED
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
static float * goutbins;
#endif

uint16_t embeddedbins32[FIXBINS]; 

//NOTES to self:
//
// Let's say we want to try this on an AVR.
//  24 bins, 5 octaves = 120 bins.
// 20 MHz clock / 4.8k sps = 4096 IPS = 34 clocks per bin = :(
//  We can do two at the same time, this frees us up some 

static uint8_t Sdonefirstrun;

//A table of precomputed sin() values.  Ranging -1500 to +1500
//If we increase this, it may cause overflows elsewhere in code.
const int16_t Ssinonlytable[256] = {
             0,    36,    73,   110,   147,   183,   220,   256,
           292,   328,   364,   400,   435,   470,   505,   539,
           574,   607,   641,   674,   707,   739,   771,   802,
           833,   863,   893,   922,   951,   979,  1007,  1034,
          1060,  1086,  1111,  1135,  1159,  1182,  1204,  1226,
          1247,  1267,  1286,  1305,  1322,  1339,  1355,  1371,
          1385,  1399,  1412,  1424,  1435,  1445,  1455,  1463,
          1471,  1477,  1483,  1488,  1492,  1495,  1498,  1499,
          1500,  1499,  1498,  1495,  1492,  1488,  1483,  1477,
          1471,  1463,  1455,  1445,  1435,  1424,  1412,  1399,
          1385,  1371,  1356,  1339,  1322,  1305,  1286,  1267,
          1247,  1226,  1204,  1182,  1159,  1135,  1111,  1086,
          1060,  1034,  1007,   979,   951,   922,   893,   863,
           833,   802,   771,   739,   707,   674,   641,   607,
           574,   539,   505,   470,   435,   400,   364,   328,
           292,   256,   220,   183,   147,   110,    73,    36,
             0,   -36,   -73,  -110,  -146,  -183,  -219,  -256,
          -292,  -328,  -364,  -399,  -435,  -470,  -505,  -539,
          -573,  -607,  -641,  -674,  -706,  -739,  -771,  -802,
          -833,  -863,  -893,  -922,  -951,  -979, -1007, -1034,
         -1060, -1086, -1111, -1135, -1159, -1182, -1204, -1226,
         -1247, -1267, -1286, -1305, -1322, -1339, -1355, -1371,
         -1385, -1399, -1412, -1424, -1435, -1445, -1454, -1463,
         -1471, -1477, -1483, -1488, -1492, -1495, -1498, -1499,
         -1500, -1499, -1498, -1495, -1492, -1488, -1483, -1477,
         -1471, -1463, -1455, -1445, -1435, -1424, -1412, -1399,
         -1385, -1371, -1356, -1339, -1322, -1305, -1286, -1267,
         -1247, -1226, -1204, -1182, -1159, -1135, -1111, -1086,
         -1060, -1034, -1007,  -979,  -951,  -923,  -893,  -863,
          -833,  -802,  -771,  -739,  -707,  -674,  -641,  -608,
          -574,  -540,  -505,  -470,  -435,  -400,  -364,  -328,
          -292,  -256,  -220,  -183,  -147,  -110,   -73,   -37,};


// /** The above table was created using the following code:
// #include <math.h>
// #include <stdio.h>
// #include <stdint.h>

// int16_t Ssintable[256]; //Actually, just [sin].

// int main()
// {
	// int i;
	// for( i = 0; i < 256; i++ )
	// {
		// Ssintable[i] = (int16_t)((sinf( i / 256.0 * 6.283 ) * 1500.0));
	// }

	// printf( "const int16_t Ssinonlytable[256] = {" );
	// for( i = 0; i < 256; i++ )
	// {
		// if( !(i & 0x7 ) )
		// {
			// printf( "\n\t" );
		// }
		// printf( "%6d," ,Ssintable[i] );
	// }
	// printf( "};\n" );
// } */



uint16_t Sdatspace32A[FIXBINS*2];  //(advances,places) full revolution is 256. 8bits integer part 8bit fractional
int32_t Sdatspace32B[FIXBINS*2];  //(isses,icses)

//This is updated every time the DFT hits the octavecount, or 1 out of (1<<OCTAVES) times which is (1<<(OCTAVES-1)) samples
int32_t Sdatspace32BOut[FIXBINS*2];  //(isses,icses)

//Sdo_this_octave is a scheduling state for the running SIN/COS states for
//each bin.  We have to execute the highest octave every time, however, we can
//get away with updating the next octave down every-other-time, then the next
//one down yet, every-other-time from that one.  That way, no matter how many
//octaves we have, we only need to update FIXBPERO*2 DFT bins.
static uint8_t Sdo_this_octave[BINCYCLE];

static int32_t Saccum_octavebins[OCTAVES];
static uint8_t Swhichoctaveplace;

//
uint16_t embeddedbins[FIXBINS]; 

//From: http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
//  for sqrt approx but also suggestion for quick norm approximation that would work in this DFT

#if APPROXNORM != 1
/**
 * \brief    Fast Square root algorithm, with rounding
 *
 * This does arithmetic rounding of the result. That is, if the real answer
 * would have a fractional part of 0.5 or greater, the result is rounded up to
 * the next integer.
 *      - SquareRootRounded(2) --> 1
 *      - SquareRootRounded(3) --> 2
 *      - SquareRootRounded(4) --> 2
 *      - SquareRootRounded(6) --> 2
 *      - SquareRootRounded(7) --> 3
 *      - SquareRootRounded(8) --> 3
 *      - SquareRootRounded(9) --> 3
 *
 * \param[in] a_nInput - unsigned integer for which to find the square root
 *
 * \return Integer square root of the input value.
 */
static uint16_t SquareRootRounded(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }

    /* Do arithmetic rounding to nearest integer */
    if (op > res)
    {
        res++;
    }

    return res;
}
#endif

void UpdateOutputBins32()
{
	int i;
	int32_t * ipt = &Sdatspace32BOut[0];
	for( i = 0; i < FIXBINS; i++ )
	{
		int32_t isps = *(ipt++); //keep 32 bits
		int32_t ispc = *(ipt++);
		// take absolute values
		isps = isps<0? -isps : isps;
		ispc = ispc<0? -ispc : ispc;
		int octave = i / FIXBPERO;

		//If we are running DFT32 on regular ColorChord, then we will need to
		//also update goutbins[]... But if we're on embedded systems, we only
		//update embeddedbins32.
#ifndef CCEMBEDDED
		// convert 32 bit precision isps and ispc to floating point
		float mux = ( (float)isps * (float)isps) + ((float)ispc * (float)ispc);
		goutbins[i] = sqrtf(mux)/65536.0; // scale by 2^16
		//reasonable (but arbitrary attenuation)
		goutbins[i] /= (78<<DFTIIR)*(1<<octave); 
#endif

#if APPROXNORM == 1
		// using full 32 bit precision for isps and ispc
		uint32_t rmux = isps>ispc? isps + (ispc>>1) : ispc + (isps>>1);
		rmux = rmux>>16; // keep most significant 16 bits
#else
		// use the most significant 16 bits of isps and ispc when squaring
		// since isps and ispc are non-negative right bit shifing is well defined
		uint32_t rmux = ( (isps>>16) * (isps>>16)) + ((ispc>16) * (ispc>>16));
		rmux = SquareRootRounded( rmux );
#endif

		//bump up all outputs here, so when we nerf it by bit shifting by
		//octave we don't lose a lot of detail.
		rmux = rmux << 1;

		embeddedbins32[i] = rmux >> octave;
	}
}

static void HandleInt( int16_t sample )
{
	int i;
	uint16_t adv;
	uint8_t localipl;
	int16_t filteredsample;

	uint8_t oct = Sdo_this_octave[Swhichoctaveplace];
	Swhichoctaveplace ++;
	Swhichoctaveplace &= BINCYCLE-1;

	for( i = 0; i < OCTAVES;i++ )
	{
		Saccum_octavebins[i] += sample;
	}

	if( oct > 128 )
	{
		//Special: This is when we can update everything.
		//This gets run once out of every (1<<OCTAVES) times.
		// which is half as many samples
		//It handles updating part of the DFT.
		//It should happen at the very first call to HandleInit
		int32_t * bins = &Sdatspace32B[0];
		int32_t * binsOut = &Sdatspace32BOut[0];

		for( i = 0; i < FIXBINS; i++ )
		{
			//First for the SIN then the COS.
			int32_t val = *(bins);
			*(binsOut++) = val;
			*(bins++) -= val>>DFTIIR;

			val = *(bins);
			*(binsOut++) = val;
			*(bins++) -= val>>DFTIIR;
		}
		return;
	}

	// process a filtered sample for one of the octaves
	uint16_t * dsA = &Sdatspace32A[oct*FIXBPERO*2];
	int32_t * dsB = &Sdatspace32B[oct*FIXBPERO*2];

	filteredsample = Saccum_octavebins[oct]>>(OCTAVES-oct);
	Saccum_octavebins[oct] = 0;

	for( i = 0; i < FIXBPERO; i++ )
	{
		adv = *(dsA++);
		localipl = *(dsA) >> 8;
		*(dsA++) += adv;

		*(dsB++) += (Ssinonlytable[localipl] * filteredsample);
		//Get the cosine (1/4 wavelength out-of-phase with sin)
		localipl += 64;
		*(dsB++) += (Ssinonlytable[localipl] * filteredsample);
	}
}

int SetupDFTProgressive32()
{
	int i;
	int j;

	Sdonefirstrun = 1;
	Sdo_this_octave[0] = 0xff;
	for( i = 0; i < BINCYCLE-1; i++ )
	{
		// Sdo_this_octave = 
		// 255 4 3 4 2 4 3 4 1 4 3 4 2 4 3 4 0 4 3 4 2 4 3 4 1 4 3 4 2 4 3 4 is case for 5 octaves.
		// Initial state is special one, then at step i do octave = Sdo_this_octave with averaged samples from last update of that octave
		//search for "first" zero

		for( j = 0; j <= OCTAVES; j++ )
		{
			if( ((1<<j) & i) == 0 ) break;
		}
		if( j > OCTAVES )
		{
#ifndef CCEMBEDDED
			fprintf( stderr, "Error: algorithm fault.\n" );
			exit( -1 );
#endif
			return -1;
		}
		Sdo_this_octave[i+1] = OCTAVES-j-1;
	}
	return 0;
}



void UpdateBins32( const uint16_t * frequencies )
{
	int i;
	int imod = 0;
	for( i = 0; i < FIXBINS; i++, imod++ )
	{
		if (imod >= FIXBPERO) imod=0;
		uint16_t freq = frequencies[imod];
		Sdatspace32A[i*2] = freq;// / oneoveroctave;
	}
}

void PushSample32( int16_t dat )
{
	HandleInt( dat );
	HandleInt( dat );
}


#ifndef CCEMBEDDED

void UpdateBinsForDFT32( const float * frequencies )
{
	int i;	
	for( i = 0; i < FIXBINS; i++ )
	{
		float freq = frequencies[(i%FIXBPERO) + (FIXBPERO*(OCTAVES-1))];
		Sdatspace32A[i*2] = (65536.0/freq);// / oneoveroctave;
	}
}

#endif


#ifndef CCEMBEDDED

void DoDFTProgressive32( float * outbins, float * frequencies, int bins, const float * databuffer, int place_in_data_buffer, int size_of_data_buffer, float q, float speedup )
{
	static float backupbins[FIXBINS];
	int i;
	static int last_place;

	memset( outbins, 0, bins * sizeof( float ) );
	goutbins = outbins;

	memcpy( outbins, backupbins, FIXBINS*4 );

	if( FIXBINS != bins )
	{
		fprintf( stderr, "Error: Bins was reconfigured.  skippy requires a constant number of bins (%d != %d).\n", FIXBINS, bins );
		return;
	}

	if( !Sdonefirstrun )
	{
		SetupDFTProgressive32();
		Sdonefirstrun = 1;
	}

	UpdateBinsForDFT32( frequencies );

	for( i = last_place; i != place_in_data_buffer; i = (i+1)%size_of_data_buffer )
	{
		int16_t ifr1 = (int16_t)( ((databuffer[i]) ) * 4095 );
		HandleInt( ifr1 );
		HandleInt( ifr1 );
	}

	UpdateOutputBins32();

	last_place = place_in_data_buffer;

	memcpy( backupbins, outbins, FIXBINS*4 );
}

#endif


//Copyright 2015 <>< Charles Lohr under the ColorChord License.

#ifndef _EMBEDDEDNF_H
#define _EMBEDDEDNF_H

//Use a 32-bit DFT.  It won't work for AVRs, but for any 32-bit systems where
//they can multiply quickly, this is the bees knees.
#define USE_32DFT

#ifndef DFREQ
#define DFREQ     8000
#endif

//You may make this a float. If PRECOMPUTE_FREQUENCY_TABLE is defined, then
//it will create the table at compile time, and the float will never be used
//runtime.
#define BASE_FREQ 55.0

//The higher the number the slackier your FFT will be come.
#ifndef FUZZ_IIR_BITS
#define FUZZ_IIR_BITS  1
#endif

//Notes are the individually identifiable notes we receive from the sound.
//We track up to this many at one time.  Just because a note may appear to
//vaporize in one frame doesn't mean it is annihilated immediately.
#ifndef MAXNOTES
#define MAXNOTES  12
#endif

//We take the raw signal off of the 
#ifndef FILTER_BLUR_PASSES
#define FILTER_BLUR_PASSES 2
#endif

//Determines bit shifts for where notes lie.  We represent notes with an
//uint8_t.  We have to define all of the possible locations on the note line
//in this. note_frequency = 0..((1<<SEMIBITSPERBIN)*FIXBPERO-1)
#ifndef SEMIBITSPERBIN
#define SEMIBITSPERBIN 3 
#endif

#define NOTERANGE ((1<<SEMIBITSPERBIN)*FIXBPERO)


//If there is detected note this far away from an established note, we will
//then consider this new note the same one as last time, and move the
//established note.  This is also used when combining notes.  It is this
//distance times two.
#ifndef MAX_JUMP_DISTANCE
#define MAX_JUMP_DISTANCE 4
#endif

#ifndef MAX_COMBINE_DISTANCE
#define MAX_COMBINE_DISTANCE 7
#endif

//These control how quickly the IIR for the note strengths respond.  AMP 1 is
//the response for the slow-response, or what we use to determine size of
//splotches, AMP 2 is the quick response, or what we use to see the visual
//strength of the notes.
#ifndef AMP_1_IIR_BITS
#define AMP_1_IIR_BITS 4
#endif

#ifndef AMP_2_IIR_BITS
#define AMP_2_IIR_BITS 2
#endif

//This is the amplitude, coming from folded_bins.  If the value is below this
//it is considered a non-note.
#ifndef MIN_AMP_FOR_NOTE
#define MIN_AMP_FOR_NOTE 80
#endif

//If the strength of a note falls below this, the note will disappear, and be
//recycled back into the unused list of notes.
#ifndef MINIMUM_AMP_FOR_NOTE_TO_DISAPPEAR
#define MINIMUM_AMP_FOR_NOTE_TO_DISAPPEAR 64
#endif

//This prevents compilation of any floating-point code, but it does come with
//an added restriction: Both DFREQ and BASE_FREQ must be #defined to be
//constants.
#define PRECOMPUTE_FREQUENCY_TABLE

extern uint16_t fuzzed_bins[]; //[FIXBINS]  <- The Full DFT after IIR, Blur and Taper

extern uint16_t folded_bins[]; //[FIXBPERO] <- The folded fourier output.

//frequency of note; Note if it is == 255, then it means it is not set. It is
//generally a value from 
extern uint8_t  note_peak_freqs[]; //[MAXNOTES]
extern uint16_t note_peak_amps[];  //[MAXNOTES] 
extern uint16_t note_peak_amps2[]; //[MAXNOTES]  (Responds quicker)
extern uint8_t  note_jumped_to[];  //[MAXNOTES] When a note combines into another one,
	//this records where it went.  I.e. if your note just disappeared, check this flag.

void UpdateFreqs();		//Not user-useful on most systems.
void HandleFrameInfo();	//Not user-useful on most systems



//Call this when starting.
void InitColorChord();


#endif

//Copyright 2015 <>< Charles Lohr under the ColorChord License.

#include <stdio.h>
#include <string.h>

uint16_t folded_bins[FIXBPERO];
uint16_t fuzzed_bins[FIXBINS];
uint8_t  note_peak_freqs[MAXNOTES];
uint16_t note_peak_amps[MAXNOTES];
uint16_t note_peak_amps2[MAXNOTES];
uint8_t  note_jumped_to[MAXNOTES];


#ifndef PRECOMPUTE_FREQUENCY_TABLE
static const float bf_table[24] = {
        1.000000, 1.029302, 1.059463, 1.090508, 1.122462, 1.155353, 
        1.189207, 1.224054, 1.259921, 1.296840, 1.334840, 1.373954, 
        1.414214, 1.455653, 1.498307, 1.542211, 1.587401, 1.633915, 
        1.681793, 1.731073, 1.781797, 1.834008, 1.887749, 1.943064 };

// /* The above table was generated using the following code:

// #include <stdio.h>
// #include <math.h>

// int main()
// {
	// int i;
	// #define FIXBPERO 24
	// printf( "const float bf_table[%d] = {", FIXBPERO );
	// for( i = 0; i < FIXBPERO; i++ )
	// {
		// if( ( i % 6 ) == 0 )
			// printf( "\n\t" );
		// printf( "%f, ", pow( 2, (float)i / (float)FIXBPERO ) );
	// }
	// printf( "};\n" );
	// return 0;
// }
// */

#endif

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))


void UpdateFreqs()
{

#ifndef PRECOMPUTE_FREQUENCY_TABLE
	uint16_t fbins[FIXBPERO];
	int i;

	BUILD_BUG_ON( sizeof(bf_table) != FIXBPERO*4 );

	//Warning: This does floating point.  Avoid doing this frequently.  If you
	//absolutely cannot have floating point on your system, you may precompute
	//this and store it as a table.  It does preclude you from changing
	//BASE_FREQ in runtime.

	for( i = 0; i < FIXBPERO; i++ )
	{
		float frq =  ( bf_table[i] * BASE_FREQ );
		fbins[i] = ( 65536.0 ) / ( DFREQ ) * frq * 16 + 0.5;
	}
#else

	#define PCOMP( f )  (uint16_t)((65536.0)/(DFREQ) * (f * BASE_FREQ) * 16 + 0.5)

	static const uint16_t fbins[FIXBPERO] = { 
		PCOMP( 1.000000 ), PCOMP( 1.029302 ), PCOMP( 1.059463 ), PCOMP( 1.090508 ), PCOMP( 1.122462 ), PCOMP( 1.155353 ), 
		PCOMP( 1.189207 ), PCOMP( 1.224054 ), PCOMP( 1.259921 ), PCOMP( 1.296840 ), PCOMP( 1.334840 ), PCOMP( 1.373954 ),
		PCOMP( 1.414214 ), PCOMP( 1.455653 ), PCOMP( 1.498307 ), PCOMP( 1.542211 ), PCOMP( 1.587401 ), PCOMP( 1.633915 ),
		PCOMP( 1.681793 ), PCOMP( 1.731073 ), PCOMP( 1.781797 ), PCOMP( 1.834008 ), PCOMP( 1.887749 ), PCOMP( 1.943064 ) };
#endif

#ifdef USE_32DFT
	UpdateBins32( fbins );
#else
	UpdateBinsForProgressiveIntegerSkippyInt( fbins );
#endif
}

void InitColorChord()
{
	int i;
	//Set up and initialize arrays.
	for( i = 0; i < MAXNOTES; i++ )
	{
		note_peak_freqs[i] = 255;
		note_peak_amps[i] = 0;
		note_peak_amps2[i] = 0;
	}

	memset( folded_bins, 0, sizeof( folded_bins ) );
	memset( fuzzed_bins, 0, sizeof( fuzzed_bins ) );

	//Step 1: Initialize the Integer DFT.
#ifdef USE_32DFT
	SetupDFTProgressive32();
#else
	SetupDFTProgressiveIntegerSkippy();
#endif

	//Step 2: Set up the frequency list.  You could do this multiple times
	//if you want to change the loadout of the frequencies.
	UpdateFreqs();
}

void HandleFrameInfo()
{
	int i, j, k;
	uint8_t hitnotes[MAXNOTES];
	memset( hitnotes, 0, sizeof( hitnotes ) );

#ifdef USE_32DFT
	uint16_t * strens;
	UpdateOutputBins32();
	strens = embeddedbins32;
#else
	uint16_t * strens = embeddedbins;
#endif

	//Copy out the bins from the DFT to our fuzzed bins.
	for( i = 0; i < FIXBINS; i++ )
	{
		fuzzed_bins[i] = (fuzzed_bins[i] + (strens[i]>>FUZZ_IIR_BITS) -
			(fuzzed_bins[i]>>FUZZ_IIR_BITS));
	}

	//Taper first octave
	for( i = 0; i < FIXBPERO; i++ )
	{
		uint32_t taperamt = (65536 / FIXBPERO) * i;
		fuzzed_bins[i] = (taperamt * fuzzed_bins[i]) >> 16;
	}

	//Taper last octave
	for( i = 0; i < FIXBPERO; i++ )
	{
		int newi = FIXBINS - i - 1;
		uint32_t taperamt = (65536 / FIXBPERO) * i;
		fuzzed_bins[newi] = (taperamt * fuzzed_bins[newi]) >> 16;
	}

	//Fold the bins from fuzzedbins into one octave.
	for( i = 0; i < FIXBPERO; i++ )
		folded_bins[i] = 0;
	k = 0;
	for( j = 0; j < OCTAVES; j++ )
	{
		for( i = 0; i < FIXBPERO; i++ )
		{
			folded_bins[i] += fuzzed_bins[k++];
		}
	}

	//Now, we must blur the folded bins to get a good result.
	//Sometimes you may notice every other bin being out-of
	//line, and this fixes that.  We may consider running this
	//more than once, but in my experience, once is enough.
	for( j = 0; j < FILTER_BLUR_PASSES; j++ )
	{
		//Extra scoping because this is a large on-stack buffer.
		uint16_t folded_out[FIXBPERO];
		uint8_t adjLeft = FIXBPERO-1;
		uint8_t adjRight = 1;
		for( i = 0; i < FIXBPERO; i++ )
		{
			uint16_t lbin = folded_bins[adjLeft]>>2;
			uint16_t rbin = folded_bins[adjRight]>>2;
			uint16_t tbin = folded_bins[i]>>1;
			folded_out[i] = lbin + rbin + tbin;

			//We do this funny dance to avoid a modulus operation.  On some
			//processors, a modulus operation is slow.  This is cheap.
			adjLeft++; if( adjLeft == FIXBPERO ) adjLeft = 0;
			adjRight++; if( adjRight == FIXBPERO ) adjRight = 0;
		}

		for( i = 0; i < FIXBPERO; i++ )
		{
			folded_bins[i] = folded_out[i];
		}
	}

	//Next, we have to find the peaks, this is what "decompose" does in our
	//normal tool.  As a warning, it expects that the values in foolded_bins
	//do NOT exceed 32767.
	{
		uint8_t adjLeft = FIXBPERO-1;
		uint8_t adjRight = 1;
		for( i = 0; i < FIXBPERO; i++ )
		{
			int16_t prev = folded_bins[adjLeft];
			int16_t next = folded_bins[adjRight];
			int16_t _this = folded_bins[i];
			uint8_t thisfreq = i<<SEMIBITSPERBIN;
			int16_t offset;
			adjLeft++; if( adjLeft == FIXBPERO ) adjLeft = 0;
			adjRight++; if( adjRight == FIXBPERO ) adjRight = 0;
			if( _this < MIN_AMP_FOR_NOTE ) continue;
			if( prev > _this || next > _this ) continue;
			if( prev == _this && next == _this ) continue;

			//i is at a peak... 
			int32_t totaldiff = (( _this - prev ) + ( _this - next ));
			int32_t porpdiffP = ((_this-prev)<<16) / totaldiff; //close to 0 =
					//closer to _this side, 32768 = in the middle, 65535 away.
			int32_t porpdiffN = ((_this-next)<<16) / totaldiff;

			if( porpdiffP < porpdiffN )
			{
				//Closer to prev.
				offset = -(32768 - porpdiffP);
			}
			else
			{
				//Closer to next
				offset = (32768 - porpdiffN);
			}

			//Need to round.  That's what that extra +(15.. is in the center.
			thisfreq += (offset+(1<<(15-SEMIBITSPERBIN)))>>(16-SEMIBITSPERBIN);

			//In the event we went 'below zero' need to wrap to the top.
			if( thisfreq > 255-(1<<SEMIBITSPERBIN) )
				thisfreq = (1<<SEMIBITSPERBIN)*FIXBPERO - (256-thisfreq);

			//Okay, we have a peak, and a frequency. Now, we need to search
			//through the existing notes to see if we have any matches.
			//If we have a note that's close enough, we will try to pull it
			//closer to us and boost it.
			int8_t lowest_found_free_note = -1;
			int8_t closest_note_id = -1;
			int16_t closest_note_distance = 32767;
			
			for( j = 0; j < MAXNOTES; j++ )
			{
				uint8_t nf = note_peak_freqs[j];

				if( nf == 255 )
				{
					if( lowest_found_free_note == -1 )
						lowest_found_free_note = j;
					continue;
				}

				int16_t distance = thisfreq - nf;

				if( distance < 0 ) distance = -distance;

				//Make sure that if we've wrapped around the right side of the
				//array, we can detect it and loop it back.
				if( distance > ((1<<(SEMIBITSPERBIN-1))*FIXBPERO) )
				{
					distance = ((1<<(SEMIBITSPERBIN))*FIXBPERO) - distance;
				}

				//If we find a note closer to where we are than any of the 
				//others, we can mark it as our desired note.
				if( distance < closest_note_distance )
				{	
					closest_note_id = j;
					closest_note_distance = distance;
				}
			}

			int8_t marked_note = -1;

			if( closest_note_distance <= MAX_JUMP_DISTANCE )
			{
				//We found the note we need to augment.
				note_peak_freqs[closest_note_id] = thisfreq;
				marked_note = closest_note_id;
			}

			//The note was not found.
			else if( lowest_found_free_note != -1 )
			{
				note_peak_freqs[lowest_found_free_note] = thisfreq;
				marked_note = lowest_found_free_note;
			}

			//If we found a note to attach to, we have to use the IIR to
			//increase the strength of the note, but we can't exactly snap
			//it to the new strength.
			if( marked_note != -1 )
			{
				hitnotes[marked_note] = 1;

				note_peak_amps[marked_note] =
					note_peak_amps[marked_note] -
					(note_peak_amps[marked_note]>>AMP_1_IIR_BITS) +
					(_this>>(AMP_1_IIR_BITS-3));

				note_peak_amps2[marked_note] =
					note_peak_amps2[marked_note] -
					(note_peak_amps2[marked_note]>>AMP_2_IIR_BITS) +
					((_this<<3)>>(AMP_2_IIR_BITS));
			}
		}
	}

#if 0
	for( i = 0; i < MAXNOTES; i++ )
	{
		if( note_peak_freqs[i] == 255 ) continue;
		printf( "%d / ", note_peak_amps[i] );
	}
	printf( "\n" );
#endif

	//Now we need to handle combining notes.
	for( i = 0; i < MAXNOTES; i++ )
	for( j = 0; j < i; j++ )
	{
		//We'd be combining nf2 (j) into nf1 (i) if they're close enough.
		uint8_t nf1 = note_peak_freqs[i];
		uint8_t nf2 = note_peak_freqs[j];
		int16_t distance = nf1 - nf2;

		if( nf1 == 255 || nf2 == 255 ) continue;

		if( distance < 0 ) distance = -distance;

		//If it wraps around above the halfway point, then we're closer to it
		//on the other side. 
		if( distance > ((1<<(SEMIBITSPERBIN-1))*FIXBPERO) )
		{
			distance = ((1<<(SEMIBITSPERBIN))*FIXBPERO) - distance;
		}

		if( distance > MAX_COMBINE_DISTANCE )
		{
			continue;
		}

		int into;
		int from;

		if( note_peak_amps[i] > note_peak_amps[j] )
		{
			into = i;
			from = j;
		}
		else
		{
			into = j;
			from = i;
		}

		//We need to combine the notes.  We need to move the new note freq
		//towards the stronger of the two notes.  
		int16_t amp1 = note_peak_amps[into];
		int16_t amp2 = note_peak_amps[from];

		//0 to 32768 porportional to how much of amp1 we want.
		uint32_t porp = (amp1<<15) / (amp1+amp2);  
		uint16_t newnote = (nf1 * porp + nf2 * (32768-porp))>>15;

		//When combining notes, we have to use the stronger amplitude note.
		//trying to average or combine the power of the notes looks awful.
		note_peak_freqs[into] = newnote;
		note_peak_amps[into] = (note_peak_amps[into]>note_peak_amps[from])?
				note_peak_amps[into]:note_peak_amps[j];
		note_peak_amps2[into] = (note_peak_amps2[into]>note_peak_amps2[from])?
				note_peak_amps2[into]:note_peak_amps2[j];

		note_peak_freqs[from] = 255;
		note_peak_amps[from] = 0;
		note_jumped_to[from] = i;
	}

	//For al lof the notes that have not been hit, we have to allow them to
	//to decay.  We only do this for notes that have not found a peak.
	for( i = 0; i < MAXNOTES; i++ )
	{
		if( note_peak_freqs[i] == 255 || hitnotes[i] ) continue;

		note_peak_amps[i] -= note_peak_amps[i]>>AMP_1_IIR_BITS;
		note_peak_amps2[i] -= note_peak_amps2[i]>>AMP_2_IIR_BITS;

		//In the event a note is not strong enough anymore, it is to be
		//returned back into the great pool of unused notes.
		if( note_peak_amps[i] < MINIMUM_AMP_FOR_NOTE_TO_DISAPPEAR )
		{
			note_peak_freqs[i] = 255;
			note_peak_amps[i] = 0;
			note_peak_amps2[i] = 0;
		}
	}

	//We now have notes!!!
#if 0
	for( i = 0; i < MAXNOTES; i++ )
	{
		if( note_peak_freqs[i] == 255 ) continue;
		printf( "(%3d %4d %4d) ", note_peak_freqs[i], note_peak_amps[i], note_peak_amps2[i] );
	}
	printf( "\n") ;
#endif

#if 0
	for( i = 0; i < FIXBPERO; i++ )
	{
		printf( "%4d ", folded_bins[i] );
	}
	printf( "\n" );
#endif


}


//Copyright 2015 <>< Charles Lohr under the ColorChord License.

#ifndef _EMBEDDEDOUT_H
#define _EMBEDDEDOUT_H


//Controls brightness
#ifndef NOTE_FINAL_AMP
#define NOTE_FINAL_AMP  12   //Number from 0...255
#endif

//Controls, basically, the minimum size of the splotches.
#ifndef NERF_NOTE_PORP
#define NERF_NOTE_PORP 15 //value from 0 to 255
#endif

#ifndef NUM_LIN_LEDS
#define NUM_LIN_LEDS 32
#endif

#ifndef USE_NUM_LIN_LEDS
#define USE_NUM_LIN_LEDS NUM_LIN_LEDS
#endif


#ifndef LIN_WRAPAROUND
//Whether the output lights wrap around.
//(Can't easily run on embedded systems)
#define LIN_WRAPAROUND 0 
#endif

#ifndef SORT_NOTES
#define SORT_NOTES 0     //Whether the notes will be sorted. BUGGY Don't use.
#endif

extern uint8_t ledArray[];
extern uint8_t ledOut[]; //[NUM_LIN_LEDS*3]
extern uint8_t RootNoteOffset; //Set to define what the root note is.  0 = A.

//For doing the nice linear strip LED updates
void UpdateLinearLEDs();

//For making all the LEDs the same and quickest.  Good for solo instruments?
void UpdateAllSameLEDs();

uint32_t ECCtoHEX( uint8_t note, uint8_t sat, uint8_t val );
uint32_t EHSVtoHEX( uint8_t hue, uint8_t sat, uint8_t val ); //hue = 0..255 // TODO: TEST ME!!!


#endif

//Copyright 2015 <>< Charles Lohr under the ColorChord License.


//uint8_t ledArray[NUM_LIN_LEDS]; //Points to which notes correspond to these LEDs
uint8_t ledOut[NUM_LIN_LEDS*3];

uint16_t ledSpin;
uint16_t ledAmpOut[NUM_LIN_LEDS];
uint8_t ledFreqOut[NUM_LIN_LEDS];
uint8_t ledFreqOutOld[NUM_LIN_LEDS];

uint8_t RootNoteOffset;

void UpdateLinearLEDs()
{
	//Source material:
	/*
		extern uint8_t  note_peak_freqs[];
		extern uint16_t note_peak_amps[];  //[MAXNOTES] 
		extern uint16_t note_peak_amps2[];  //[MAXNOTES]  (Responds quicker)
		extern uint8_t  note_jumped_to[]; //[MAXNOTES] When a note combines into another one,
	*/

	//Goal: Make splotches of light that are porportional to the strength of notes.
	//Color them according to value in note_peak_amps2.

	uint8_t i;
	int8_t k;
	uint16_t j, l;
	uint32_t total_size_all_notes = 0;
	int32_t porpamps[MAXNOTES]; //LEDs for each corresponding note.
	uint8_t sorted_note_map[MAXNOTES]; //mapping from which note into the array of notes from the rest of the system.
	uint8_t sorted_map_count = 0;
	uint32_t note_nerf_a = 0;

	for( i = 0; i < MAXNOTES; i++ )
	{
		if( note_peak_freqs[i] == 255 ) continue;
		note_nerf_a += note_peak_amps[i];
	}

	note_nerf_a = ((note_nerf_a * NERF_NOTE_PORP)>>8);


	for( i = 0; i < MAXNOTES; i++ )
	{
		uint16_t ist = note_peak_amps[i];
		uint8_t nff = note_peak_freqs[i];
		if( nff == 255 )
		{
			continue;
		}
		if( ist < note_nerf_a )
		{
			continue;
		}

#if SORT_NOTES
		for( j = 0; j < sorted_map_count; j++ )
		{
			if( note_peak_freqs[ sorted_note_map[j] ] > nff )
			{
				break; // so j is correct place to insert
			}
		}
		for( k = sorted_map_count; k > j; k-- ) // make room
		{
			sorted_note_map[k] = sorted_note_map[k-1];
		}
		sorted_note_map[j] = i; // insert in correct place
#else
		sorted_note_map[sorted_map_count] = i; // insert at end
#endif
		sorted_map_count++;
	}

#if 0
	for( i = 0; i < sorted_map_count; i++ )
	{
		printf( "%d: %d: %d /", sorted_note_map[i],  note_peak_freqs[sorted_note_map[i]], note_peak_amps[sorted_note_map[i]] );
	}
	printf( "\n" );
#endif

	uint16_t local_peak_amps[MAXNOTES];
	uint16_t local_peak_amps2[MAXNOTES];
	uint8_t  local_peak_freq[MAXNOTES];

	//Make a copy of all of the variables into local ones so we don't have to keep double-dereferencing.
	for( i = 0; i < sorted_map_count; i++ )
	{
		//printf( "%5d ", local_peak_amps[i] );
		local_peak_amps[i] = note_peak_amps[sorted_note_map[i]] - note_nerf_a;
		local_peak_amps2[i] = note_peak_amps2[sorted_note_map[i]];
		local_peak_freq[i] = note_peak_freqs[sorted_note_map[i]];
//		printf( "%5d ", local_peak_amps[i] );
	}
//	printf( "\n" );

	for( i = 0; i < sorted_map_count; i++ )
	{
		uint16_t ist = local_peak_amps[i];
		porpamps[i] = 0;
		total_size_all_notes += local_peak_amps[i];
	}

	if( total_size_all_notes == 0 )
	{
		for( j = 0; j < USE_NUM_LIN_LEDS * 3; j++ )
		{
			ledOut[j] = 0;
		}
		return;
	}

	uint32_t porportional = (uint32_t)(USE_NUM_LIN_LEDS<<16)/((uint32_t)total_size_all_notes);

	uint16_t total_accounted_leds = 0;

	for( i = 0; i < sorted_map_count; i++ )
	{
		porpamps[i] = (local_peak_amps[i] * porportional) >> 16;
		total_accounted_leds += porpamps[i];
	}

	int16_t total_unaccounted_leds = USE_NUM_LIN_LEDS - total_accounted_leds;

	int addedlast = 1;
	do
	{
		for( i = 0; i < sorted_map_count && total_unaccounted_leds; i++ )
		{
			porpamps[i]++; total_unaccounted_leds--;
			addedlast = 1;
		}
	} while( addedlast && total_unaccounted_leds );

	//Put the frequencies on a ring.
	j = 0;
	for( i = 0; i < sorted_map_count; i++ )
	{
		while( porpamps[i] > 0 )
		{
			ledFreqOut[j] = local_peak_freq[i];
			ledAmpOut[j] = (local_peak_amps2[i]*NOTE_FINAL_AMP)>>8;
			j++;
			porpamps[i]--;
		}
	}

	//This part totally can't run on an embedded system.
#if LIN_WRAPAROUND
	uint16_t midx = 0;
	uint32_t mqty = 100000000;
	for( j = 0; j < USE_NUM_LIN_LEDS; j++ )
	{
		uint32_t dqty;
		uint16_t localj;

		dqty = 0;
		localj = j;
		for( l = 0; l < USE_NUM_LIN_LEDS; l++ )
		{
			int32_t d = (int32_t)ledFreqOut[localj] - (int32_t)ledFreqOutOld[l];
			if( d < 0 ) d *= -1;
			if( d > (NOTERANGE>>1) ) { d = NOTERANGE - d + 1; }
			dqty += ( d * d );

			localj++;
			if( localj == USE_NUM_LIN_LEDS ) localj = 0;
		}

		if( dqty < mqty )
		{
			mqty = dqty;
			midx = j;
		}
	}

	ledSpin = midx;

#else
	ledSpin = 0;
#endif

	j = ledSpin;
	for( l = 0; l < USE_NUM_LIN_LEDS; l++, j++ )
	{
		if( j >= USE_NUM_LIN_LEDS ) j = 0;
		ledFreqOutOld[l] = ledFreqOut[j];

		uint16_t amp = ledAmpOut[j];
		if( amp > 255 ) amp = 255;
		uint32_t color = ECCtoHEX( (ledFreqOut[j]+RootNoteOffset)%NOTERANGE, 255, amp );
		ledOut[l*3+0] = ( color >> 0 ) & 0xff;
		ledOut[l*3+1] = ( color >> 8 ) & 0xff;
		ledOut[l*3+2] = ( color >>16 ) & 0xff;
	}
/*	j = ledSpin;
	for( i = 0; i < sorted_map_count; i++ )
	{
		while( porpamps[i] > 0 )
		{
			uint16_t amp = ((uint32_t)local_peak_amps2[i] * NOTE_FINAL_AMP) >> 8;
			if( amp > 255 ) amp = 255;
			uint32_t color = ECCtoHEX( local_peak_freq[i], 255, amp );
			ledOut[j*3+0] = ( color >> 0 ) & 0xff;
			ledOut[j*3+1] = ( color >> 8 ) & 0xff;
			ledOut[j*3+2] = ( color >>16 ) & 0xff;

			j++;
			if( j == USE_NUM_LIN_LEDS ) j = 0;
			porpamps[i]--;
		}
	}*/

	//Now, we use porpamps to march through the LEDs, coloring them.
/*	j = 0;
	for( i = 0; i < sorted_map_count; i++ )
	{
		while( porpamps[i] > 0 )
		{
			uint16_t amp = ((uint32_t)local_peak_amps2[i] * NOTE_FINAL_AMP) >> 8;
			if( amp > 255 ) amp = 255;
			uint32_t color = ECCtoHEX( local_peak_freq[i], 255, amp );
			ledOut[j*3+0] = ( color >> 0 ) & 0xff;
			ledOut[j*3+1] = ( color >> 8 ) & 0xff;
			ledOut[j*3+2] = ( color >>16 ) & 0xff;

			j++;
			porpamps[i]--;
		}
	}*/
}




void UpdateAllSameLEDs()
{
	int i;
	uint8_t freq = 0;
	uint16_t amp = 0;

	for( i = 0; i < MAXNOTES; i++ )
	{
		uint16_t ist = note_peak_amps2[i];
		uint8_t ifrq = note_peak_freqs[i];
		if( ist > amp && ifrq != 255 )
		{
			freq = ifrq;
			amp = ist;
		}
	}

	amp = (((uint32_t)(amp))*NOTE_FINAL_AMP)>>10;

	if( amp > 255 ) amp = 255;
	uint32_t color = ECCtoHEX( (freq+RootNoteOffset)%NOTERANGE, 255, amp );

	for( i = 0; i < USE_NUM_LIN_LEDS; i++ )
	{
		ledOut[i*3+0] = ( color >> 0 ) & 0xff;
		ledOut[i*3+1] = ( color >> 8 ) & 0xff;
		ledOut[i*3+2] = ( color >>16 ) & 0xff;
	}
}






uint32_t ECCtoHEX( uint8_t note, uint8_t sat, uint8_t val )
{
	uint16_t hue = 0;
	uint16_t third = 65535/3;
	uint16_t scalednote = note;
	uint32_t renote = ((uint32_t)note * 65536) / NOTERANGE;

	//Note is expected to be a vale from 0..(NOTERANGE-1)
	//renote goes from 0 to the next one under 65536.


	if( renote < third )
	{
		//Yellow to Red.
		hue = (third - renote) >> 1;
	}
	else if( renote < (third<<1) )
	{
		//Red to Blue
		hue = (third-renote);
	}
	else
	{
		//hue = ((((65535-renote)>>8) * (uint32_t)(third>>8)) >> 1) + (third<<1);
		hue = (uint16_t)(((uint32_t)(65536-renote)<<16) / (third<<1)) + (third>>1); // ((((65535-renote)>>8) * (uint32_t)(third>>8)) >> 1) + (third<<1);
	}
	hue >>= 8;

	return EHSVtoHEX( hue, sat, val );
}

uint32_t EHSVtoHEX( uint8_t hue, uint8_t sat, uint8_t val )
{
	#define SIXTH1 43
	#define SIXTH2 85
	#define SIXTH3 128
	#define SIXTH4 171
	#define SIXTH5 213

	uint16_t _or = 0, og = 0, ob = 0;

	hue -= SIXTH1; //Off by 60 degrees.

	//TODO: There are colors that overlap here, consider 
	//tweaking this to make the best use of the colorspace.

	if( hue < SIXTH1 ) //Ok: Yellow->Red.
	{
		_or = 255;
		og = 255 - ((uint16_t)hue * 255) / (SIXTH1);
	}
	else if( hue < SIXTH2 ) //Ok: Red->Purple
	{
		_or = 255;
		ob = (uint16_t)hue*255 / SIXTH1 - 255;
	}
	else if( hue < SIXTH3 )  //Ok: Purple->Blue
	{
		ob = 255;
		_or = ((SIXTH3-hue) * 255) / (SIXTH1);
	}
	else if( hue < SIXTH4 ) //Ok: Blue->Cyan
	{
		ob = 255;
		og = (hue - SIXTH3)*255 / SIXTH1;
	}
	else if( hue < SIXTH5 ) //Ok: Cyan->Green.
	{
		og = 255;
		ob = ((SIXTH5-hue)*255) / SIXTH1;
	}
	else //Green->Yellow
	{
		og = 255;
		_or = (hue - SIXTH5) * 255 / SIXTH1;
	}

	uint16_t rv = val;
	if( rv > 128 ) rv++;
	uint16_t rs = sat;
	if( rs > 128 ) rs++;

	//_or, og, ob range from 0...255 now.
	//Need to apply saturation and value.

	_or = (_or * val)>>8;
	og = (og * val)>>8;
	ob = (ob * val)>>8;

	//_or..OB == 0..65025
	_or = _or * rs + 255 * (256-rs);
	og = og * rs + 255 * (256-rs);
	ob = ob * rs + 255 * (256-rs);
//printf( "__%d %d %d =-> %d\n", _or, og, ob, rs );

	_or >>= 8;
	og >>= 8;
	ob >>= 8;

	return _or | (og<<8) | ((uint32_t)ob<<16);
}

