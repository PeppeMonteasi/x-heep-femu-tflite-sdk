/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#pragma message "hello_world_test.cc"
extern "C" {
  #include "lenet5_test.h"
  #include <math.h>
  #include <stdio.h>
  #include "core_v_mini_mcu.h"
}

#include "models/lenet5_input.h"
//#include "models/lenet5.h"
#include "models/lenet5_stolen.h"
#include "tensorflow/lite/core/c/common.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_profiler.h"
#include "tensorflow/lite/micro/recording_micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace {
constexpr int kTensorArenaSize = 0x4000;
uint8_t tensor_arena[kTensorArenaSize];
const tflite::Model* model = nullptr;

using Lenet5OpResolver = tflite::MicroMutableOpResolver<7>;

TfLiteStatus RegisterOps(Lenet5OpResolver& op_resolver) {
  TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
  TF_LITE_ENSURE_STATUS(op_resolver.AddConv2D());
  TF_LITE_ENSURE_STATUS(op_resolver.AddAveragePool2D());
  TF_LITE_ENSURE_STATUS(op_resolver.AddTanh());
  TF_LITE_ENSURE_STATUS(op_resolver.AddReshape());
  TF_LITE_ENSURE_STATUS(op_resolver.AddSoftmax());
  TF_LITE_ENSURE_STATUS(op_resolver.AddLogistic());
  return kTfLiteOk;
}
}  // namespace

TfLiteStatus LoadModel() {
  if (model != nullptr) {
    return kTfLiteOk;
  }
  model = ::tflite::GetModel(tflite_rom);
  TFLITE_CHECK_EQ(model->version(), TFLITE_SCHEMA_VERSION);
  return kTfLiteOk;
}

TfLiteStatus Infer(const char *data, size_t len, int8_t **out, size_t *out_len) {
  if (model == nullptr) {
    return kTfLiteError;
  }
  Lenet5OpResolver op_resolver;
  TF_LITE_ENSURE_STATUS(RegisterOps(op_resolver));

  tflite::MicroInterpreter interpreter(model, op_resolver, tensor_arena,
                                       kTensorArenaSize);
  TF_LITE_ENSURE_STATUS(interpreter.AllocateTensors());
  TfLiteTensor* input = interpreter.input(0);
  TFLITE_CHECK_NE(input, nullptr);
  TfLiteTensor* output = interpreter.output(0);
  TFLITE_CHECK_NE(output, nullptr);
  memcpy(input->data.int8, data, len);
  TF_LITE_ENSURE_STATUS(interpreter.Invoke());
  *out = output->data.int8;
  *out_len = output->bytes;

  return kTfLiteOk;
}

extern "C" int init_tflite() {
  tflite::InitializeTarget();
  TF_LITE_ENSURE_STATUS(LoadModel());
  return kTfLiteOk;
}

extern "C" int infer(const char *data, size_t len, int8_t **out, size_t *out_len) {
  return Infer(data, len, out, out_len);
}


// Calcolo della loss 
int calculate_loss(const int8_t *label, const int8_t *output, size_t len) {
    int loss = 0;
    for (size_t i = 0; i < len; i++) {
        int diff = label[i] - output[i];
        loss += diff * diff;
    }
    return loss;
}

// Calcolo del gradiente 
void calculate_gradient(const int8_t *original_image, const int8_t *label, int8_t *gradient, size_t len) {
    int8_t perturbed_image[len];
    int8_t *output;
    size_t output_len;

    Infer((const char *)original_image, len, &output, &output_len);
    int original_loss = calculate_loss(label, output, output_len);


    memcpy(perturbed_image, original_image, len);
    for (size_t i = 0; i < len; i++) {
        perturbed_image[i] += 1; 
        Infer((const char *)perturbed_image, len, &output, &output_len);
        int new_loss = calculate_loss(label, output, output_len);
        if(new_loss - original_loss > 0)
          gradient[i] = 1;
        else
          gradient[i] = -1;
    }
}



void fgsm_attack(const int8_t *original_image, const int8_t *label, int8_t *adversarial_image, int epsilon) {

  int8_t adv_image[lenet_input_data_size];
  memcpy(adv_image, original_image, lenet_input_data_size);

  //inference
  int8_t *output;
  size_t output_len;
  Infer((const char *)adv_image, lenet_input_data_size, &output, &output_len);

  int8_t gradient[lenet_input_data_size];
  memset(gradient, 0, lenet_input_data_size);

  // Calcolo del gradiente facendo la derivata della loss rispetto all'immagine
   calculate_gradient(original_image, label, gradient, lenet_input_data_size);


  // Create the adversarial image
  for (size_t i = 0; i < lenet_input_data_size; i++) {
    adversarial_image[i] = original_image[i] + ((epsilon) * gradient[i]);
    if (adversarial_image[i] < 0) adversarial_image[i] = 0;
    if (adversarial_image[i] > 127) adversarial_image[i] = 127;
  }
}


void fgsm_attack_with_shift(const int8_t *original_image, const int8_t *label, int8_t *adversarial_image, int epsilon) {


    int8_t shifted_image[lenet_input_data_size];
    int8_t gradient[lenet_input_data_size];
    int extra_bits[lenet_input_data_size] = {0};
    int8_t shifted_grad[lenet_input_data_size];
    int extra_bits_grad[lenet_input_data_size] = {0};



    
    calculate_gradient(original_image, label, gradient, lenet_input_data_size);

    for (size_t i = 0; i < lenet_input_data_size; i++) {
        extra_bits[i] = original_image[i]; // 1111 2222
        extra_bits[i] = extra_bits[i]>>(8-4);// 0000 1111
        extra_bits[i] = extra_bits[i]<<(8-4);// 1111 0000

        shifted_image[i] = original_image[i]; // 1111 2222
        shifted_image[i] = shifted_image[i]<<(4);// 2222 0000

        extra_bits_grad[i] = gradient[i]; // 1111 2222
        extra_bits_grad[i] = extra_bits_grad[i]>>(8-4); // 0000 1111
        extra_bits_grad[i] = extra_bits_grad[i]<<(8-4); // 1111 0000

        shifted_grad[i] = gradient[i]; // 1111 2222
        shifted_grad[i] = shifted_grad[i]<<(4); // 2222 0000



        adversarial_image[i] = extra_bits[i]; // 1111 0000 + 1111 0000 
        adversarial_image[i] = adversarial_image[i] + (shifted_image[i] + ((epsilon<<4) * shifted_grad[i]))>>4; // 1111 0000 +  0000 4444

        if (adversarial_image[i] < 0) adversarial_image[i] = 0;
        if (adversarial_image[i] > 127) adversarial_image[i] = 127;
        
    }
} 
