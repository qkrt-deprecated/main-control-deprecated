#include "cmsis_os.h"
#include "stdlib.h"

#include "rm_hal_lib.h"
#include "pneu_task.h"

#include "detect_task.h"
#include "uart_device.h"
#include "keyboard.h"
#include "sys.h"


uint8_t usart4_send[4]; //set an arry with 4 #
uint8_t L1=0,L2=0,R1=0,R2=0;
void USART4_pneu_callback(void){
		L1 = usart4_send[0];
		L2 = usart4_send[1];
		R1 = usart4_send[2];
		R2 = usart4_send[3];
}

void pneu_task (const void*argu){
	uart_init(USER_UART3,9600,WORD_LEN_8B,STOP_BITS_1,PARITY_NONE);
	//uart_recv_callback_register(USER_UART3,USARTpneu_callback);
	//uart_receive_start(USER_UART3,usart3_recv,4);

	while(1){
				switch (rc.sw1)
  {
    case RC_UP:{
		L1 = 1;
		L2 = 1;
		} //user custom function
   break;
    
   case RC_MI: {
		L1 = 1;
		L2 = 0;
	 }//user custom function
    break;

    case RC_DN:
    {
		L1 = 0;
		L2 = 0;
    }
    break;
  }
	switch (rc.sw2)
  {
    case RC_UP:{
		R1 = 1;
		R2 = 1;
		} //user custom function
   break;
    
   case RC_MI: {
	 	R1 = 1;
		R2 = 0;
	 }//user custom function
    break;

    case RC_DN:
    {
		 R1 = 0;
		 R2 = 0;
    }
    break;
  }
	osDelay(30);
		USART4_pneu_callback();
		write_uart(USER_UART4,usart4_send,13);
	}
}

