#include <stdio.h>
#include "models/lenet5_input.h"
#include "lenet5_test.h"
#include "scpi/scpi.h"
#include "uart.h"
#include "soc_ctrl.h"
#include "core_v_mini_mcu.h"
#include "mmio.h"

#define ECHO 1

#define SCPI_IDN1 "MANUFACTURE"
#define SCPI_IDN2 "INSTR2013"
#define SCPI_IDN3 NULL
#define SCPI_IDN4 "01-02"

volatile soc_ctrl_t soc_ctrl;
volatile uart_t uart;
volatile scpi_t scpi_context;
volatile int exit_scpi = 0;


/*
scpi_result_t __attribute__((noinline)) InferExample(scpi_t * context) {
  const char *out;
  size_t len;
  const int8_t *data = lenet_input_data;
  int a = infer((const char *) data, lenet_input_data_size, &out, &len);
  if (a == 0) {
    SCPI_ResultArrayInt8(context, (const int8_t *) out, len, SCPI_FORMAT_ASCII);
  } else {
    SCPI_ResultText(context, "Error");
  }
  return SCPI_RES_OK;
}

scpi_result_t __attribute__((noinline)) InferData(scpi_t * context) {
  const char *out;
  size_t out_len;

  const int8_t tflite_input_data[lenet_input_data_size];
  const int8_t *scpi_out;
  size_t scpi_len;
  SCPI_ParamArbitraryBlock(context, &scpi_out, &scpi_len, true);
  printf("Read: %d bytes\r\n", scpi_len);
  memcpy(tflite_input_data, scpi_out, scpi_len);

  int a = infer((const char *) tflite_input_data, lenet_input_data_size, &out, &out_len);
  if (a == 0) {
    SCPI_ResultArrayInt8(context, (const int8_t *) out, out_len, SCPI_FORMAT_ASCII);
  } else {
    SCPI_ResultText(context, "Inference error");
  }
  return SCPI_RES_OK;
}
scpi_result_t __attribute__((noinline)) InferDataAdv(scpi_t * context) {
  const char *out;
  size_t len;
  const int8_t *data = lenet_input_data;
  const int8_t data_adv[lenet_input_data_size];
  int8_t label[10] = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  fgsm_attack(data,&label,&data_adv,1)
  int a = infer((const char *) data_adv, lenet_input_data_size, &out, &len);
  if (a == 0) {
    SCPI_ResultArrayInt8(context, (const int8_t *) out, len, SCPI_FORMAT_ASCII);
  } else {
    SCPI_ResultText(context, "Error");
  }
  return SCPI_RES_OK;
}
*/

#define SCPI_INPUT_BUFFER_LENGTH 2048
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

#define SCPI_ERROR_QUEUE_SIZE 17
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

static int modifier = 0;

mmio_region_t mmio_region_from_adr(uintptr_t address) {
  return (mmio_region_t){
      .base = (volatile void *)address,
  };
}

int main() {
  printf("Initializing peripherals...\r\n");
  soc_ctrl.base_addr = mmio_region_from_adr((uintptr_t)SOC_CTRL_START_ADDRESS);
  printf("Initialized soc_ctrl\r\n");

  init_tflite();
  printf("Initialized TFLite\r\n");


  const char *out;
  size_t len;
  const int8_t *data = lenet_input_data;
  const int8_t data_adv[lenet_input_data_size];
  int8_t label[10] = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  fgsm_attack(data,&label,&data_adv,1);
  int a = infer((const char *) data_adv, lenet_input_data_size, &out, &len);
  if (a == 0) {
    printf("Inference result: ");
    for (size_t i = 0; i < 10; i++) {
      printf("%d ", out[i]);
    } else {
    printf("Inference error");
  }
  //return SCPI_RES_OK;
 
  

  return 0;
}
}
void __attribute__((optimize("O0"))) should_not_happen() {
    printf("This should not happen\r\n");
}