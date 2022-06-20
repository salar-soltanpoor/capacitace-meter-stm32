#include "stm32f10x.h"
#include "stdio.h"

void SysTick_Handler (void);
void Delay_ms (uint32_t dlyTicks);
volatile uint32_t msTicks;
unsigned int value = 0;
int Data=0;

int counter=0;
char string[50];
char string2[50];
int time=0;
double charge_time=0;
double c=0;
double period=1.0/72000000;
int flag=0;
int val_Tx = 0;              /* Globals used for display */
int AD_last=0;
int flag2;
char str[200];
char data;
void ADC_Init (void) 
{
  RCC->APB2ENR |=(1<<2); /* enable periperal clock for GPIOA */
  GPIOC->CRL &= ~0xF0;                 /* set ADC1_IN1 (PA1) as analog input  */
//  /* Setup and initialize ADC converter                                       */
  RCC->APB2ENR |= ( 1UL <<  9);           /* enable periperal clock for ADC1  */
  ADC1->SQR1    =  0;                     /* Regular chn. Sequence length = 1 */
  ADC1->SQR2    =  0;                     /* Clear register                   */
  ADC1->SQR3    = ( 1UL <<  0);           /* 1. conversion = channel 1        */
  ADC1->CR2     = ( 7UL << 17)         /* select SWSTART                   */
                  |( 1UL << 20) ;          // enable external Trigger          
  ADC1->CR2    |= ( 1UL <<  0);           /* ADC enable                       */
  ADC1->CR2    |= ( 1UL <<  1); 
  ADC1->CR2    |=  1 <<  3;               /* Initialize calibration registers */
  while (ADC1->CR2 & (1 << 3));           /* Wait for init to finish          */
  ADC1->CR2    |=  1 <<  2;               /* Start calibration                */
  while (ADC1->CR2 & (1 << 2));           /* Wait for calibration to finish   */
}

//// initialize usart
void usart_init(void) 
{
  RCC->APB2ENR |=(1<<2); /* Enable GPIOA clock*/
  RCC->APB2ENR |=(1<<14);  /* Enable USART1 clock*/
  GPIOA->CRH   &= ~(0xFF <<  4);        /* clear PA9, PA10                  */
  GPIOA->CRH   |=  (0xB <<  4);        /* USART1 Tx (PA9) alternate function output push-pull */
  GPIOA->CRH   |=  (0x4 <<  8);        /* USART1 Rx (PA10) input floating  */
  USART1->BRR=0x1D4C;     // baudrate 9600 assuming f=8MHz
  USART1->CR1=((1<<2)|(1<<3)|(0<<12)|(1<<13));  // enable transmit and receive word length = 8 bit and usart enable 
	USART1->CR2   = 0x0000;                 /* 1 stop bit                       */
  USART1->CR3   = 0x0000;                 /* no flow control                  */
}	
void usart_sendchar(char data)
{
	while(!(USART1->SR&(1<<7)));
	USART1->DR=data;
}
void usart_sendstring(char *s) 
{
  	while (*s) 
	{
   	usart_sendchar(*s);
		s++;
	}
}
void Timer_init(void)
{
  RCC->APB2ENR |= (1<<11);             /* enable clock for TIM1    */
  TIM1->PSC   = ( 1 - 1 );   /* set prescaler   = 72000000Hz  */
  TIM1->ARR   = ( 1000 - 1);   /* set auto-reload = 13/89 us */
  TIM1->RCR   =  0;           /* set repetition counter   */
  TIM1->DIER = (1<<0);   /* Update Interrupt enable  */
  TIM1->CR1  |= (1<<0);   /* 0x0001 timer enable       */
  NVIC_EnableIRQ(TIM1_UP_IRQn);
}
void TIM1_UP_IRQHandler (void) 
{
  if (TIM1->SR & (1<<0)) 
  { /* UIF set    */
	  time++;
	  TIM1->SR &= ~0x1;         /* clear UIF flag           */	
	}
}
int main()
{
  SysTick_Config(SystemCoreClock/1000); // setup systick timer for 1ms interrupts
  usart_init();	
  RCC->APB2ENR |= (1<<3);
  GPIOB->CRL &=~0xF0;
  GPIOB->CRL |= 0x30;            //////// GPIOB.1  output
while (1)
{
  if (flag==0)
  {
    Delay_ms(500);
    GPIOB->BSRR=(1<<17);
    Delay_ms(500);	
    GPIOB->BSRR=(1<<1);
    flag=1;
    Timer_init();
    ADC_Init();
    ADC1->CR2    |=  (1<<22);	      // start A/D conversion
  }
else if (flag==1)
{
  if (ADC1->SR & (1 << 1))
  {
    AD_last = ADC1->DR;
    val_Tx = (AD_last & 0xFFF);        /* use upper 12 bits of ADC  */
	  if (val_Tx>2580)
		{
		  ADC1->CR2    &= ~(1<<22); /* stop A/D conversion     */
	    flag=0;
  	  TIM1->CR1  &= 0;
	    counter=TIM1->CNT;
	    charge_time=(time*1000*period)+(counter*period);
	    c=10*charge_time;
	    sprintf(string,"the capacitance value is : %f uF\n", c);
	    sprintf(string2,"charge time is  %f\n time is %i\n counter is %i\n ",charge_time,time,counter);
	    usart_sendstring(string);
	    usart_sendstring(string2);						
	    time=0;
      flag=0;	
      counter=0;
      charge_time=0;
      c=0;
      ADC_Init();
			GPIOB->BSRR=(1<<17);
	  }
 }
	
	
}
}	
	
}

void SysTick_Handler(void) 
{
  msTicks++;
}
void Delay_ms (uint32_t dlyTicks) 
{
  uint32_t curTicks;
  curTicks = msTicks;
  while ((msTicks - curTicks)< dlyTicks);
}
	



