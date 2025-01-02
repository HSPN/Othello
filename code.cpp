// 오델로게임. 플레이어가 흑, 컴퓨터가 백을 잡는다.

#include<windows.h>
#include<tchar.h>
#include<fstream>
#include<cstdlib>

using namespace std;

#define ID_SAVE 1
#define ID_LOAD 2
#define ID_START 3
#define ID_UNDO 4
#define ID_EXIT 5
#define ID_LEVEL 6
#define ID_EDIT 999
#define ID_LEVEL3 401
#define ID_LEVEL5 402
#define ID_SCORE 500
#define ID_TURN 501

#define MAP_X 8
#define MAP_Y 8

#define HORIZON 1
#define VERTICAL 2
#define R_SLASH 3
#define SLASH 4



HINSTANCE g_hInst;


HWND hScore;
HWND hTurn;
HWND hEdit;
HWND button[MAP_X * MAP_Y];	//게임판 버튼의 핸들이 저장되는곳
char gameCal[MAP_X * MAP_Y]; // 게임판의 계산정보 저장. 0 = 빈칸, 1 = 흑, 2 = 백
char stoneOrder[MAP_X * MAP_Y -4]; // 놓은 순서를 저장하는 배열
LPWSTR lpszClass = L"PL";
int level = 3;	// 몇수를 내다볼건지 저장하는 변수
int stoneNum = 4; // 지금까지 놓은 돌의 개수
bool playerTurn = false;	// true일때 플레이어가 둘 수 있다. 플레이어의 턴에만 버튼 조작이 가능.
bool isPlaying = false;
LPTSTR str;	// 불러온 파일이 저장되는 변수

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM); // 각종 신호에 반응하여 신호에 맞는 동작을 하는 함수
int AutoPlayer(int level,char gameCal[],bool isPlayer);	//level만큼의 수를 내다보고 둘 위치를 결정하여 index로 리턴한다.
int calGame_finish(char gameCal[]);	//밑의 함수와 동일하나, 게임 막바지에 쓰이는 계산함수
int calGame(char gameCal[]);	// 게임판의 판도를 계산하여 점수를 냄
void ChangeAllColor(int x1,int y1,int x2, int y2,bool isPlayer,bool isReal,char gameCal[]);	// 두 돌의 좌표를 받아 사이의 돌들의 주인을 전부 바꿈
int myCheck(int x,int y,bool isPlayer,char gameCal[]);	// 돌을 둘 수 있는곳인지 검사하고, 둘수 있다면 반응하는 돌을 리턴
bool CheckAllLine(bool isPlayer,char gameCal[]);	//둘 곳이 있는지 검사
void ChangeColor(int x,int y,bool isPlayer);	//하나의 돌의 주인을 바꿈
void saveGame();
void loadGame();
void setBoard();		// 게임의 상황을 알려주는 전광판을 갱신
void undo();		//한수무르기

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam,int nCmdShow)
{	
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;
	for(int i=0;i<MAP_X*MAP_Y-4;i++) stoneOrder[i] = '\0';
	str = (LPTSTR)malloc(500);
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName=NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass,lpszClass,WS_CAPTION | WS_SYSMENU, 
		200,200,400,400,NULL,(HMENU)NULL,hInstance,NULL);
	ShowWindow(hWnd,nCmdShow);

	while(GetMessage(&Message,0,0,0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int wparam;
	switch(iMessage)
	{
	case WM_CREATE :
		CreateWindow(L"button",L"Save",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0,0,90,70,hWnd,(HMENU)ID_SAVE, g_hInst,NULL);
		CreateWindow(L"button",L"Load",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			90,0,90,70,hWnd,(HMENU)ID_LOAD, g_hInst,NULL);
		CreateWindow(L"button",L"Start",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0,150,180,70,hWnd,(HMENU)ID_START, g_hInst,NULL);
		CreateWindow(L"button",L"Undo",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0,220,180,70,hWnd,(HMENU)ID_UNDO, g_hInst,NULL);
		CreateWindow(L"button",L"Exit",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0,290,180,70,hWnd,(HMENU)ID_EXIT, g_hInst,NULL);
		//	버튼들을 생성하는 코드

		hEdit = CreateWindow(L"Edit",L"File Name",WS_CHILD | WS_VISIBLE | WS_BORDER |
			ES_AUTOVSCROLL | ES_MULTILINE, 0,70,180,70,hWnd,(HMENU)ID_EDIT,g_hInst,NULL);
		// 파일명을 입력받는 에디터박스를 만드는 코드

		CreateWindow(L"button",L"Level",WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			180,0,200,50,hWnd,(HMENU)ID_LEVEL, g_hInst,NULL);
		CreateWindow(L"button",L"3",WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			190,15,30,25,hWnd,(HMENU)ID_LEVEL3, g_hInst,NULL);
		CreateWindow(L"button",L"5",WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
			280,15,30,25,hWnd,(HMENU)ID_LEVEL5, g_hInst,NULL);
		CheckRadioButton(hWnd,ID_LEVEL3,ID_LEVEL5,ID_LEVEL3);
		//	라디오버튼(레벨3, 레벨5) 생성 코드

		hScore = CreateWindow(L"static",L"흑 : 00 백 : 00",WS_CHILD | WS_VISIBLE | SS_CENTER,
			185,70,190,25,hWnd,(HMENU)ID_SCORE, g_hInst,NULL);
		hTurn = CreateWindow(L"static",L"Load a File",WS_CHILD | WS_VISIBLE | SS_CENTER,
			185,95,190,25,hWnd,(HMENU)ID_LEVEL3, g_hInst,NULL);

		for(int i=0;i<MAP_Y;i++)
			for(int j=0;j<MAP_X;j++)
				button[i*MAP_X+j] = CreateWindow(L"button",L"",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			185+24*j,160+24*i,24,24,hWnd,(HMENU)(100 + j + i*MAP_X), g_hInst,NULL);	// 100~ ID로 칸 생성
		//	게임판버튼을 생성하는 코드
		return 0;
	case WM_COMMAND :
		wparam = LOWORD(wParam);
		switch(wparam)
		{
		case ID_SAVE :
			if(isPlaying)
				saveGame();
			break;
		case ID_LOAD :
			loadGame();
			isPlaying = true;
			playerTurn = false;
			setBoard();
			SetWindowText(hTurn,TEXT("Player Turn"));
			break;
		case ID_LEVEL3 :
			level = 3;
			break;
		case ID_LEVEL5 :
			level = 5;
			break;
		case ID_START :
			if(isPlaying)
				playerTurn = true;
			break;
		case ID_UNDO :
			undo();
			break;
		case ID_EXIT :
			exit(0);
		}

		if(playerTurn && wparam > 99 && wparam < 100+MAP_X*MAP_Y )	//버튼 이벤트(100~에 칸 할당)
		{
			switch(HIWORD(wParam))
			{
			case BN_CLICKED :			//게임판버튼이 클릭되면
				int x = (wparam - 100) % MAP_X;
				int y = (wparam- 100) / MAP_X;
				int index;
				if( (index = myCheck(x,y,playerTurn,gameCal)) != -1 && gameCal[x+y*MAP_X] == 0)	// 유효한 곳인지 판명하고
				{
					gameCal[y*MAP_X+x] = 1;
					stoneOrder[strlen(stoneOrder)] = (y*MAP_X+x+1); //플레이어가 둔 곳 저장 (양수)
					do ChangeAllColor(x,y,index%MAP_X,index/MAP_X,playerTurn,true,gameCal);	//유효할경우 돌을 둔 뒤 처리한다.
					while( (index = myCheck(x,y,playerTurn,gameCal)) != -1);
					stoneNum++;
					playerTurn = !playerTurn;// 컴퓨터의 턴
					SetWindowText(hTurn,TEXT("Computer Turn"));
				}
				else break;
				while(true)
				{
					Sleep(1000); // 1초후 인공지능 시작
					char gameCal_AI[MAP_X*MAP_Y];
					for(int i=0;i<MAP_Y;i++)		//인자로 받은 맵을 계산용 AI맵에 복사
						for(int j=0;j<MAP_X;j++)
							gameCal_AI[j+i*MAP_X] = gameCal[j+i*MAP_X];
					index = AutoPlayer(level,gameCal_AI,false);		//level만큼 계산한 결과로 나온 최적의 수를 리턴
					x = index % MAP_X;
					y = index / MAP_X;
					gameCal[y*MAP_X+x] = 2;
					if(index >= 0)		//AI가 낸 수가 유효한지 검사 (둘곳이 없을경우 음수를 리턴하기 때문)
					{
						stoneOrder[strlen(stoneOrder)] = -(index+1); //컴퓨터가 둔 곳 저장 (음수)
						index = myCheck(x,y,playerTurn,gameCal);
						do ChangeAllColor(x,y,index%MAP_X,index/MAP_X,playerTurn,true,gameCal);	//돌을 두고 처리
						while( (index = myCheck(x,y,playerTurn,gameCal)) != -1);
						stoneNum++;
					}
					else MessageBox(hWnd,L"AI가 둘 곳이 없어 한 턴 쉽니다.",L"알림",MB_OK);	//둘 곳이 없었던 경우 돌을 안둠
					playerTurn = !playerTurn;	//플레이어의 턴
					SetWindowText(hTurn,TEXT("Player Turn"));
					if(!CheckAllLine(playerTurn,gameCal))	// 플레이어가 둘곳이 없는경우 AI가 또 둔다.
					{
						MessageBox(hWnd,L"플레이어가 둘 곳이 없어 턴이 넘어갑니다.",L"알림",MB_OK);
						playerTurn = !playerTurn;	//다시 컴퓨터의 턴
						SetWindowText(hTurn,TEXT("Computer Turn"));
						if(!CheckAllLine(playerTurn,gameCal)) // 플레이어도 컴퓨터도 둘 곳이 없는 경우 게임종료
						{
							MessageBox(hWnd,L"양 플레이어 모두 둘 곳이 없어 게임이 종료됩니다.",L"알림",MB_OK);
							int stone_c = 0;
							int stone_p = 0;	
							for(int i=0;i<MAP_Y;i++)
							{
								for(int j=0;j<MAP_X;j++)
								{
									if( gameCal[j+i*MAP_X] == 1) stone_p++;
									if( gameCal[j+i*MAP_X] == 2) stone_c++;
								}
							}
							if(stone_p > stone_c) SetWindowText(hTurn,TEXT("End. Winner is Player!"));
							else if(stone_p < stone_c) SetWindowText(hTurn,TEXT("End. Winner is Computer!"));
							else SetWindowText(hTurn,TEXT("End. Draw!"));
							break;
						}
						continue;	// 컴퓨터의 턴 반복
					}
					break;
				}
				break;
			}	
			setBoard();
		}
		return 0;
	case WM_DESTROY :
		PostQuitMessage(0);
		return 0;
	}
	return (DefWindowProc(hWnd,iMessage,wParam,lParam));
}

int AutoPlayer(int level,char gameCal[],bool isPlayer)
{
	char gameCal_AI[MAP_X * MAP_Y];
	int MinMaxScore;
	int x = -1;
	int y = -1;
	int score;
	if(isPlayer) MinMaxScore = -99999999;	//플레이어는 최대점수, 컴퓨터는 최소점수를 선택하는 MinMax알고리즘
	else MinMaxScore = 99999999;

	for(int i=0;i<MAP_Y;i++)		//인자로 받은 맵을 계산용 AI맵에 복사
		for(int j=0;j<MAP_X;j++)
			gameCal_AI[j+i*MAP_X] = gameCal[j+i*MAP_X];
	
	for(int i=0;i<MAP_Y;i++)
	{
		for(int j=0;j<MAP_X;j++)	//맵전체에 대해
		{
			if(gameCal[j + i*MAP_X] == 0 && myCheck(j,i,isPlayer,gameCal) >= 0)	// 유효한장소만 검사
			{
				for(int i2=0;i2<MAP_Y;i2++)		//인자로 받은 맵을 계산용 AI맵에 복사
					for(int j2=0;j2<MAP_X;j2++)
						gameCal_AI[j2+i2*MAP_X] = gameCal[j2+i2*MAP_X];
				gameCal_AI[j + i*MAP_X] = 2;
				if( stoneNum + ::level <= MAP_X*MAP_Y && level > 1 ) // 재귀할게 남았고, 계산이 마지막턴까지가 아닌경우 계속 재귀함.
					AutoPlayer(level-1,gameCal_AI,!isPlayer);
				if( stoneNum + ::level < MAP_X * MAP_Y) //마지막턴까지 계산하지 않는 경우
					score = calGame(gameCal_AI);		//판의 점수를 매김. 돌의 위치에 따라 가중치가 있음.
				if( stoneNum + ::level >= MAP_X * MAP_Y) //마지막턴까지 계산하는 경우
					score = calGame_finish(gameCal_AI);	//판의 점수를 매김. 단,돌의 갯수만 셈.
				if(isPlayer && MinMaxScore < score)	//판의 점수를 매겨서 MinMax알고리즘에 따라 갱신
				{
					MinMaxScore = score;
					x = j;
					y = i;
				}
				else if( !isPlayer && MinMaxScore > score)
				{
					MinMaxScore = score;
					x = j;
					y = i;
				}
			}
		}
	}
	if(x != -1)	// 놓을 말이 있는경우 리턴 이전에 정보 갱신. 
	{
		if(isPlayer) gameCal[x + y*MAP_X] = 1;
		else gameCal[x + y*MAP_X] = 2;
	}
	return (x + y*MAP_X);	//놓을 말의 인덱스 리턴
}

int calGame(char gameCal[])
{
	int score = 0;

	for(int i=0;i<MAP_Y;i++)
	{
		for(int j=0;j<MAP_X;j++)
		{
			if(gameCal[j + i*MAP_X] == 1)
			{
				if(j==0 || j==MAP_X-1)	//세로 테두리선인경우
				{
					if(i==0 || i==MAP_Y-1)	//구석자리인경우
						score += (MAP_X+MAP_Y-4);
					else if(i == 1 || i == MAP_Y-2) // 구석 직전위치인경우
						score -= ( (MAP_X+MAP_Y-4)/2 );
					else
						score += ( (MAP_X+MAP_Y-4)/2 );//그냥 테두리선인경우
				}
				else if(j==1 || j==MAP_Y-2)
				{
					if(i==0 || i==MAP_Y-1)	//구석직전위치인경우
						score -= ( (MAP_X+MAP_Y-4)/2 );
					else if(i== 1 || i==MAP_Y-2)	//구석에서 대각선으로 한칸떨어진 위치인경우
						score -= (MAP_X+MAP_Y-4);
					else
						score -= ((MAP_X+MAP_Y-4)/4);
				}
				else
				{
					if(i == 0 || i == MAP_Y-1)
						score += ( (MAP_X+MAP_Y-4)/2);
					else if(i==1 || i==MAP_Y-2)
						score -= ( (MAP_X + MAP_Y-4)/4);
					else
						score++;
				}
			}
			else if(gameCal[j + i*MAP_X] == 2)
			{
				if(j==0 || j==MAP_X-1)	//세로 테두리선인경우
				{
					if(i==0 || i==MAP_Y-1)	//구석자리인경우
						score -= (MAP_X+MAP_Y-4);
					else if(i == 1 || i == MAP_Y-2) // 구석 직전위치인경우
						score += ( (MAP_X+MAP_Y-4)/2 );
					else
						score -= ( (MAP_X+MAP_Y-4)/2 );//그냥 테두리선인경우
				}
				else if(j==1 || j==MAP_Y-2)
				{
					if(i==0 || i==MAP_Y-1)	//구석직전위치인경우
						score += ( (MAP_X+MAP_Y-4)/2 );
					else if(i== 1 || i==MAP_Y-2)	//구석에서 대각선으로 한칸떨어진 위치인경우
						score += (MAP_X+MAP_Y-4);
					else
						score += ((MAP_X+MAP_Y-4)/4);
				}
				else
				{
					if(i == 0 || i == MAP_Y-1)
						score -= ( (MAP_X+MAP_Y-4)/2);
					else if(i==1 || i==MAP_Y-2)
						score += ( (MAP_X + MAP_Y-4)/4);
					else
						score--;
				}
			}
		}
	}
	return score;
}

int calGame_finish(char gameCal[])
{
	int score = 0;

	for(int i=0;i<MAP_Y;i++)
	{
		for(int j=0;j<MAP_X;j++)
		{
			if(gameCal[j + i*MAP_X] == 1)
				score++;
			else if(gameCal[j + i*MAP_X] == 2)
				score--;
		}
	}
	return score;
}

void ChangeAllColor(int x1,int y1,int x2, int y2, bool isPlayer,bool isReal,char gameCal[])	//두 돌의 좌표를 받아 사이의 돌들을 전부 바꿈 
{
	int minX,minY,maxX,maxY; // 각 라인 검사시마다, 두 숫자를 비교하여 큰숫자와 작은숫자를 둠.
	int check;	// 가로, 세로, 슬래쉬, 역슬래쉬 방향을 검사하여 저장

	x1 < x2 ? (minX = x1,maxX = x2) : (minX = x2, maxX = x1);
	y1 < y2 ? (minY = y1,maxY = y2) : (minY = y2, maxY = y1);

	if ( y1 != y2 )			// 받은 좌표를 이용해 고려해야될 방향을 지정
	{
		if(x1 == x2) check = VERTICAL;
		else if( (x1 > x2) == (y1 > y2) ) check = R_SLASH;
		else check = SLASH;
	}
	else check = HORIZON;

	int distance;
	(check == HORIZON) ? (distance = maxX-minX) : (distance = maxY - minY);	//두 돌이 몇칸 떨어져있는지 저장
	
	if(isReal)
		for(int j=0; j<=distance; j++)	//두 돌 사이에 있는 돌들의 주인을 실제로 바꿈
		{
			if(check == SLASH) ChangeColor(maxX-j,minY+j,isPlayer);
			if(check == VERTICAL) ChangeColor(minX, minY+j,isPlayer);
			if(check == HORIZON) ChangeColor(minX+j, minY,isPlayer);
			if(check == R_SLASH) ChangeColor(minX+j, minY+j,isPlayer);		
		}
	else
		for(int j=0; j<=distance; j++)	//두 돌 사이에 있는 돌들의 주인을 인자로 받은 배열에서만 바꿈
		{
			if(check == SLASH)
			{
				if(isPlayer) gameCal[maxX-j,(minY+j)*MAP_X] = 1;
				else gameCal[maxX-j,(minY+j)*MAP_X] = 2;
			}
			if(check == VERTICAL) 
			{
				if(isPlayer) gameCal[maxX,(minY+j)*MAP_X] = 1;
				else gameCal[maxX,(minY+j)*MAP_X] = 2;
			}
			if(check == HORIZON)
			{
				if(isPlayer) gameCal[maxX+j,minY*MAP_X] = 1;
				else gameCal[maxX+j,minY*MAP_X] = 2;
			}
			if(check == R_SLASH)		
			{
				if(isPlayer) gameCal[maxX+j,(minY+j)*MAP_X] = 1;
				else gameCal[maxX+j,(minY+j)*MAP_X] = 2;
			}
		}
}

bool CheckAllLine(bool isPlayer,char gameCal[])	//모든라인을 검사. 놓을곳이 한곳이라도 있으면 true
{
	for(int i=0;i<MAP_Y;i++)	// 칸 검사
		for(int j=0;j<MAP_X;j++)
			if(gameCal[j + i*MAP_X] == 0 && myCheck(j,i,isPlayer,gameCal) >= 0) return true;
	return false;
}

int myCheck(int x, int y,bool isPlayer,char gameCal[])	//해당위치에 돌을 놓을수 있는지 검사. 놓을 수 있으면 반응하는 돌의 좌표리턴, 아니면 -1리턴
{
	int pNum = 1;	// Player Number
	int eNum = 2;	// Enemy Number
	int minX,minY,maxX,maxY; // 각 라인 검사시마다, 두 숫자를 비교하여 큰숫자와 작은숫자를 둠.
	int tempX, tempY; // 여러 인덱스를 검사하기 위해 이 값을 조금씩 증가시키며 검사한다.
	
	if(!isPlayer)	// 1이 흑, 2가 백이다. 색깔구분용 Number
	{
		pNum = 2;
		eNum = 1;
	}

	for(int check=1;check <=4; check++)	// 가로, 세로, 슬래쉬방향, 역슬래쉬방향(총 4개) 검사.
	{
		tempX = x;
		tempY = y;
		while(true)	// 검사할 방향에서 인덱스가 작아지는쪽으로 이동. 단, 게임판을 벗어나지 않게.
		{
			if(check == SLASH && (tempX >= MAP_X-1 || tempY <= 0)) break;	//더 이동하면 게임판을 벗어나므로 break
			else if(check == R_SLASH && (tempX <= 0 || tempY <= 0)) break;
			else if(check == VERTICAL && (tempY <= 0)) break;
			else if(check == HORIZON && (tempX <= 0)) break;

			if(check == HORIZON || check == R_SLASH) tempX--;				//인덱스가 작아지는쪽으로 이동.
			if(check != HORIZON) tempY--;
			if(check == SLASH) tempX++;
		}

		while(true)	//검사 시작
		{
			if(check == SLASH && (tempX < 0 || tempY > MAP_Y-1)) break;		//게임판을 벗어나게되면 break
			else if(check != SLASH && (tempX > MAP_X-1 || tempY > MAP_Y-1)) break;

			if(gameCal[tempX + tempY * MAP_X] == pNum)	// 자신의 돌이 발견되면
			{
				int distance;

				tempY < y ? (minY = tempY,maxY = y) : (minY = y, maxY = tempY);	// 놓은 돌과 발견한 돌의 xy값을 대소비교한다.
				tempX < x ? (minX = tempX,maxX = x) : (minX = x, maxX = tempX);

				(check == HORIZON) ? (distance = maxX-minX) : (distance = maxY - minY);	//두 돌이 몇칸 떨어져있는지 저장
				if(distance >= 2)		//두 돌은 같은돌이면 안되고, 최소한 한칸 떨어져있어야한다.
				{
					int j;
					for(j=1; j<distance; j++)	//두 돌 사이가 적의돌로 차있는지 검사
					{
						if(check == SLASH && gameCal[maxX-j + (minY+j)*MAP_X] != eNum) break;
						if(check == VERTICAL && gameCal[minX + (minY+j)*MAP_X] != eNum) break;
						if(check == HORIZON && gameCal[minX+j + minY*MAP_X] != eNum) break;
						if(check == R_SLASH && gameCal[minX+j + (minY+j)*MAP_X] != eNum) break;
						
					}
					if(j == distance) return tempX+tempY*MAP_X;	//사이에 적의 돌만 있는경우 인덱스 리턴
				}
			}
			if(check == HORIZON || check == R_SLASH) tempX++;	//그다음칸 검사를 위해 인덱스 추가
			if(check != HORIZON) tempY++;
			if(check == SLASH) tempX--;
		}
	}
	return -1;	// 위의 검사 결과, 반응할 돌이 발견되지 않는 경우 -1 리턴
}

void ChangeColor(int x,int y,bool isPlayer)
{
	if(isPlayer)
	{
		gameCal[x + y*MAP_X] = 1;
		SetWindowText(button[x + y*MAP_X],TEXT("●"));
	}
	else
	{
		gameCal[x + y*MAP_X] = 2;
		SetWindowText(button[x + y*MAP_X],TEXT("○"));
	}
}

void saveGame()
{
	int index,x,y;
	ofstream fout;
	fout.open(str);
	for(int i=0;i<MAP_X*MAP_Y; i++)	// 판 정보 저장
	{
		if(gameCal[i] == 0) fout << '0';
		else if(gameCal[i] == 1) fout << 'B';
		else if(gameCal[i] == 2) fout << 'W';
		//fout << gameCal[i];
	}
	if(playerTurn) fout << " B\n";
	else fout << " W\n";
	for(int i=0;i<strlen(stoneOrder);i++)
	{
		index = stoneOrder[i];
		if(stoneOrder[i] < 0) index = -index;
		index--;
		
		x = index % MAP_X;
		y = index / MAP_X;
		if(stoneOrder[i] < 0) fout << 'W';
		else fout << 'B';
		fout << '<' << x << ',' << y << "> "; 
	}
	fout.close();
}

void loadGame()
{
	int i,x,y;
	char buf;	// 주로 안쓰는 값을 받아내는데에 쓰임
	char player; // 두번째줄 읽을때 W,B 구분용
	ifstream fin;
	GetWindowText(hEdit,str,255);
	fin.open(str);
	stoneNum = 0;
	for(i=0;i<MAP_X * MAP_Y-4;i++)
		stoneOrder[i] = '\0';
	for(i=0;i<MAP_X*MAP_Y;i++)	// 세이브된 파일을 통해 판 복구
	{
		gameCal[i] = -1;
		fin >> gameCal[i];
		if(gameCal[i] == '0')
		{
			SetWindowText(button[i],TEXT(" "));
			gameCal[i] = 0;
		}
		else if(gameCal[i] == 'B') 
		{
			SetWindowText(button[i],TEXT("●"));
			gameCal[i] = 1;
			stoneNum++;
		}
		else if(gameCal[i] == 'W') 
		{
			SetWindowText(button[i],TEXT("○"));
			gameCal[i] = 2;
			stoneNum++;
		}
		else break;
	}
	if(i != MAP_X*MAP_Y)	//세이브된 파일이 아닌경우 초기화
	{
		stoneNum = 4;
		for(i=0;i<MAP_X * MAP_Y;i++)
		{
				gameCal[i] = 0;
				SetWindowText(button[i],TEXT(" "));
		}
		gameCal[MAP_X/2-1 + (MAP_Y/2-1)*MAP_X] = gameCal[MAP_X/2 + MAP_Y/2 * MAP_X] = 2;
		gameCal[MAP_X/2 + (MAP_Y/2-1)*MAP_X] = gameCal[MAP_X/2-1 + MAP_Y/2*MAP_X] = 1;
		SetWindowText(button[MAP_X/2-1 + (MAP_Y/2-1)*MAP_X],TEXT("○"));
		SetWindowText(button[MAP_X/2   + MAP_Y/2*MAP_X],TEXT("○"));
		SetWindowText(button[MAP_X/2   + (MAP_Y/2-1) * MAP_X],TEXT("●"));
		SetWindowText(button[MAP_X/2-1 + MAP_Y/2 * MAP_X],TEXT("●"));
	}
	setBoard();
	fin >> player; // 누구턴인지 받음
	if(player == 'W') playerTurn = false;
	else if(player == 'B') playerTurn = true;
	for(int i=0;i<stoneNum-4;i++)
	{
		fin >> player; // 누구턴인지 받음

		fin >> buf; // <를 받음

		fin >> x; // x를받음

		fin >> buf; // ,를 받음

		fin >> y; // y를받음

		fin >> buf; // >를받음
		
		if(player == 'B') stoneOrder[i] = y*MAP_X + x + 1;
		else if(player == 'W') stoneOrder[i] = -(y*MAP_X + x + 1);
	}
	fin.close();
}

void setBoard()
{
	int stone_c = 0;
	int stone_p = 0;
	char buf[100] = "P : 00 C : 00";
	
	LPWSTR x;
	x = (LPWSTR)malloc(100);

	for(int i=0;i<MAP_Y;i++)	// 점수를 재기 위해 돌을 셈
	{
		for(int j=0;j<MAP_X;j++)
		{
			if( gameCal[j+i*MAP_X] == 1) stone_p++;
			if( gameCal[j+i*MAP_X] == 2) stone_c++;
		}
	}
	buf[4] = stone_p/10 + '0';
	buf[5] = stone_p%10 + '0';
	buf[11] = stone_c/10 + '0';
	buf[12] = stone_c%10 + '0';

	mbstowcs(x,buf,100);		// char*를 LPWSTR로 바꿔주는 함수
	SetWindowText(hScore,x);
	free(x);
}

void undo() //무르기함수. 둔 순서를 배열로 가지고있는점을 이용해, 한번 누를때마다 판을 초기화하고 한턴 전 상황까지 다시 둔다.
{
	if(strlen(stoneOrder) <= 0) return;	//더 무를곳이 없는 경우 리턴
	int index,x,y;
	for(;;)
	{
		stoneNum = 4;
		for(int i=0;i<MAP_X * MAP_Y;i++)	//판을 처음상태로 초기화
		{
				gameCal[i] = 0;
				SetWindowText(button[i],TEXT(" "));
		}
		gameCal[MAP_X/2-1 + (MAP_Y/2-1)*MAP_X] = gameCal[MAP_X/2 + MAP_Y/2 * MAP_X] = 2;
		gameCal[MAP_X/2 + (MAP_Y/2-1)*MAP_X] = gameCal[MAP_X/2-1 + MAP_Y/2*MAP_X] = 1;
		SetWindowText(button[MAP_X/2-1 + (MAP_Y/2-1)*MAP_X],TEXT("○"));
		SetWindowText(button[MAP_X/2 + MAP_Y/2*MAP_X],TEXT("○"));
		SetWindowText(button[MAP_X / 2 + (MAP_Y / 2 - 1) * MAP_X], TEXT("●"));
		SetWindowText(button[MAP_X / 2 - 1 + MAP_Y / 2 * MAP_X], TEXT("●"));

		for(int i=0;i<strlen(stoneOrder)-1;i++)	//로드한 뒤의 기록을 통해 undo
		{
			if(stoneOrder[i] > 0)	// 플레이어가 놓는 경우
			{
				index = stoneOrder[i]-1;
				x = index%MAP_X;
				y = index/MAP_X;
				do ChangeAllColor(x,y,index%MAP_X,index/MAP_X,true,true,gameCal);	//돌을 두고 처리
				while( (index = myCheck(x,y,true,gameCal)) != -1);
				stoneNum++;
			}
			else             //컴퓨터가 놓는 경우
			{
				index = -(stoneOrder[i]+1);
				x = index%MAP_X;
				y = index/MAP_X;
				do ChangeAllColor(x,y,index%MAP_X,index/MAP_X,false,true,gameCal);	//돌을 두고 처리
				while( (index = myCheck(x,y,false,gameCal)) != -1);
				stoneNum++;
			}
		}
		if(stoneOrder[strlen(stoneOrder)-1] > 0)
		{
			stoneOrder[strlen(stoneOrder)-1] = '\0';
			break;
		}
		else
		{
			stoneOrder[strlen(stoneOrder)-1] = '\0';
			continue;
		}
	}

	setBoard();
}
