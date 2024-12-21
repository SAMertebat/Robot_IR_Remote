#include "stm32g0xx.h"                
#include "mb-crc.h" 
//------------------------------------------------------------//
volatile uint8_t data[6];
volatile uint8_t receive_complete=0;
volatile uint8_t timeout=0;
//------------------------------------------------------------//
/*******************|Define|***********************************/
//------------------------------------------------------------//
/*******************|Motors|***********************************/
//forward->motor_1A && motor_2A
//reverse->motor_1B && motor_2B
//------------------------------------------------------------//
//motor_1-> (A->A5) & (B->A6)
#define   motor_1_A_on     GPIOA->ODR |= GPIO_ODR_OD5         
#define   motor_1_A_off    GPIOA->ODR &=~GPIO_ODR_OD5

#define   motor_1_B_on     GPIOA->ODR |= GPIO_ODR_OD6    
#define   motor_1_B_off    GPIOA->ODR &=~GPIO_ODR_OD6
//------------------------------------------------------------//
//motor_2-> (A->A8)  & (B->A7)
#define   motor_2_A_on     GPIOA->ODR |= GPIO_ODR_OD8         
#define   motor_2_A_off    GPIOA->ODR &=~GPIO_ODR_OD8

#define   motor_2_B_on     GPIOA->ODR |= GPIO_ODR_OD7    
#define   motor_2_B_off    GPIOA->ODR &=~GPIO_ODR_OD7
//------------------------------------------------------------//
//motor_3-> (A->A11)  & (B->A12)
#define   motor_3_A_on     GPIOA->ODR |= GPIO_ODR_OD11      
#define   motor_3_A_off    GPIOA->ODR &=~GPIO_ODR_OD11

#define   motor_3_B_on     GPIOA->ODR |= GPIO_ODR_OD12     
#define   motor_3_B_off    GPIOA->ODR &=~GPIO_ODR_OD12
//------------------------------------------------------------//
//mosfet-> (1->14)  & (2->13)
#define   mosfet_1_on     GPIOA->ODR |= GPIO_ODR_OD15      
#define   mosfet_1_off    GPIOA->ODR &=~GPIO_ODR_OD15

#define   mosfet_2_on     GPIOA->ODR |= GPIO_ODR_OD13     
#define   mosfet_2_off    GPIOA->ODR &=~GPIO_ODR_OD13
//------------------------------------------------------------//
//buzzer
#define   buzzer_on     GPIOA->ODR |= GPIO_ODR_OD4      
#define   buzzer_off    GPIOA->ODR &=~GPIO_ODR_OD4
/*******************|END Motors|*******************************/
//------------------------------------------------------------//
/*******************|btn|**************************************/
#define motor_3_up_cm       (uint8_t)(0x01 << 1)               /*!< 0b00000010 */
#define motor_3_down_cm     (uint8_t)(0x01 << 2)               /*!< 0x00000100 */

