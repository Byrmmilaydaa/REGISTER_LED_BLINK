#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

                   //REG�STER �LE LED BLINK �SLEM�
//1)STM32F407 kart�n�n Reference manualini a��yoruz ve GPIO ile ilgili b�l�me girece�iz.
//2)GPIO register b�l�m�ne giriyoruz ve GPIO port mode register b�l�m� ile ba�l�yoruz.
    //(Register seviyesinde yazd���n�z kodlar donan�ma yak�n yazabilece�iniz en alt seviyedir)
//3)32 tane register var ve her iki tanesi bir bit yap�yor.
    //Orada yazan rw ise hem yaz�l�p hemde okunabilen pin demek(ledlerimiz 12,13,14,15.pinlerde oldu�u i�in moder12,moder13,moder14,moder15 oluyor)
//4)GPIO register i�lemi bittikten sonra RCC register ayarlar�na ge�iyoruz.
//5)RCC AHB1 peripheral clock register(RCC_AHB1ENR) sekmesine t�kl�yoruz.
//6)Portlara ��k�� i�lemi verece�iz.


void RCC_Config(void){
	//sistem ayarlar�m�z 8 MHz den 168 MHz ye ��km��t�r.

	RCC -> CR &= ~ (1 << 0);//ayr�ca 0x00000083 �eklinde de yazabiliriz ayn� adresi ifade eder.
	//&= bu i�aret kaydet demek
	//~ bu i�aret tersle demek.Bu �ekilde (&= ~) kullan�l�rsa ise sadece 0.biti tersle demek
	//HSI OFF, HSI=dahili osilat�rler. Dahili osilat�rler harici osilat�rlere g�re daha kalitesizdir.�zellikle kritik uygulamalarda �nemli bu durum
	//kontrol register�m�z� resetlendi demek bu kod ile

	RCC -> CR |= 1 << 16; //HSE ON HSE=harici osilat�r. //16.bite 1 yazmam gerekiyor ve bu �ekilde sadece 16.bite yazm�� oluyoruz.
	//|= bu i�aret daha �nceki kulland���m�z &= bu i�arete +1 ekler. ya da e�ittir yapar


	while(!(RCC -> CR & (1 << 17))); //HSE nin aktif olmas�n� bekle
	//HSE �al���yor mu bunu takip etmem i�in bu kodu yaz�yoruz.17.bit 1 olana kadar demek ama parantez d���nda �nlem i�areti ile tersleme i�lemi yap�l�yor.
	//8 000 000 -> 168 000 000 ��karacak i�lemler;

	RCC -> CR |= 1 << 19; //saat g�venlik sistemi var bu bitte
	//|= i�areti ile ilave i�lemler yap�yoruz.

	//pll a�madan �nce pll config ayarlar� yapmam�z gerekiyor.
	//PLL (Phase-Locked Loop) konfig�rasyon ayarlar�, bir PLL devresinin �al��ma parametrelerini belirleyen ayarlard�r.
	//PLL, bir osilat�r�n faz�n� ba�ka bir sinyalin faz� ile kilitleyerek �al���r ve genellikle frekans �arpma, frekans b�lme, frekans sentezi ve sinyal faz kilitleme gibi uygulamalarda kullan�l�r.
	//PLL konfig�rasyon ayarlar�, PLL devresinin giri� ve ��k�� frekanslar�n�, kilitleme s�resini, bant geni�li�ini ve di�er performans �zelliklerini belirler.

	/*
	 RCC -> PLLCFGR &= ~ (1 << 0); //PLLM0 0 //b�lme oran�n�n 4 oldu�u anlam�na geliyor.
	 RCC -> PLLCFGR &= ~ (1 << 1); //PLLM1 0
	 RCC -> PLLCFGR &= ~ (1 << 2); //PLLM2 1
	 RCC -> PLLCFGR &= ~ (1 << 3); //PLLM3 0
	 RCC -> PLLCFGR &= ~ (1 << 4); //PLLM4 0
	 RCC -> PLLCFGR &= ~ (1 << 5); //PLLM5 0

	 B�yle uzun uzun yazmak yerine RCC -> PLLCFGR |=(4 << 0); bu �ekilde yazarakta ayn� �eyi yapm�� oluyoruz asl�nda
	  */

	//cubemx program�nda istedi�im de�erlere g�re bana verilmi� olan m,n ve p de�erlerini kulland�m.

	RCC -> PLLCFGR = 0x00000000; //PLLCFGR Reset 4 l� 4 l� 0 lama i�lemi yap�ld�.
	RCC -> PLLCFGR |=(1 << 22); //pll s�r�c�s�n� se�memizi sa�lar biz hangi pll i kullanaca��z bunu se�eriz(HSE m� yoksa HSI m� kullanacaks�n demek)
	//PLL Source HSE

	RCC -> PLLCFGR |=(4 << 0); // PLL M 4
	RCC -> PLLCFGR |=(168 << 6);// PLL N 168 ayarlam�� olduk //6 bit sola �tele 168 yazd�r demek

	/*RCC -> PLLCFGR &= ~(1 << 16); // PLL P 2 //16 bit �tele ve 0 yaz demek
	RCC -> PLLCFGR &= ~(1 << 17); // PLL P 2 //17 bit �tele ve 0 yaz demek
    */

	RCC -> CR |= (1 << 24); //pll a�ma i�lemi yap�l�yor. //PLL ON
	while(!(RCC -> CR & (1 << 25))); // PLL aktif olmas�n� bekle demek

	//flag=sistem haz�r oldu�unda sistemi kontrol eder. bu bitte flag i�lemi var. her flag i�lemi g�rd���m�zde while kullanaca��z.

	//System Clock is PLL i�lemi
	RCC -> CFGR &= ~ (1 << 0);
	RCC -> CFGR |=  (1 << 1);

	while(!(RCC -> CFGR & (1 << 1))); //sistem clock pll den kullan�lacak demektir. //Select system clock is PLL clock
}
void GPIO_Config(void){

	RCC->AHB1ENR |= 1 << 3; //3.biti 1 yap�yoruz ki d portu aktif hale gelsin


	//parametre almayan fonksiyon ve bu fonksiyon benim ayarlar�m� yapacak
	//D portunu kullanmak i�in �nce d portunun clock hatt�n� aktif etmem laz�m ama moder ile gitti�imiz i�in sonra yapaca��m bu i�lemi

	GPIOD->MODER |= 1<<24; //ya da i�lemi ile ekleme yap�yoruz 24.register� 0 yapm�� olduk (12.pini 1 yap�yoruz)
	GPIOD->MODER &= ~(1<<25); //25.register� 0 yapm�� olduk
	GPIOD->MODER |= 1<<26; //13.pini 1 yap�yoruz
	GPIOD->MODER &= ~(1<<27);
	GPIOD->MODER |= 1<<28; //14.pini 1 yap�yoruz
	GPIOD->MODER &= ~(1<<29);
	GPIOD->MODER |= 1<<30; //15.pini 1 yap�yoruz
	GPIOD->MODER &= ~(1<<31);

	GPIOD ->OSPEEDR |= 0xFF000000; //h�z ayar�n� maksimum se�tik
	//24.pinden 31.pine kadar 1 yazma i�lemidir bu ifade hesap makinesinden programlay�c� ba�l��� alt�ndan hesapl�yoruz.
}

