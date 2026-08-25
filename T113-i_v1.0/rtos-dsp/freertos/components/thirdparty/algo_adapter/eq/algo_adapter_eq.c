/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define AW_RPAF_DEBUG
#include "../../algo_common/algo_adapter.h"
#include <aw_rpaf/substream.h>
#include <aw_rpaf/component.h>
#include <aw_rpaf/common.h>
#include "include/aweq.h"

#define offset(type, v) (&(((type *)0)->v))

typedef struct
{
	struct snd_soc_dsp_native_component *native_component;
	struct snd_soc_dsp_pcm_params *pcm_params;

	void **equalizer;
	void **equalizer_buf; // each equalizer can only process 2 channel data;
	int num_equalizer;

	int frame_size;	   // source data frame size(byte)
	int eq_frame_size; // equalizer frame size(byte), 2 channels
					   // save here in order to reduce multiplication
} eq_handle_t;

int algo_adapter_eq_prepare(void *native_component, void *data)
{
	struct snd_soc_dsp_algo_params_transfer *src_algo_params = data;
	struct snd_soc_dsp_native_component *native_component_l = (struct snd_soc_dsp_native_component *)native_component;
	struct snd_soc_dsp_algo_params *dst_algo_params = native_component_l->private_data[SND_DSP_COMPONENT_EQ];
	eq_prms_transfer_t *src_eq_params = src_algo_params->data;

	awrpaf_debug("begin\n");
	if (!dst_algo_params) {
		dst_algo_params = rpaf_malloc(sizeof(struct snd_soc_dsp_algo_params));
		if (!dst_algo_params) {
			awrpaf_err("failed to malloc native_component private_data");
			return -1;
		}

		eq_prms_t *eq_prms = rpaf_malloc(sizeof(eq_prms_t));
		if (!eq_prms) {
			awrpaf_err("failed to malloc eq_prms_t");
			return -1;
		}

		eq_prms->biq_num = src_eq_params->biq_num;
		eq_prms->core_prms = rpaf_malloc(sizeof(eq_core_prms_t) * eq_prms->biq_num);
		if (!eq_prms->core_prms) {
			awrpaf_err("failed to malloc eq core_prms");
			return -1;
		}
		dst_algo_params->data = eq_prms;
		native_component_l->private_data[SND_DSP_COMPONENT_EQ] = dst_algo_params;
	}

	eq_prms_t *dst_eq_params = (eq_prms_t *)dst_algo_params->data;
	if (dst_eq_params->biq_num != src_eq_params->biq_num) {
		rpaf_free(dst_eq_params->core_prms);
		dst_eq_params->biq_num = src_eq_params->biq_num;
		dst_eq_params->core_prms = rpaf_malloc(sizeof(eq_core_prms_t) * dst_eq_params->biq_num);
		if (!dst_eq_params->core_prms) {
			awrpaf_err("failed to malloc eq core_prms");
			return -1;
		}
	}
	int i = 0;
	int transfer_size = src_algo_params->transfer_size - (int)offset(struct snd_soc_dsp_algo_params_transfer, data);
	int size = transfer_size / sizeof(src_eq_params->core_prms[0]);
	while (i < size) {
		eq_core_prms_transfer_t src = src_eq_params->core_prms[i];
		eq_core_prms_t *dst = dst_eq_params->core_prms + src.index;
		dst->G = src.G;
		dst->fc = src.fc;
		dst->Q = src.Q;
		dst->type = src.type;
		i++;
	}
	dst_algo_params->is_changed = src_algo_params->is_changed;
	awrpaf_debug("end\n");
	return 0;
}

