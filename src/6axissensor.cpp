#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "pCommon.h"
#include "rs_comm.h"
#include "pComResInternal.h"

// =============================================================================
//	マクロ定義
// =============================================================================
#define PRG_VER	"Ver 1.0.0"

// =============================================================================
//	構造体定義
// =============================================================================
typedef struct ST_SystemInfo {
	int com_ok;
} SystemInfo;

// =============================================================================
//	プロトタイプ宣言
// =============================================================================
void App_Init(void);
void App_Close(void);
int GetRcv_to_Cmd( char *rcv, char *prm);
ULONG SendData(UCHAR *pucInput, USHORT usSize);
void GetProductInfo(void);
void SerialStart(void);
void SerialStop(void);

// =============================================================================
//	モジュール変数定義
// =============================================================================
SystemInfo gSys;
UCHAR CommRcvBuff[256];
UCHAR CommSendBuff[1024];
UCHAR SendBuff[512];

//シリアル通信用
#include <ros/ros.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "rs_comm.h"

#define MAX_BUFF		10
#define MAX_LENGTH	255
#define STS_IDLE		0
#define STS_WAIT_STX	1
#define STS_DATA		2
#define STS_WAIT_ETX	3
#define STS_WAIT_BCC	4

static int Comm_RcvF=0;								//受信データ有フラグ
static int p_rd=0,p_wr=0;							//受信リングバッファ読出し、書込みポインタ
static int fd=0;										//
static int rcv_n=0;									//受信文字数
static UCHAR delim;									//受信データデリミタ
static UCHAR rcv_buff[MAX_BUFF][MAX_LENGTH];	//受信リングバッファ
static UCHAR stmp[MAX_LENGTH];						//
struct termios tio;									//ポート設定構造体

unsigned char rbuff[MAX_LENGTH];
unsigned char ucBCC;

geometry_msgs::Twist force_1;
geometry_msgs::Twist force_2;
geometry_msgs::Twist force_3;
geometry_msgs::Twist force_4;
geometry_msgs::Twist force_5;

double roop_c = 0;



// ----------------------------------------------------------------------------------
//	メイン関数
// ----------------------------------------------------------------------------------
//	引数	: non
//	戻り値	: non
// ----------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    ros::init(argc, argv, "force_sensor");
    ros::NodeHandle nh;
    ros::Publisher cmd_pub = nh.advertise<geometry_msgs::Twist>("force_sensor_vel", 10);

	int i, l = 0, rt = 0;
	int mode_step = 0;
	long cnt = 0;
    geometry_msgs::Twist cmd;
	UCHAR strprm[256];
	ST_R_DATA_GET_F *stForce;

	App_Init();
	
	if (gSys.com_ok == NG) {
		printf("ComPortadsdadasdas Open Fail\n");
		exit(0);
	}

	// 連続送信開始
	SerialStart();
	while(ros::ok()){
		Comm_Rcv();
		memset(CommRcvBuff,0,sizeof(CommRcvBuff)); 
		rt = Comm_GetRcvData( CommRcvBuff );
		if ( rt>0 ) {
			cnt++;

			if (cnt%10 == 0) {//ここの数字を変えることでサンプリング周期が変わります．
				//roop_c = roop_c + 1;
				
				stForce = (ST_R_DATA_GET_F *)CommRcvBuff;
                cmd.linear.x = stForce->ssForce[0] * 0.03;
                cmd.linear.y = stForce->ssForce[1] * 0.03;
				cmd.linear.z = stForce->ssForce[2] * 0.03;
                cmd.angular.x = stForce->ssForce[3] * 0.0006;
				cmd.angular.y = stForce->ssForce[4] * 0.0006;
                cmd.angular.z = stForce->ssForce[5] * 0.0006;

                cmd_pub.publish(cmd);
				ROS_INFO("----------------------------");
                //std::cout << roop_c << std::endl;
                std::cout << " linear force" << std::endl;
                std::cout << "     Fx : " << std::setprecision(2) << cmd.linear.x << " [N]" << std::endl;
                std::cout << "     Fy : " << std::setprecision(2) << cmd.linear.y << " [N]" << std::endl;
                std::cout << "     Fz : " << std::setprecision(2) << cmd.linear.z << " [N]" << std::endl;
                std::cout << " angular force" << std::endl;
                std::cout << "     Mx : " << std::setprecision(4) << cmd.angular.x << " [Nm]" << std::endl;
                std::cout << "     My : " << std::setprecision(4) << cmd.angular.y << " [Nm]" << std::endl;
                std::cout << "     Mz : " << std::setprecision(4) << cmd.angular.z << " [Nm]" << std::endl;
			}
		}
	}
    SerialStop();
	App_Close();
	return 0;
}



