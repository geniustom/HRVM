#ifndef MEUNUI_H
#define MEUNUI_H


#define CleenScreen 0x10
#define VerString "VER 3.0"
//---------OtherSetting's Control word-------------
#define NormalNode 0
#define StaticScreen 0x01//BIT0
#define RDModeNode 0x02//BIT1     //�[�F���ѼơA�|�b�W����TITTLE SHOW�X�ثe��RD MODE..�t�~�A�]���Ѹ`�I�������ѬO�_��RDMODE��FLAG
#define DynamicScreen 0x04//BIT2  //������즹NODE�AOS���|�۰ʧ�s�ù��A�ݭn��ʭ�ø(�ϥΩ���qOR�ɶ����H�ɷ|��s�e����NODE)
#define LCDTEST 0x08//BIT3 //������즹NODE�A�W�褣�|SHOW ���D
#define DontPrintName 0x10//BIT4 //������즹NODE�A�������|SHOW ���D
#define NoBuzzer 0x20//BIT5 //�[�F���ѼơA������즹�B�ɡA�����sBUZZER���|�s

typedef struct{
    unsigned int   *ButtonLeft,*ButtonRight,*ButtonEnter;     //�u����ҫ�NODE
    unsigned int   *LButtonLeft,*LButtonRight,*LButtonEnter;  //������ҫ�NODE
    void (*OnEnterNode)(void);                       //�i�J�`�I��FUNTION POINT
    void (*OnExitNode)(void);                        //���}�`�I��FUNTION POINT
    unsigned char *NAME;
    unsigned int Page;                                      // 11/1 Allen Adder
    unsigned char OtherSetting;    //if this node is RD type..the RDType=1 
}MenuNode;


extern void (*WhenExitMenuNode)(void); //�i�J�`�I��FUNTION POINT
extern void ButtonPress();
extern void ButtonLongPress();
extern void StartECG();
extern void StopECG();
extern void Show_DateTime();

extern const MenuNode Standby;
extern const MenuNode Main_DateTime,Main_ECG,Main_Thermal;         //�D���3
extern const MenuNode ECG_Main,ECG_Success,ECG_Abort;              //ECG���A
extern const MenuNode Thermal_Main,Thermal_Abort,Thermal_Success;  //Thermal���A
extern const MenuNode Main_User,User1,User2,User3,User4,User5,User6; //User Select
#endif