int main(void)
{
	RCC_Config();

	SystemCoreClockUpdate(); //sistem clock'u g�ncellenmi� olacak

	GPIO_Config();
  while (1)
  {
	  GPIOD->ODR |= 1 << 12; //12.portuna ��k�� i�lemi(1 de�eri) vermi� oluyoruz
	  GPIOD->ODR |= 1 << 13; //13.portuna ��k�� i�lemi vermi� oluyoruz
	  GPIOD->ODR |= 1 << 14; //14.portuna ��k�� i�lemi vermi� oluyoruz
	  GPIOD->ODR |= 1 << 15; //15.portuna ��k�� i�lemi vermi� oluyoruz

	  //led yak�p s�nd�rme i�lemi yapal�m
	  for(int i=0; i<1600000;i++); //asl�nda burada 1600000 bu de�ere kadar bekleme i�lemi yapt�rm�� oluyoruz.

	  GPIOD->ODR &= ~(1 << 12); //12.portuna 0 vermi� oluyoruz.
	  GPIOD->ODR &= ~(1 << 13); //13.portuna 0 vermi� oluyoruz.
	  GPIOD->ODR &= ~(1 << 14); //14.portuna 0 vermi� oluyoruz.
	  GPIOD->ODR &= ~(1 << 15); //15.portuna 0 vermi� oluyoruz.

	  for(int i=0; i<1600000;i++); //bekleme i�lemini tekrar yapm�� oluyoruz.
  }
}


/*
 * Callback used by stm32f4_discovery_audio_codec.c.
 * Refer to stm32f4_discovery_audio_codec.h for more info.
 */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size){
  /* TODO, implement your code here */
  return;
}

/*
 * Callback used by stm324xg_eval_audio_codec.c.
 * Refer to stm324xg_eval_audio_codec.h for more info.
 */
uint16_t EVAL_AUDIO_GetSampleCallBack(void){
  /* TODO, implement your code here */
  return -1;
}
