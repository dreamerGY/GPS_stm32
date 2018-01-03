#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"     
#include "usmart.h"			
#include "usart3.h"
#include "gps.h"
//ATK-S1216F8 GPSģ�����
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//����1,���ͻ�����
nmea_msg gpsx; 											//GPS��Ϣ
__align(4) u8 dtbuf[50];   								//��ӡ������
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode�ַ��� 
int localTime_year,localTime_month,localTime_date,localTime_hour,timezone;

//����ʱ�����������ݾ��ȼ���
static int calculateTimezone(double lat,double lon){
    int a,b,c,timezone;
    a = (int)(fabs(lon)+0.5);//�Ծ��Ƚ����������룬��ȡ������
    b = a/15; //��
    c = a%15; //����
    if((lat >=17.9 && lat <=53 && lon>=75 && lon<=125) || (lat>=40 && lat <=53 && lon>=125 && lon<= 135)){
			//�����γ�ȴ����й���ͼ�ڣ��򶼻�Ϊ������
    timezone = 8;
    }
    else{

        if(c > 7.5)
            timezone = b+1;
        else
            timezone = b;
        if(lon > 0.0f)
            timezone = timezone;
        else
            timezone = (-1)*timezone;
    }
    return timezone;
}

// UTCʱ��ת��Ϊ����ʱ�亯��
static int UTCTOLocalTime(int timezone){
    int year,month,day,hour;
	  //int localTime_year,localTime_month,localTime_date,localTime_hour;
    int lastday = 0;// �µ����һ������
    int lastlastday = 0;// ���µ����һ������

    year = gpsx.utc.year; 
    month = gpsx.utc.month;
    day = gpsx.utc.date;
    hour = gpsx.utc.hour + timezone; 

    if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12){
        lastday = 31;
        if(month == 3){
            if((year%400 == 0)||(year%4 == 0 && year%100 != 0))//�ж��Ƿ�Ϊ���꣬����ܱ�400��4�����������ܱ�100����
                lastlastday = 29;//�����2��29�죬ƽ��28��
            else
                lastlastday = 28;
        }
        if(month == 8)
            lastlastday = 31;
    }
    else if(month == 4 || month == 6 || month == 9 || month == 11){
        lastday = 30;
        lastlastday = 31;
    }
    else{
        lastlastday = 31;
        if((year%400 == 0)||(year%4 == 0 && year%100 != 0))// �����2��29�죬ƽ��28��
            lastday = 29;
        else
            lastday = 28;
    }

    if(hour >= 24){//�����ʱ�����ڻ����24ʱ��Ӧ��ȥ24,���ڼ�һ��
            hour -= 24;
            day += 1; 
            if(day > lastday){ 
                day -= lastday;
                month += 1;

                if(month > 12){
                    month -= 12;
                    year += 1;
                }
            }
        }
    if(hour < 0){//�����ʱ��Ϊ����ʱ��Ӧ����24:00,���ڼ�һ��
            hour += 24;
            day -= 1; 
            if(day < 1){ //����Ϊ0ʱ�����ڱ�Ϊ��һ�����һ�죬�·ݼ�һ����
                day = lastlastday;
                month -= 1;
                if(month < 1){ //�·�Ϊ0ʱ���·�Ϊ12�£���ݼ�1��
                    month = 12;
                    year -= 1;
                }
            }
        }
   //�õ�ת����ı���ʱ��
   localTime_year = year;
   localTime_month = month;
   localTime_date = day;
   localTime_hour = hour;
	 return localTime_year;
	 return localTime_month;
	 return localTime_date;
	 return localTime_hour;
}

