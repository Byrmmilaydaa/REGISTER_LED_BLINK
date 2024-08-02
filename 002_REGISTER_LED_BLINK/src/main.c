#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

                   //REGÝSTER ÝLE LED BLINK ÝSLEMÝ
//1)STM32F407 kartýnýn Reference manualini açýyoruz ve GPIO ile ilgili bölüme gireceðiz.
//2)GPIO register bölümüne giriyoruz ve GPIO port mode register bölümü ile baþlýyoruz.
    //(Register seviyesinde yazdýðýnýz kodlar donanýma yakýn yazabileceðiniz en alt seviyedir)
//3)32 tane register var ve her iki tanesi bir bit yapýyor.
    //Orada yazan rw ise hem yazýlýp hemde okunabilen pin demek(ledlerimiz 12,13,14,15.pinlerde olduðu için moder12,moder13,moder14,moder15 oluyor)
//4)GPIO register iþlemi bittikten sonra RCC register ayarlarýna geçiyoruz.
//5)RCC AHB1 peripheral clock register(RCC_AHB1ENR) sekmesine týklýyoruz.
//6)Portlara çýkýþ iþlemi vereceðiz.


void RCC_Config(void){
	//sistem ayarlarýmýz 8 MHz den 168 MHz ye çýkmýþtýr.

	RCC -> CR &= ~ (1 << 0);//ayrýca 0x00000083 þeklinde de yazabiliriz ayný adresi ifade eder.
	//&= bu iþaret kaydet demek
	//~ bu iþaret tersle demek.Bu þekilde (&= ~) kullanýlýrsa ise sadece 0.biti tersle demek
	//HSI OFF, HSI=dahili osilatörler. Dahili osilatörler harici osilatörlere göre daha kalitesizdir.Özellikle kritik uygulamalarda önemli bu durum
	//kontrol registerýmýzý resetlendi demek bu kod ile

	RCC -> CR |= 1 << 16; //HSE ON HSE=harici osilatör. //16.bite 1 yazmam gerekiyor ve bu þekilde sadece 16.bite yazmýþ oluyoruz.
	//|= bu iþaret daha önceki kullandýðýmýz &= bu iþarete +1 ekler. ya da eþittir yapar


	while(!(RCC -> CR & (1 << 17))); //HSE nin aktif olmasýný bekle
	//HSE çalýþýyor mu bunu takip etmem için bu kodu yazýyoruz.17.bit 1 olana kadar demek ama parantez dýþýnda ünlem iþareti ile tersleme iþlemi yapýlýyor.
	//8 000 000 -> 168 000 000 çýkaracak iþlemler;

	RCC -> CR |= 1 << 19; //saat güvenlik sistemi var bu bitte
	//|= iþareti ile ilave iþlemler yapýyoruz.

	//pll açmadan önce pll config ayarlarý yapmamýz gerekiyor.
	//PLL (Phase-Locked Loop) konfigürasyon ayarlarý, bir PLL devresinin çalýþma parametrelerini belirleyen ayarlardýr.
	//PLL, bir osilatörün fazýný baþka bir sinyalin fazý ile kilitleyerek çalýþýr ve genellikle frekans çarpma, frekans bölme, frekans sentezi ve sinyal faz kilitleme gibi uygulamalarda kullanýlýr.
	//PLL konfigürasyon ayarlarý, PLL devresinin giriþ ve çýkýþ frekanslarýný, kilitleme süresini, bant geniþliðini ve diðer performans özelliklerini belirler.

	/*
	 RCC -> PLLCFGR &= ~ (1 << 0); //PLLM0 0 //bölme oranýnýn 4 olduðu anlamýna geliyor.
	 RCC -> PLLCFGR &= ~ (1 << 1); //PLLM1 0
	 RCC -> PLLCFGR &= ~ (1 << 2); //PLLM2 1
	 RCC -> PLLCFGR &= ~ (1 << 3); //PLLM3 0
	 RCC -> PLLCFGR &= ~ (1 << 4); //PLLM4 0
	 RCC -> PLLCFGR &= ~ (1 << 5); //PLLM5 0

	 Böyle uzun uzun yazmak yerine RCC -> PLLCFGR |=(4 << 0); bu þekilde yazarakta ayný þeyi yapmýþ oluyoruz aslýnda
	  */

	//cubemx programýnda istediðim deðerlere göre bana verilmiþ olan m,n ve p deðerlerini kullandým.

	RCC -> PLLCFGR = 0x00000000; //PLLCFGR Reset 4 lü 4 lü 0 lama iþlemi yapýldý.
	RCC -> PLLCFGR |=(1 << 22); //pll sürücüsünü seçmemizi saðlar biz hangi pll i kullanacaðýz bunu seçeriz(HSE mý yoksa HSI mý kullanacaksýn demek)
	//PLL Source HSE

	RCC -> PLLCFGR |=(4 << 0); // PLL M 4
	RCC -> PLLCFGR |=(168 << 6);// PLL N 168 ayarlamýþ olduk //6 bit sola ötele 168 yazdýr demek

	/*RCC -> PLLCFGR &= ~(1 << 16); // PLL P 2 //16 bit ötele ve 0 yaz demek
	RCC -> PLLCFGR &= ~(1 << 17); // PLL P 2 //17 bit ötele ve 0 yaz demek
    */

	RCC -> CR |= (1 << 24); //pll açma iþlemi yapýlýyor. //PLL ON
	while(!(RCC -> CR & (1 << 25))); // PLL aktif olmasýný bekle demek

	//flag=sistem hazýr olduðunda sistemi kontrol eder. bu bitte flag iþlemi var. her flag iþlemi gördüðümüzde while kullanacaðýz.

	//System Clock is PLL iþlemi
	RCC -> CFGR &= ~ (1 << 0);
	RCC -> CFGR |=  (1 << 1);

	while(!(RCC -> CFGR & (1 << 1))); //sistem clock pll den kullanýlacak demektir. //Select system clock is PLL clock
}
void GPIO_Config(void){

	RCC->AHB1ENR |= 1 << 3; //3.biti 1 yapýyoruz ki d portu aktif hale gelsin


	//parametre almayan fonksiyon ve bu fonksiyon benim ayarlarýmý yapacak
	//D portunu kullanmak için önce d portunun clock hattýný aktif etmem lazým ama moder ile gittiðimiz için sonra yapacaðým bu iþlemi

	GPIOD->MODER |= 1<<24; //ya da iþlemi ile ekleme yapýyoruz 24.registerý 0 yapmýþ olduk (12.pini 1 yapýyoruz)
	GPIOD->MODER &= ~(1<<25); //25.registerý 0 yapmýþ olduk
	GPIOD->MODER |= 1<<26; //13.pini 1 yapýyoruz
	GPIOD->MODER &= ~(1<<27);
	GPIOD->MODER |= 1<<28; //14.pini 1 yapýyoruz
	GPIOD->MODER &= ~(1<<29);
	GPIOD->MODER |= 1<<30; //15.pini 1 yapýyoruz
	GPIOD->MODER &= ~(1<<31);

	GPIOD ->OSPEEDR |= 0xFF000000; //hýz ayarýný maksimum seçtik
	//24.pinden 31.pine kadar 1 yazma iþlemidir bu ifade hesap makinesinden programlayýcý baþlýðý altýndan hesaplýyoruz.
}

