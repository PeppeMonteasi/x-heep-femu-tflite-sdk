#ifndef LENET_5_TEST_H
#define LENET_5_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

int init_tflite();
int infer(const char *data, size_t len, int8_t **out, size_t *out_len);
void fgsm_attack(const int8_t *original_image, const int8_t *label, int8_t *adversarial_image, int epsilon);

#ifdef __cplusplus
}
#endif

#endif