// ----------------------------------------------------------------------------------
//	受信監視スレッド
// ----------------------------------------------------------------------------------
//	引数	: pParam .. 
//	戻り値	: non
// ----------------------------------------------------------------------------------
void Comm_Rcv(void)
{
	int i,rt=0;
	unsigned char ch;
	static int RcvSts = 0;

	while(1){
		rt=read(fd, rbuff, 1);
		//受信データあり
		if (rt > 0) {
			rbuff[rt]=0;
			ch=rbuff[0];
			
			switch (RcvSts) {
			case STS_IDLE:
				ucBCC = 0;								/* BCC */
				rcv_n = 0;
				if (ch == CHR_DLE) RcvSts = STS_WAIT_STX;
				break;
			case STS_WAIT_STX:
				if (ch == CHR_STX) {					/* STXがあれば次はデータ */
					RcvSts = STS_DATA;
				} else {								/* STXでなければ元に戻る */
					RcvSts = STS_IDLE;
				}
				break;
			case STS_DATA:
				if (ch == CHR_DLE) {					/* DLEがあれば次はETX */
					RcvSts = STS_WAIT_ETX;
				} else {								/* 受信データ保存 */
					stmp[rcv_n] = ch;
					ucBCC ^= ch;						/* BCC */
					rcv_n++;
				}
				break;
			case STS_WAIT_ETX:
				if (ch == CHR_DLE) {					/* DLEならばデータである */
					stmp[rcv_n] = ch;
					ucBCC ^= ch;						/* BCC */
					rcv_n++;
					RcvSts = STS_DATA;
				} else if (ch == CHR_ETX) {				/* ETXなら次はBCC */
					RcvSts = STS_WAIT_BCC;
					ucBCC ^= ch;						/* BCC */
				} else if (ch == CHR_STX) {			/* STXならリセット */
					ucBCC = 0;							/* BCC */
					rcv_n = 0;
					RcvSts = STS_DATA;
				} else {
					ucBCC = 0;							/* BCC */
					rcv_n = 0;
					RcvSts = STS_IDLE;
				}
				break;
			case STS_WAIT_BCC:
				if (ucBCC == ch) {						/* BCC一致 */
					//作成された文字列をリングバッファへコピー
					memcpy(rcv_buff[p_wr], stmp, rcv_n);
					p_wr++;
					if ( p_wr >= MAX_BUFF ) p_wr=0;
				}
				/* 次のデータ受信に備える */
				ucBCC = 0;					/* BCC */
				rcv_n = 0;
				RcvSts = STS_IDLE;
				break;
			default:
				RcvSts = STS_IDLE;
				break;
			}
			
			if (rcv_n  > MAX_LENGTH) {
				ucBCC = 0;
				rcv_n = 0;
				RcvSts = STS_IDLE;
			}
		} else {
			break;
		}
		
		//受信完了フラグ
		if (p_rd != p_wr) {
			Comm_RcvF=1;
		} else {
			Comm_RcvF=0;
		}
	}
}


