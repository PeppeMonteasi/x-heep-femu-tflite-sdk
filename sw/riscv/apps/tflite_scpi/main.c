#include <stdio.h>
#include "models/lenet5_input.h"
#include "lenet5_test.h"
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

volatile int exit_scpi = 0;





static int modifier = 0;



mmio_region_t mmio_region_from_adr(uintptr_t address) {
  return (mmio_region_t){
      .base = (volatile void *)address,
  };
}

int main() {


  int x = 0;
  printf("this is x %d\n", x);
  printf("Initializing peripherals...\r\n");
  soc_ctrl.base_addr = mmio_region_from_adr((uintptr_t)SOC_CTRL_START_ADDRESS);
  printf("Initialized soc_ctrl\r\n");
  printf("soc_ctrl.base_addr: %p\r\n", soc_ctrl.base_addr);
    uart.base_addr   = mmio_region_from_adr((uintptr_t)UART_START_ADDRESS);
    uart.baudrate    = 115200;
    uart.clk_freq_hz = soc_ctrl_get_frequency(&soc_ctrl);
    if (uart_init(&uart) != kErrorOk) {
        return;
    }
  printf("Initialized UART\r\n");
  printf("uart.base_addr: %p\r\n", uart.base_addr);
  printf("uart.baudrate: %d\r\n", uart.baudrate);
  printf("uart.clk_freq_hz: %d\r\n", uart.clk_freq_hz);


 
  init_tflite();
  printf("Initialized TFLite\r\n");

  printf("infere Example:\n");

  //  InferExample(&scpi_context);

  printf("infere Example Done, no we do adversial inference:\n");

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
    } 
  }
    else {
    printf("Inference error");
  }
  


  //uart_scpi(&scpi_context, &uart);

  return 0;

}

void __attribute__((optimize("O0"))) should_not_happen() {
    printf("This should not happen\r\n");
}