int algo_adapter_eq_create(void **handle, void *native_component)
{
	struct snd_dsp_hal_substream *hal_substream = container_of(native_component, struct snd_dsp_hal_substream, native_component);
	struct snd_soc_dsp_substream *soc_substream = hal_substream->soc_substream;
	struct snd_soc_dsp_pcm_params *pcm_params = &soc_substream->params;
	struct snd_soc_dsp_algo_params *algo_params;
	eq_prms_t *eq_params = NULL;

	awrpaf_debug("begin\n");
	eq_handle_t *eq_handle = rpaf_malloc(sizeof(*eq_handle));
	if (!eq_handle) {
		awrpaf_err("create eq_handle failed\n");
		return -1;
	}
	eq_handle->native_component = (struct snd_soc_dsp_native_component *)native_component;

	algo_params = (struct snd_soc_dsp_algo_params *)eq_handle->native_component->private_data[SND_DSP_COMPONENT_EQ];
	algo_params->is_changed = false;
	eq_params = (eq_prms_t *)algo_params->data;

	if (!algo_params || !eq_params || !eq_params->biq_num) {
		awrpaf_info("not need to create equalizer\n");
		goto EXIT;
	}

	eq_params->chan = pcm_params->channels;
	eq_params->sampling_rate = pcm_params->rate;
	int bit_width = 0;
	switch (pcm_params->format) {
	case SND_PCM_FORMAT_S24:
	case SND_PCM_FORMAT_U24:
	case SND_PCM_FORMAT_S32:
	case SND_PCM_FORMAT_U32:
		bit_width = 32;
		break;
	case SND_PCM_FORMAT_S16:
	case SND_PCM_FORMAT_U16:
	default:
		bit_width = 16;
		break;
	}

	eq_handle->num_equalizer = 1;
	while (pcm_params->channels - eq_handle->num_equalizer * 2 > 2) {
		eq_handle->num_equalizer++;
	}

	eq_handle->equalizer = rpaf_malloc(sizeof(*eq_handle->equalizer) * eq_handle->num_equalizer);
	eq_handle->equalizer_buf = rpaf_malloc(sizeof(*eq_handle->equalizer_buf) * eq_handle->num_equalizer);
	int i = 0;
	while (i < eq_handle->num_equalizer) {
		eq_prms_t params;
		memcpy(&params, eq_params, sizeof(params));
		params.chan = (i == eq_handle->num_equalizer - 1) ? params.chan - (i * 2) : 2;

		awrpaf_debug("equalizer(%d):biq_num = %d, sampling_rate = %d, chan = %d\n", i, params.biq_num, params.sampling_rate, params.chan);
		int j = 0;
		for (j = 0; j < params.biq_num; j++) {
			eq_core_prms_t *tmp = params.core_prms + j;
			awrpaf_debug("j = %d, G = %d, fc = %d, Q = %f, type = %d\n", j, tmp->G, tmp->fc, tmp->Q, tmp->type);
		}

		void **equalizer = eq_handle->equalizer + i;
		*equalizer = eq_create(&params);
		void **equalizer_buf = eq_handle->equalizer_buf + i;
		*equalizer_buf = rpaf_malloc(pcm_params->period_size * params.chan * (bit_width >> 3));

		i++;
		awrpaf_info("create %d equalizer = %p\n", i, *equalizer);
	}

	eq_handle->frame_size = (bit_width * pcm_params->channels) >> 3;
	eq_handle->eq_frame_size = (bit_width * 2) >> 3;
	eq_handle->pcm_params = pcm_params;
EXIT:
	*handle = eq_handle;
	awrpaf_debug("end\n");
	return 0;
}

