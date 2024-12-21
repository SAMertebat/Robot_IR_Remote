#include "stm32g0xx.h"
#include "mb-crc.h"

//------------------------------------------------------------//
uint8_t data[6];
//------------------------------------------------------------//
#define push_S !(GPIOA->IDR & (GPIO_IDR_ID3))
#define push_A !(GPIOA->IDR & (GPIO_IDR_ID1))
#define push_D !(GPIOA->IDR & (GPIO_IDR_ID2))
#define push_W !(GPIOA->IDR & (GPIO_IDR_ID0))

#define push_btn !(GPIOA->IDR & (GPIO_IDR_ID11 | GPIO_IDR_ID12 | GPIO_IDR_ID13 | GPIO_IDR_ID14))

//------------------------------------------------------------//
#define push_buzzer !(GPIOB->IDR & (GPIO_IDR_ID6))
//------------------------------------------------------------//
static uint8_t get_btn(void)
{
   uint16_t x = 0;

   x = (uint16_t) ~(GPIOA->IDR & (GPIO_IDR_ID11 |
                                  GPIO_IDR_ID12 |
                                  GPIO_IDR_ID13 |
                                  GPIO_IDR_ID14));
   x = x >> 11;
   return (uint8_t)x;
}
//------------------------------------------------------------//
uint8_t get_add(void)
{
   return (uint8_t)(0x0000000F & (GPIOA->IDR & (GPIO_IDR_ID4 |
                                                GPIO_IDR_ID5 |
                                                GPIO_IDR_ID6 |
                                                GPIO_IDR_ID7)) >> 4);
}
//------------------------------------------------------------//
uint8_t get_buzzer(void)
{
   if (push_buzzer)
      return 0xFF;
   else 
      return 0x00;
}

//------------------------------------------------------------//
void send_data(void)
{
   for (int i = 0; i < 6; i++)
   {
      USART1->TDR = data[i];
      // wait for free space in buffer
      while (!(USART1->ISR & USART_ISR_TXE_TXFNF))
         ;
   }
   // wait to transmit complete
   while (!(USART1->ISR & USART_ISR_TC))
      ;
}
//------------------------------------------------------------//
void SysTick_Handler(void)
{

   if (push_S)
   {
      data[0] = get_add();
      data[1] = 's';
      data[2] = get_btn();
      data[3] = get_buzzer();
      mb_crc_add(data, 4);
      send_data();
   }
   else if (push_A)
   {
      data[0] = get_add();
      data[1] = 'a';
      data[2] = get_btn();
      data[3] = get_buzzer();
      mb_crc_add(data, 4);
      send_data();
   }
   else if (push_D)
   {
      data[0] = get_add();
      data[1] = 'd';
      data[2] = get_btn();
      data[3] = get_buzzer();
      mb_crc_add(data, 4);
      send_data();
   }
   else if (push_W)
   {
      data[0] = get_add();
      data[1] = 'w';
      data[2] = get_btn();
      data[3] = get_buzzer();
      mb_crc_add(data, 4);
      send_data();
   }
   else
   {
      data[0] = get_add();
      data[1] = 'e';
      data[2] = get_btn();
      data[3] = get_buzzer();
      mb_crc_add(data, 4);
      send_data();
   }
}
//------------------------------------------------------------//
int main()
{
   //------------------------------------------------------------//
   // USART & Timer & IR enable
   RCC->APBENR2 = RCC_APBENR2_TIM17EN |
                  RCC_APBENR2_USART1EN |
                  RCC_APBENR2_SYSCFGEN;
   //------------------------------------------------------------//
   RCC->IOPENR = RCC_IOPENR_GPIOAEN |
                 RCC_IOPENR_GPIOBEN;
   //------------------------------------------------------------//
   // IR config
   SYSCFG->CFGR1 = SYSCFG_CFGR1_IR_MOD_0 |
                   SYSCFG_CFGR1_I2C_PB9_FMP;
   // SYSCFG_CFGR1_IR_POL
   //------------------------------------------------------------//
   GPIOA->MODER = 0x00000000;
   // btn->A0~A3->in(PU)
   GPIOA->PUPDR = GPIO_PUPDR_PUPD0_0 |
                  GPIO_PUPDR_PUPD1_0 |
                  GPIO_PUPDR_PUPD2_0 |
                  GPIO_PUPDR_PUPD3_0;

   // Add->A4~A7->in(PD)
   GPIOA->PUPDR |= GPIO_PUPDR_PUPD4_1 |
                   GPIO_PUPDR_PUPD5_1 |
                   GPIO_PUPDR_PUPD6_1 |
                   GPIO_PUPDR_PUPD7_1;
   // WSDA->A11~A14->in(PU)
   GPIOA->PUPDR |= GPIO_PUPDR_PUPD11_0 |
                   GPIO_PUPDR_PUPD12_0 |
                   GPIO_PUPDR_PUPD13_0 |
                   GPIO_PUPDR_PUPD14_0;

   // Buzzer B6->in(PU)
   GPIOB->PUPDR = GPIO_PUPDR_PUPD6_0;

   // IR_OUT-> PB9 AF0
   GPIOB->MODER = GPIO_MODER_MODE9_1;
   GPIOB->OSPEEDR = GPIO_OSPEEDR_OSPEED9_1;

   //------------------------------------------------------------//
   //timer17
   TIM17->PSC = 0;

   TIM17->CCR1 = 105 - 1;

   TIM17->CCMR1 = TIM_CCMR1_OC1M_1 |
                  TIM_CCMR1_OC1M_0 ;

   TIM17->BDTR = TIM_BDTR_MOE;

   TIM17->CCER = TIM_CCER_CC1E;

   TIM17->CR1 = TIM_CR1_CEN;
   //------------------------------------------------------------//
   //USART1
   USART1->BRR = 0x2EE0;
   USART1->CR1 = USART_CR1_TE |
                 USART_CR1_RE |
                 USART_CR1_UE;
   //125ms
   SysTick_Config(2000000);
   while (1)
   {
   }
}



