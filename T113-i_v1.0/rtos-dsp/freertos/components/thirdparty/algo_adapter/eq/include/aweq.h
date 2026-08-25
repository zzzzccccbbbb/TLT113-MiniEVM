#ifndef _AWEQ_H_
#define _AWEQ_H_
/* EQ filter type definition */
typedef enum
{
	/* low pass shelving filter */
	LOWPASS_SHELVING,
	/* band pass peak filter */
	BANDPASS_PEAK,
	/* high pass shelving filter */
	HIHPASS_SHELVING,
	LOWPASS,
	HIGHPASS
} eq_ftype_t;

/* equalizer parameters */
typedef struct
{
	/* boost/cut gain in dB */
	int G;
	/* cutoff/center frequency in Hz */
	int fc;
	/* quality factor */
	float Q;
	/* filter type */
	eq_ftype_t type;
} eq_core_prms_t;

typedef struct
{
	/*num of items(biquad)*/
	int biq_num;
	/* sampling rate */
	int sampling_rate;
	/* channel num */
	int chan;
	/* eq parameters for generate IIR coeff*/
	eq_core_prms_t *core_prms;
} eq_prms_t;

/**
 * see eq_core_prms_t.
 */
typedef struct
{
	/* the index of *eq_prms_t->core_prms */
	int index;
	int G;
	int fc;
	float Q;
	eq_ftype_t type;
} eq_core_prms_transfer_t;

/**
 * see eq_prms_t
 * this is used for transfering data from
 *      linux user space -> linux kernel space -> dsp.
 * linux kernel space and dsp transfer data by using shared memory, the size of core_prms is limited by
 * msgbox(RPC:Remote Procedure Call),which has defined message buffer size.
 */
typedef struct
{
	int biq_num;
	int sampling_rate;
	int chan;
	/*
	 * different with eq_prms_t.
	 * eq_prms_t is used for eq eq_prms_transfer_t, while user_eq_prms_t is used for transfering,
	 * bucause the msgbox can transfer the pointer only, which is useless in dsp.
	 */
	eq_core_prms_transfer_t core_prms[12];
} eq_prms_transfer_t;

/*
	function eq_create
description:
	use this function to create the equalizer object
prms:
	eq_prms_t: [in], desired frequency response array
returns:
	the equalizer handle
*/
void *eq_create(eq_prms_t *prms);
/*
	function eq_process
description:
	equalizer processing function
prms:
	handle:[in,out] equalzier handle
	x:[in,out],	input signal
	len:[in], input length(in samples)
returns:
	none
*/
void eq_process(void *handle, short *x, int len);
/*
	function eq_destroy
description:
	use this function to destroy the equalizer object
prms:
	handle:[in], equalizer handle
returns:
	none
*/
void eq_destroy(void *handle);

#endif
