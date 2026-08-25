#ifndef _ALGO_ADAPTER_H_
#define _ALGO_ADAPTER_H_

int algo_adapter_eq_prepare(void *native_component, void *data);
int algo_adapter_eq_create(void **handle, void *native_component);
int algo_adapter_eq_process(void *handle, void *input_buffer,
								 unsigned int *const input_size, void *output_buffer,
								 unsigned int *const output_size);
int algo_adapter_eq_release(void *handle);

int algo_adapter_aec_prepare(void *native_component, void *data);
int algo_adapter_aec_create(void **handle, void *native_component);
int algo_adapter_aec_process(void *handle, void *input_buffer,
								 unsigned int *const input_size, void *output_buffer,
								 unsigned int *const output_size);
int algo_adapter_aec_release(void *handle);

#endif