// ----------------------------------------------------------------------------------
//	デバイスオープン
// ----------------------------------------------------------------------------------
//	引数	: dev .. シリアルポート
//	戻り値	: 正常時:0   エラー時:-1
// ----------------------------------------------------------------------------------
int Comm_Open(char *dev)
{
	//既にオープンしているときは一度閉じる
	if (fd != 0) Comm_Close();
	//ポートオープン
	fd = open(dev, O_RDWR | O_NDELAY | O_NOCTTY);
	if (fd < 0) return NG;
	//デリミタ
	delim=0;
		
	return OK;
}

// ----------------------------------------------------------------------------------
//	デバイスクローズ
// ----------------------------------------------------------------------------------
//	引数	: non
//	戻り値	: non
// ----------------------------------------------------------------------------------
void Comm_Close()
{
	if (fd > 0) {
		close(fd);
	}
	fd=0;
		
	return;
}

// ----------------------------------------------------------------------------------
//	ポート設定
// ----------------------------------------------------------------------------------
//	引数	: boud   .. ボーレート 9600 19200 ....
//			: parity .. パリティー 
//			: bitlen .. ビット長
//			: rts    .. RTS制御
//			: dtr    .. DTR制御
//	戻り値	: non
// ----------------------------------------------------------------------------------
void Comm_Setup(long baud ,int parity ,int bitlen ,int rts ,int dtr ,char code)
{
	long brate;
	long cflg;
	
	switch (baud) {
	case 2400  :brate=B2400;  break;
	case 4800  :brate=B4800;  break;
	case 9600  :brate=B9600;  break;
	case 19200 :brate=B19200; break;
	case 38400 :brate=B38400; break;
	case 57600 :brate=B57600; break;
	case 115200:brate=B115200;break;
	case 230400:brate=B230400;break;
	case 460800:brate=B460800;break;
	default    :brate=B9600;  break;
	}
	//パリティ
	switch (parity) {
	case PAR_NON:cflg=0;					 break;
	case PAR_ODD:cflg=PARENB | PARODD;break;
	default     :cflg=PARENB;			 break;
	}
	//データ長
	switch (bitlen) {
	case 7 :cflg |= CS7;break;
	default:cflg |= CS8;break;
	}
	//DTR
	switch (dtr) {
	case 1 :cflg &= ~CLOCAL;break;
	default:cflg |= CLOCAL; break;
	}
	//RTS CTS
	switch (rts) {
	case 0 :cflg &= ~CRTSCTS;break;
	default:cflg |= CRTSCTS; break;
	}
	
	//ポート設定フラグ
	tio.c_cflag = cflg | CREAD;
	tio.c_lflag = 0;
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_cc[VTIME] = 0;
	tio.c_cc[VMIN]  = 0;
	
	cfsetspeed(&tio, brate);	
	tcflush( fd, TCIFLUSH);				//バッファの消去
	tcsetattr( fd, TCSANOW , &tio);		//属性の設定
	
	delim=code;								//デリミタコード
	return;
}

// ----------------------------------------------------------------------------------
//	文字列送信
// ----------------------------------------------------------------------------------
//	引数	: buff .. 文字列バッファ
//			: l    .. 送信文字数
//	戻り値	: 1:OK -1:NG
// ----------------------------------------------------------------------------------
int Comm_SendData( UCHAR *buff, int l)
{
	if (fd <= 0 ) return -1;
	
	write( fd, buff, l);
	
	return OK;
}

// ----------------------------------------------------------------------------------
//	受信データ取得
// ----------------------------------------------------------------------------------
//	引数	: buff .. 文字列バッファ
//	戻り値	: 受信文字数
// ----------------------------------------------------------------------------------
int Comm_GetRcvData(UCHAR *buff)
{
	int l = rcv_buff[p_rd][0];
	
	if ( p_wr == p_rd ) return 0;
	
	memcpy(buff, &rcv_buff[p_rd][0], l);
	p_rd++;
	if (p_rd >= MAX_BUFF) p_rd=0;
	
	std::string s(reinterpret_cast< char const* >(buff)) ;

	l=s.size();
	
	//l=strlen(buff);
	
	return l;
}

