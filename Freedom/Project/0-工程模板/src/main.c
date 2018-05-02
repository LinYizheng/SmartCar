#include "app_include.h"
uint8_t  OV7725_IMAGES[HIGH_D][(WIDTH_D / 8)];

int main(void)
{   
    DelayInit();
    System_Init();	
#ifdef USE_OV7725
//����д�����������ʱ������Ļ��������ɫѭ�������Ļ���
//ͬʱҲ��Ϊ�˷�����ʱ���ܹ���һ��ʱ��
		DelayMs(2000);
		StartGetImage();
#endif

    while(1)
    {
        if(Image_Flag != FALSE)	//�ɼ�һ�β��20ms
        { 	
						GPIO_WriteBit(HW_GPIOE, 10, 1);	//���Դ���ʱ��
						Get_Speed(&MotorQD);//��ȡ�ٶ�
						
						//VGA---10tline---ѡ��QVGA�Ƚ���
						//QVGA---14tline---ʱ�䲻��---��������
						for(int i=0; i<HIGH_D; i++)
							for(int j=0; j<(WIDTH_D / 8); j++)
								OV7725_IMAGES[i][j] = OV7725_IMAGEB[i][j];
								
						//���ˣ���֤��Current_Rol=0��Ҳ����˵����һ�鸴������һ�¼���
						//��ѹͼ�����ݣ���ǰһ�����ն�ĩ������������---�ǳ���Ҫ
						//��������������н�ѹ�����ݵģ�����ʱ�䲻������˸�Ϊ�˸�ֵ���
						//�����ڸ�ֵ���ִ����󣬲��ܽ���ͼ���ѹ������Ȼ��ά�����ѹ̫��ʱ��
						Array_ImageDeal(OV7725_IMAGES,OV7725_IMAGE);
						
						//�ڴ���Ĺ����У�Current_Rol��ʵʱ�����ģ����ʾ�ڴ����ͬʱ�����ж�Ҳ�ڲ���ִ��
						//���������һ���ɼ���ͼ�񣬵�ǰҲ�ڽ��вɼ�ͼ��Ϊ����һ��ʹ��
						//���Ƶ���ı��뱣֤�̶����ڣ�Ӧ��ͼ����ǰ������Ϊͼ�����ʱ���п�����
						//�����Ӧʱ�䲻�Ǻܸߣ����ڶ�����ԣ����Խ����ƽ���Ϊһ���̶�������
						TimeToControlMotor(); //�������
						ImageFinshed();	//ͼ����
						TimeToControlSteer(); //�������
						BeeperLoopTime(&BeepLoopTime);//�������죬�������
						Image_Flag = FALSE;	//������ɣ���һͼ�����
						
						GPIO_WriteBit(HW_GPIOE, 10, 0);	//���Դ���ʱ��
        }
				
    }
}