int algo_adapter_eq_process(void *handle, void *input_buffer,
								 unsigned int *const input_size, void *output_buffer,
								 unsigned int *const output_size)
{
	awrpaf_print("begin\n");
	if (!handle) {
		goto ERROR_EXIT;
	}
	eq_handle_t *eq_handle = (eq_handle_t *)handle;
	if (!eq_handle->equalizer || !eq_handle->equalizer_buf) {
		goto ERROR_EXIT;
	}

	struct snd_soc_dsp_algo_params *algo_params = (struct snd_soc_dsp_algo_params *)eq_handle->native_component->private_data[SND_DSP_COMPONENT_EQ];
	if (algo_params->is_changed) {
		eq_prms_t *eq_params = (eq_prms_t *)algo_params->data;
		eq_params->chan = eq_handle->pcm_params->channels;
		eq_params->sampling_rate = eq_handle->pcm_params->rate;
		int i = 0;
		while (i < eq_handle->num_equalizer) {
			if (*(eq_handle->equalizer + i)) {
				eq_destroy(*(eq_handle->equalizer + i));
			}

			eq_prms_t params;
			memcpy(&params, eq_params, sizeof(params));
			params.chan = (i == eq_handle->num_equalizer - 1) ? params.chan - (i * 2) : 2;

			awrpaf_debug("equalizer(%d):biq_num = %d, sampling_rate = %d, chan = %d\n", i, params.biq_num, params.sampling_rate, params.chan);
			int j = 0;
			for (j = 0; j < params.biq_num; j++) {
				eq_core_prms_t *tmp = params.core_prms + j;
				awrpaf_debug("j = %d, G = %d, fc = %d, Q = %f, type = %d\n", j, tmp->G, tmp->fc, tmp->Q, tmp->type);
			}

			void **equalizer = eq_handle->equalizer + i;
			*equalizer = eq_create(&params);

			i++;
			awrpaf_info("release %d equalizer, and recreate %d equalizer = %p， while eq params is changed\n", i, i, *equalizer);
		}

		algo_params->is_changed = false;
	}

	/* divide the input buffer */
	int i, j;
#if 1
	i = 0;
	while (i < eq_handle->num_equalizer) {
		int frame_size = eq_handle->eq_frame_size;
		if (i == eq_handle->num_equalizer - 1 && eq_handle->pcm_params->channels % 2 == 1) {
			frame_size = eq_handle->eq_frame_size >> 1;
		}
		j = 0;
		while (j < eq_handle->pcm_params->period_size) {
			void *dst = *(eq_handle->equalizer_buf + i) + eq_handle->eq_frame_size * j;
			void *src = input_buffer + eq_handle->frame_size * j + eq_handle->eq_frame_size * i;
			memcpy(dst, src, eq_handle->eq_frame_size);
			j++;
		}
		i++;
	}
#endif

	/* process eq */
	i = 0;
	while (i < eq_handle->num_equalizer) {
		void *equalizer = *(eq_handle->equalizer + i);
		void *buffer = *(eq_handle->equalizer_buf + i);
		int size = sizeof(buffer);
		eq_process(equalizer, buffer, size);
		i++;
	}

#if 1
	/* merge the output buffer */
	i = 0;
	while (i < eq_handle->num_equalizer) {
		int frame_size = eq_handle->eq_frame_size;
		if (i == eq_handle->num_equalizer - 1 && eq_handle->pcm_params->channels % 2 == 1) {
			frame_size = eq_handle->eq_frame_size >> 1;
		}
		j = 0;
		while (j < eq_handle->pcm_params->period_size) {
			void *dst = output_buffer + eq_handle->frame_size * j + eq_handle->eq_frame_size * i;
			void *src = *(eq_handle->equalizer_buf + i) + eq_handle->eq_frame_size * j;
			memcpy(dst, src, eq_handle->eq_frame_size);
			j++;
		}
		i++;
	}
#endif

	*output_size = *input_size;
	awrpaf_print("input_buffer:%p, size;%u, output_buffer:%p, size:%u\n",
				 input_buffer, *input_size, output_buffer, *output_size);
	return 0;
ERROR_EXIT:
	*output_size = *input_size;
	memcpy(output_buffer, input_buffer, *input_size);
	awrpaf_err("not need to used EQ, because handle is null or equalizer is null or buffer is null\n");
	return -1;
}

int algo_adapter_eq_release(void *handle)
{
	awrpaf_debug("begin\n");
	if (!handle) {
		awrpaf_info("the handle is null");
		goto FREE_EQ_HANDLE;
	}
	eq_handle_t *eq_handle = (eq_handle_t *)handle;
	if (!eq_handle->equalizer || !eq_handle->equalizer_buf) {
		awrpaf_info("the equalizer is null");
		goto FREE_EQ_HANDLE;
	}
	int i = 0;
	while (i < eq_handle->num_equalizer) {
		if (*(eq_handle->equalizer + i)) {
			eq_destroy(*(eq_handle->equalizer + i));
		}
		if (*(eq_handle->equalizer_buf + i)) {
			rpaf_free(*(eq_handle->equalizer_buf + i));
		}
		i++;
		awrpaf_info("release %d equalizer\n", i);
	}
	rpaf_free(eq_handle->equalizer);
	rpaf_free(eq_handle->equalizer_buf);
FREE_EQ_HANDLE:
	rpaf_free(handle);
	awrpaf_debug("end\n");
	return 0;
}