// ----------------------------------------------------------------------------------
//	受信有無確認
// ----------------------------------------------------------------------------------
//	引数	: non 
//	戻り値	: 0:なし 0以外：あり
// ----------------------------------------------------------------------------------
int Comm_CheckRcv()
{
	return p_wr-p_rd;
}

// ----------------------------------------------------------------------------------
//	アプリケーション初期化
// ----------------------------------------------------------------------------------
//	引数	: non
//	戻り値	: non
// ----------------------------------------------------------------------------------
void App_Init(void)
{
	int rt;
	
	//Commポート初期化
	gSys.com_ok = NG;

	rt = Comm_Open("/dev/ttyACM0");
	if ( rt==OK ) {
		Comm_Setup( 460800, PAR_NON, BIT_LEN_8, 0, 0, CHR_ETX);
		gSys.com_ok = OK;
	}

}

// ----------------------------------------------------------------------------------
//	アプリケーション終了処理
// ----------------------------------------------------------------------------------
//	引数	: non
//	戻り値	: non
// ----------------------------------------------------------------------------------
void App_Close(void)
{
	printf("Application Close\n");
	
	if ( gSys.com_ok == OK) {
		Comm_Close();
	}
}

/*********************************************************************************
* Function Name  : HST_SendResp
* Description    : データを整形して送信する
* Input          : pucInput 送信データ
*                : 送信データサイズ
* Output         : 
* Return         : 
*********************************************************************************/
ULONG SendData(UCHAR *pucInput, USHORT usSize)
{
	USHORT usCnt;
	UCHAR ucWork;
	UCHAR ucBCC = 0;
	UCHAR *pucWrite = &CommSendBuff[0];
	USHORT usRealSize;
	
	// データ整形 
	*pucWrite = CHR_DLE;					// DLE 
	pucWrite++;
	*pucWrite = CHR_STX;					// STX 
	pucWrite++;
	usRealSize =2;
	
	for (usCnt = 0; usCnt < usSize; usCnt++) {
		ucWork = pucInput[usCnt];
		if (ucWork == CHR_DLE) {			// データが0x10ならば0x10を付加 
			*pucWrite = CHR_DLE;			// DLE付加 
			pucWrite++;						// 書き込み先 
			usRealSize++;					// 実サイズ
			// BCCは計算しない!
		}
		*pucWrite = ucWork;					// データ 
		ucBCC ^= ucWork;					// BCC 
		pucWrite++;							// 書き込み先 
		usRealSize++;						// 実サイズ 
	}
	
	*pucWrite = CHR_DLE;					// DLE 
	pucWrite++;
	*pucWrite = CHR_ETX;					// ETX 
	ucBCC ^= CHR_ETX;						// BCC計算 
	pucWrite++;
	*pucWrite = ucBCC;						// BCC付加 
	usRealSize += 3;
	
	Comm_SendData(&CommSendBuff[0], usRealSize);
	
	return OK;
}

void SerialStart(void)
{
	USHORT len;
	
	printf("Start\n");
	len = 0x04;								// データ長
	SendBuff[0] = len;						// レングス
	SendBuff[1] = 0xFF;						// センサNo.
	SendBuff[2] = CMD_DATA_START;			// コマンド種別
	SendBuff[3] = 0;						// 予備
	
	SendData(SendBuff, len);
}

void SerialStop(void)
{
	USHORT len;
	
	printf("Stop\n");
	len = 0x04;								// データ長
	SendBuff[0] = len;						// レングス
	SendBuff[1] = 0xFF;						// センサNo.
	SendBuff[2] = CMD_DATA_STOP;			// コマンド種別
	SendBuff[3] = 0;						// 予備
	
	SendData(SendBuff, len);
}