#define mosfet_1_cm         (uint8_t)(0x01 << 0)               /*!< 0x00000001 */
#define mosfet_2_cm         (uint8_t)(0x01 << 3)               /*!< 0x00001000 */
/*******************|USART|************************************/
// USART Receiver buffer
#define RX_BUFFER_SIZE 6
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t rx_wr_index=0;
/*******************|END_USART|********************************/                              
//------------------------------------------------------------//
uint8_t get_add(void)
{
   return (uint8_t)(GPIOA->IDR & ( GPIO_IDR_ID0 |
                                   GPIO_IDR_ID1 | 
                                   GPIO_IDR_ID2 |
                                   GPIO_IDR_ID3 ));
}
//------------------------------------------------------------//
void write_rx_buffer(uint8_t data)
{
    rx_buffer[rx_wr_index++]=data;
    if(!(rx_buffer[0]==get_add()))
    {
       rx_wr_index=0;
    }
         
    if(rx_wr_index==6)        
    {
       receive_complete=1;
       rx_wr_index=0;
    }
    if (rx_wr_index == RX_BUFFER_SIZE)
       rx_wr_index=0;                  
}
//------------------------------------------------------------//
void USART1_IRQHandler (void)
{
  if(USART1->ISR&USART_ISR_RXNE_RXFNE)
  {
     write_rx_buffer((uint8_t)USART1->RDR);
  }
}
//------------------------------------------------------------//
void SysTick_Handler (void)
{
    timeout++;
    if(timeout==4)
    {
       motor_1_B_off;
       motor_1_A_off;
                
       motor_2_A_off;
       motor_2_B_off;
       
       motor_3_A_off;
       motor_3_B_off;
   
       buzzer_off;
    }
}
//------------------------------------------------------------//
int main()
{
    //------------------------------------------------------------//
    //USART 
    RCC->APBENR2 = RCC_APBENR2_USART1EN ;
    //------------------------------------------------------------//
    RCC->IOPENR  = RCC_IOPENR_GPIOAEN | 
                   RCC_IOPENR_GPIOBEN ;	  
    //------------------------------------------------------------//
    GPIOA->MODER=0x00000000;
    
    GPIOA->MODER |= GPIO_MODER_MODE4_0|
                   GPIO_MODER_MODE5_0|
                   GPIO_MODER_MODE6_0|
                   GPIO_MODER_MODE7_0|
                   GPIO_MODER_MODE8_0|
                   GPIO_MODER_MODE11_0|
                   GPIO_MODER_MODE12_0|
                   GPIO_MODER_MODE13_0|
                   GPIO_MODER_MODE15_0;
   
    //Add->A0~A3->in(PD)
    GPIOA->PUPDR=0x00000000;
    GPIOA->PUPDR  |=  GPIO_PUPDR_PUPD0_1 |
                     GPIO_PUPDR_PUPD1_1 |
                     GPIO_PUPDR_PUPD2_1 |
                     GPIO_PUPDR_PUPD3_1 ;
                    
    //IR_in_rx-> PB7 AF0
    GPIOB->AFR[0] = 0x00000000;
    
    GPIOB->MODER   = 0x00000000;
    GPIOB->MODER  |= GPIO_MODER_MODE6_1|GPIO_MODER_MODE7_1;
    GPIOB->OSPEEDR = GPIO_OSPEEDR_OSPEED6_1 ; 
   
    GPIOB->PUPDR = GPIO_PUPDR_PUPD7_0 ;

    //USART            
    USART1->BRR=0x2EE0;
    USART1->CR1=  USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_RXNEIE_RXFNEIE|
                  USART_CR1_UE ;						
	 NVIC_EnableIRQ(USART1_IRQn);
       
    //62.5ms
    SysTick_Config(1000000);
   while(1)
   {
      //add->0
      //wsda('w','s','d','a')->1
      //btn->2
      //buzzer->3
      //crc->4
      //crc->5
      if(receive_complete)
      {
         if(rx_buffer[0]==get_add())
         {
            //check crc
            if(mb_crc_check(rx_buffer,6)==MB_CRC_OK)
            {
               timeout=0;
               //------------------------------------------------------------//
               //motor 1&2
               if(rx_buffer[1]=='w')
               {
                  //forward-> (motor_1A & motor_2A on) & (motor_1B & motor_2B off)
                  motor_1_A_on;
                  motor_1_B_off;
                
                  motor_2_A_on;
                  motor_2_B_off;
               }
               else if(rx_buffer[1]=='s')
               {  
                  //reverse-> (motor_1B & motor_2B on) & (motor_1A & motor_2A off)
                  motor_1_B_on;
                  motor_1_A_off;
                
                  motor_2_B_on;
                  motor_2_A_off;
               }
               else if(rx_buffer[1]=='a')
               {  
                  //left-> (motor_1A & motor_2B on) & (motor_1B & motor_2A off)
                  motor_1_A_on;
                  motor_1_B_off;
                
                  motor_2_B_on;
                  motor_2_A_off;
               }
               else if(rx_buffer[1]=='d')
               {  
                  //reverse-> (motor_1B & motor_2A on) & (motor_1A & motor_2B off)
                  motor_1_B_on;
                  motor_1_A_off;
                
                  motor_2_A_on;
                  motor_2_B_off;
               }
               else if(rx_buffer[1]=='e')
               {  
                  //off all
                  motor_1_B_off;
                  motor_1_A_off;
                
                  motor_2_A_off;
                  motor_2_B_off;
               }
               //------------------------------------------------------------//
               //motor3
               if(rx_buffer[2] & motor_3_up_cm)
               {
                  motor_3_A_on;
                  motor_3_B_off;
               }
               else if(rx_buffer[2] & motor_3_down_cm)
               {
                  motor_3_A_off;
                  motor_3_B_on;
               }
               else 
               {
                 //off all
                  motor_3_A_off;
                  motor_3_B_off;
               } 
               //------------------------------------------------------------//
               //mosfets
               if(rx_buffer[2] & mosfet_1_cm)
               {
                  mosfet_1_on;
               }
               else if(!(rx_buffer[2] & mosfet_1_cm))
               {
                 mosfet_1_off;
               }

               if(rx_buffer[2] & mosfet_2_cm)
               {
                  mosfet_2_on;

               }
               else if(!(rx_buffer[2] & mosfet_2_cm))
               {
                  mosfet_2_off;
 
               }
               //------------------------------------------------------------//
               //buzzer
               if(rx_buffer[3]==0xFF)
               {
                  buzzer_on;

               }
               else if(rx_buffer[3]==0x00)
               {
                  buzzer_off;
               }
            }
         }
         
         receive_complete=0;
      }
   }
}