int main(void)
{
	RCC_Config();

	SystemCoreClockUpdate(); //sistem clock'u güncellenmiþ olacak

	GPIO_Config();
  while (1)
  {
	  GPIOD->ODR |= 1 << 12; //12.portuna çýkýþ iþlemi(1 deðeri) vermiþ oluyoruz
	  GPIOD->ODR |= 1 << 13; //13.portuna çýkýþ iþlemi vermiþ oluyoruz
	  GPIOD->ODR |= 1 << 14; //14.portuna çýkýþ iþlemi vermiþ oluyoruz
	  GPIOD->ODR |= 1 << 15; //15.portuna çýkýþ iþlemi vermiþ oluyoruz

	  //led yakýp söndürme iþlemi yapalým
	  for(int i=0; i<1600000;i++); //aslýnda burada 1600000 bu deðere kadar bekleme iþlemi yaptýrmýþ oluyoruz.

	  GPIOD->ODR &= ~(1 << 12); //12.portuna 0 vermiþ oluyoruz.
	  GPIOD->ODR &= ~(1 << 13); //13.portuna 0 vermiþ oluyoruz.
	  GPIOD->ODR &= ~(1 << 14); //14.portuna 0 vermiþ oluyoruz.
	  GPIOD->ODR &= ~(1 << 15); //15.portuna 0 vermiþ oluyoruz.

	  for(int i=0; i<1600000;i++); //bekleme iþlemini tekrar yapmýþ oluyoruz.
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