//��ʾGPS��λ��Ϣ 
void Gps_Msg_Show(void)
{
 	//UTCTOLocalTime(8);
	float tp;		   
	POINT_COLOR=BLUE;  	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//�õ������ַ���
 	LCD_ShowString(30,120,200,16,16,dtbuf);	 	   
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//�õ�γ���ַ���
 	LCD_ShowString(30,140,200,16,16,dtbuf);	 	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//�õ��߶��ַ���
 	LCD_ShowString(30,160,200,16,16,dtbuf);	 			   
	tp=gpsx.speed;	   
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//�õ��ٶ��ַ���	 
 	LCD_ShowString(30,180,200,16,16,dtbuf);	 				    
	if(gpsx.fixmode<=3)														//��λ״̬
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
	  LCD_ShowString(30,200,200,16,16,dtbuf);			   
	}	 	   
	sprintf((char *)dtbuf,"GPS+BD Valid satellite:%02d",gpsx.posslnum);	 		//���ڶ�λ��GPS������
 	LCD_ShowString(30,220,200,16,16,dtbuf);	    
	sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);	 		//�ɼ�GPS������
 	LCD_ShowString(30,240,200,16,16,dtbuf);
	
	sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);	 		//�ɼ�����������
 	LCD_ShowString(30,260,200,16,16,dtbuf);
	
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//��ʾUTC����
	LCD_ShowString(30,280,200,16,16,dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾUTCʱ��
  LCD_ShowString(30,300,200,16,16,dtbuf);		  
	
	sprintf((char *)dtbuf,"Timezone:%04d     ",timezone);	     			//�õ�ʱ��
 	LCD_ShowString(30,20,200,16,16,dtbuf);	
	sprintf((char *)dtbuf,"BeiJing Date:%04d/%02d/%02d   ",localTime_year,localTime_month,localTime_date);	//��ʾBeiJing����
	LCD_ShowString(30,40,200,16,16,dtbuf);	
	sprintf((char *)dtbuf,"BeiJing Time:%02d:%02d:%02d   ",localTime_hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾBeiJingʱ��
	LCD_ShowString(30,60,200,16,16,dtbuf);
}

int main(void)
{        
	//int localTime_year,localTime_month,localTime_date,localTime_hour;
	u16 i,rxlen;
	u16 lenx;
	u8 key=0XFF;
	u8 upload=0; 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	usart3_init(384200);  //��ʼ������3������Ϊ115200
	LED_Init();					//��ʼ��LED  
 	LCD_Init();					//LCD��ʼ��  
 	KEY_Init();					//������ʼ��  
	usmart_dev.init(168);		//��ʼ��USMART
	POINT_COLOR=RED;
	LCD_ShowString(30,80,200,16,16,"KEY0:Upload NMEA Data SW");   	 										   	   
  LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF"); 
	if(SkyTra_Cfg_Rate(5)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ5Hz,˳���ж�GPSģ���Ƿ���λ. 
	{
   	LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Setting...");
		do
		{
			usart3_init(9600);			//��ʼ������3������Ϊ9600
	  	SkyTra_Cfg_Prt(3);			//��������ģ��Ĳ�����Ϊ38400
			usart3_init(38400);			//��ʼ������3������Ϊ38400
      key=SkyTra_Cfg_Tp(100000);	//������Ϊ100ms
		}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);//����SkyTraF8-BD�ĸ�������Ϊ5Hz
	  LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Set Done!!");
		delay_ms(500);
		LCD_Fill(30,120,30+200,120+16,WHITE);//�����ʾ 
	}
	while(1) 
	{		
		delay_ms(1);
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{
			rxlen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//������һ�ν���
			USART1_TX_BUF[i]=0;			//�Զ���ӽ�����
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ���
			calculateTimezone(gpsx.latitude,gpsx.longitude);
			//UTCTOLocalTime(timezone);
			UTCTOLocalTime(8);
			Gps_Msg_Show();				//���ú�������ʾ��Ϣ	
			if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//���ͽ��յ������ݵ�����1
 		}
		key=KEY_Scan(0);
		if(key==KEY0_PRES)
		{
			upload=!upload;
			POINT_COLOR=RED;
			if(upload)LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:ON ");
			else LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF");
 		}
		if((lenx%500)==0)
			LED0=!LED0;
		lenx++;	
	}
}













