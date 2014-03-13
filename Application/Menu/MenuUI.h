#ifndef MEUNUI_H
#define MEUNUI_H


#define CleenScreen 0x10
#define VerString "VER 3.0"
//---------OtherSetting's Control word-------------
#define NormalNode 0
#define StaticScreen 0x01//BIT0
#define RDModeNode 0x02//BIT1     //加了此參數，會在上面的TITTLE SHOW出目前為RD MODE..另外，也提供節點之間辨識是否為RDMODE的FLAG
#define DynamicScreen 0x04//BIT2  //當切換到此NODE，OS不會自動更新螢幕，需要手動重繪(使用於測量OR時間等隨時會更新畫面的NODE)
#define LCDTEST 0x08//BIT3 //當切換到此NODE，上方不會SHOW 標題
#define DontPrintName 0x10//BIT4 //當切換到此NODE，中央不會SHOW 標題
#define NoBuzzer 0x20//BIT5 //加了此參數，當切換到此處時，按按鈕BUZZER不會叫

typedef struct{
    unsigned int   *ButtonLeft,*ButtonRight,*ButtonEnter;     //短按後所指NODE
    unsigned int   *LButtonLeft,*LButtonRight,*LButtonEnter;  //長按後所指NODE
    void (*OnEnterNode)(void);                       //進入節點的FUNTION POINT
    void (*OnExitNode)(void);                        //離開節點的FUNTION POINT
    unsigned char *NAME;
    unsigned int Page;                                      // 11/1 Allen Adder
    unsigned char OtherSetting;    //if this node is RD type..the RDType=1 
}MenuNode;


extern void (*WhenExitMenuNode)(void); //進入節點的FUNTION POINT
extern void ButtonPress();
extern void ButtonLongPress();
extern void StartECG();
extern void StopECG();
extern void Show_DateTime();

extern const MenuNode Standby;
extern const MenuNode Main_DateTime,Main_ECG,Main_Thermal;         //主選單3
extern const MenuNode ECG_Main,ECG_Success,ECG_Abort;              //ECG狀態
extern const MenuNode Thermal_Main,Thermal_Abort,Thermal_Success;  //Thermal狀態
extern const MenuNode Main_User,User1,User2,User3,User4,User5,User6; //User Select
#endif


