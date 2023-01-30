 
#include <ctime> 
#include <conio.h>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <limits>

int k = 0; // можно попробовать исползовать для еды

bool contornot (); // если вернет 0, то конец игры, если 1, то вызовем снов функцию headcoord() 
void writescore ();
void showscore (); 
void readscore ();
void clearfood ();
void drawfood (); // будет рисовать еду после паузы 
bool foodeaten (int*, int [], unsigned int*, int*); // будет менять значение к  
bool polygonrestriction ();
void food (COORD);
bool pause ();
void gameover ();
int headcoord ();
void polygon (WORD);
bool body (int, int [], int [], int [], COORD, COORD, int);
void specialrestriction (double*, int*);
void clear (int, int [], COORD, COORD);
bool intersection (int, int [], int []);


void GotoLine(std::fstream& file, unsigned int num) // будет бегать к нужной строке в файле
{
    file.seekg(std::ios::beg);
    for(int i=0; i < num - 1; ++i){
        file.ignore(std::numeric_limits < std::streamsize > ::max(), '\n'); // добавил std:: 
    }
}

HWND console;
HANDLE hin = GetStdHandle (STD_INPUT_HANDLE);
HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
WORD tailcolor, Attributes, yellowfontblacktext; // мы сюда будем складывать цвет хвоста и черный экран для бекапа 
INPUT_RECORD InputRecord;
DWORD Events;
CONSOLE_CURSOR_INFO cci;

COORD cursorpos;
COORD headpos;
COORD foodpos;
COORD scoretablepos;
COORD scorepos;
COORD decisionpos; 

unsigned int currentscore = 0, previousscore = 0, bestscore = 0;

std::fstream fsold;
std::ifstream file;
std::fstream fsnew;

COORD debugcord = {10,28};
int main () 
{
	srand (time(0));
    console = GetConsoleWindow(); // получаем handle to the window 
    RECT ConsoleRect; // структура rect держит в себе координаты прямоугольника  
    // далее только х и у используем верхние левые 
    GetWindowRect (console, &ConsoleRect); 
    //GetWindowRect () возвращает размеры окна консольного в структуру rect, первый аргумент 
    //это handle to the window   
    
	MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 1000, 550, TRUE);
    // сделаем курсор невидимым 
    cci.bVisible = false; cci.dwSize = 25; // делаем невидимым 
	SetConsoleCursorInfo (hout, &cci);

    GetConsoleScreenBufferInfo (hout, &csbiInfo);
	WORD Attributes = csbiInfo.wAttributes; // атрибуты для бекапа сохранили
    
    headcoord (); 	
	return 0;
} 
int headcoord () 
{
    
    SetConsoleTextAttribute (hout, Attributes);
    SetConsoleTextAttribute (hout, BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN);
    system ("cls");	
    
	GetConsoleScreenBufferInfo (hout, &csbiInfo);
	yellowfontblacktext = csbiInfo.wAttributes; // атрибуты для заливки фона 

	
	   
    headpos.X = 15; headpos.Y = 10; // начальные координаты
    readscore ();
    
	int arraywithX [90], arraywithY [90]; // в них будут координаты хвоста, чтобы было 
	                                      // легко проверить пересечение    
    
	// 1-вверх 2-влево 3-вниз 4-вправо
    int arraywithdirections [90] = {2,2,2,2,2}; 
  
    // увеличим доступную длину змеи
	int amountofelements = 6; // в любом случае 2 клетки, по х или у 
	
	double keypressed; // просто костыль
    int head_direction = 77; // в-вправо ы-вниз ф-влево ц-вверх (код в аске)  
    
    int sleepmil = 350;
  
   	polygon (Attributes);
	 
	while (keypressed != 27) // пока не нажат escape
	{
      	if (kbhit())
		{
			
	    	keypressed = getch();
    		if ((keypressed != 224 ) && (keypressed != 27)) /*&& (keypressed != 235 ) && (keypressed != 228 ) && (keypressed != 230 ))*/
			{
				if (keypressed == 32) // отрабатывает пауза 
				{   
				    if (pause ())
			    	{
				        SetConsoleCursorPosition (hout, cursorpos);
                      //  drawfood (); переместил в pause ()    
			        }
			        else 
			           
					    return 0;
				}

				keypressed = head_direction; // это мы делаем, чтобы обезопасить себя от левых нажатий
				
			}
			
           	else if (keypressed == 224) //|| (keypressed == 72) || (keypressed == 77) || (keypressed == 80) || (keypressed == 75)) 
			{
				keypressed = getch(); // тут уже будет значение другое от 224
                specialrestriction (&keypressed, &head_direction);
			} 
			}
		    switch (head_direction)
			{
			    case 72 :
			    	headpos.Y -= 1; 
		            arraywithdirections [amountofelements - 2] = 3; // заполняем массив с направленими
					break;
				case 77 :
				    headpos.X += 2; // пробелы по иксу не равны пробелам по игрику 
		            arraywithdirections [amountofelements - 2] = 2; // заполняем массив с направленими 	 
                    break;
	            case 80 :
				    headpos.Y += 1; 
	                arraywithdirections [amountofelements - 2] = 1; // заполняем массив с направленими
					break;      
		        case 75 :
		        	headpos.X -= 2; 
		            arraywithdirections [amountofelements - 2] = 4; // заполняем массив с направленими
					break;
		    }
	    
 
      	if (body (amountofelements, arraywithdirections, arraywithX, arraywithY, cursorpos, headpos, sleepmil/*, k*/) == 0)
    	{
			return 0;
	    } 

		
	    if (foodeaten (&amountofelements, arraywithdirections, &currentscore, &sleepmil))
		
		    showscore ();
	
	}
    writescore ();
}
 // headpos передаем из прошлой функции, в ней хранятся координаты головы 
bool body (int amountofelements, int arraywithdirections [],int arraywithX [], int arraywithY [],COORD cursorpos, COORD headpos, int sleepmil/*, int k*/) 
{
	// индекс елемента именно -1, потому что по сравнению с массивом с направлениями 
	// в массивах с координатами будет на 1 елемент больше 
    arraywithX [amountofelements - 1] = headpos.X; // загнали в массив с координатами
    arraywithY [amountofelements - 1] = headpos.Y; // расположение головы 
 
    // в массиве с направлениями мы идем с конца, 
    // в массиве с координатами тоже будем идти с конца
	// ВЫСТАВИМ ЦВЕТ ГОЛОВЫ 
	SetConsoleTextAttribute (hout, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	headpos.X --;
	SetConsoleCursorPosition (hout, headpos); // выставляем курсор на коорд. головы 
    std::cout << "\xB1\xB1";
	cursorpos = headpos;
    headpos.X ++;	
	

	// выставляем цвет хвоста змеи
	SetConsoleTextAttribute (hout, FOREGROUND_INTENSITY | FOREGROUND_RED | BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN); 
	GetConsoleScreenBufferInfo (hout, &csbiInfo);
	
	tailcolor = csbiInfo.wAttributes; // сохранили  
	
	SetConsoleTextAttribute (hout, tailcolor);
	// ТЕПЕРЬ МОЖЕМ РИСОВАТЬ ХВОСТ 
	
	for (int i = amountofelements - 2; i >= 0; i--) //голова готова, поэтому <, начинаем с еденицы
	{
        
		
			if (arraywithdirections [i] == 1)
		    {
		 	cursorpos.Y --;
		 	SetConsoleCursorPosition (hout, cursorpos);
	        }
		 	
		    else if (arraywithdirections [i] == 2)
		    {
		 	cursorpos.X -= 2;
		 	SetConsoleCursorPosition (hout, cursorpos);
		    }
		    else if (arraywithdirections [i] == 3)
		    {
		 	cursorpos.Y ++;
		 	SetConsoleCursorPosition (hout, cursorpos);
		    }	
		    else if (arraywithdirections [i] == 4)
		    {
		 	cursorpos.X += 2;
		 	SetConsoleCursorPosition (hout, cursorpos);
	        }		
	        
			
		std::cout << "\xB1\xB1";	
		 
	    arraywithX [i] = cursorpos.X; // загнали в массивы 
        arraywithY [i] = cursorpos.Y; // остальные координаты
		  
        	
	}
	    if (intersection (amountofelements, arraywithX, arraywithY) == 0) // вызываем проверку совпадения
		{
			
			if (k != 0)
			{
			    Sleep (300);
                gameover ();
			    return 0;	
			}
			else if (k == 0)
			{
                system ("cls");
                Sleep (300);
			    return 0;
			}            
			
		}
        
		if (polygonrestriction () == 0)
		{
			return 0;
		}
			
		Sleep (sleepmil);
        clear (amountofelements, arraywithdirections,/* arraywithX [], arraywithY [],*/cursorpos, headpos/*, int k*/);
 	 
	// меняем местами елементы массива с направлениями    
	   for (int i = 0; i < amountofelements - 2; i++)
	   {
		// вот это стоит проверить  
	        arraywithdirections [i] = arraywithdirections [i + 1]; 
	 
	    	if (i == amountofelements - 3)
	            arraywithdirections [amountofelements - 2] = 0;
	   }
    
 	
}

void readscore ()
{

	file.open ("scoremain.txt");
	if (file)
	{

		fsold.open ("scoremain.txt", std::fstream::in | std::fstream::out ); // without append
		
		GotoLine (fsold, 1);
		fsold >> previousscore;
		
		GotoLine (fsold, 2);
		fsold >> bestscore;
	
		fsold.close();
	} 
	else if (!file)
	{
        // создаем файл и закрываем его, если он был несоздан 
		std::ofstream { "scoremain.txt" }; 
		fsold.open ("scoremain.txt", std::fstream::in | std::fstream::out | std::fstream::app); // without append
	    fsold.close();
	    previousscore = 0;
	    bestscore = 0;
	    fsold.close();
	}
	file.close (); 
}

void writescore()
{
	std::ofstream { "scorenext.txt" };
	fsnew.open ("scorenext.txt", std::fstream::in | std::fstream::out); // without append
    std::string object = "\n\n\n\n";
	fsnew << object;
	if (bestscore <= currentscore)
	{
		GotoLine (fsnew, 2);
		fsnew << currentscore; 
	}
	else 
	{
		GotoLine (fsnew, 2);
		fsnew << bestscore;
	}
		
   	GotoLine (fsnew, 1);
	fsnew << currentscore ; 
	fsnew.close(); 
	
	remove ("scoremain.txt"); 
	rename ("scorenext.txt", "scoremain.txt");	
	fsnew.close();
}
void showscore ()
{
	scorepos = {107,10};

    SetConsoleCursorPosition (hout, scorepos);
    
	if (bestscore <= currentscore)
	{
	    // УСТАНОВИМ КРАСНЫЙ ЦВЕТ 
   	    SetConsoleTextAttribute (hout, yellowfontblacktext | FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		//SetConsoleTextAttribute (hout, FOREGROUND_INTENSITY | FOREGROUND_RED);
	    
	    std::cout << currentscore;
	    scorepos.Y++;
	    SetConsoleCursorPosition (hout, scorepos);
	    
	    SetConsoleTextAttribute (hout, yellowfontblacktext);
      
	    std::cout << previousscore;
        scorepos.Y++;
    	SetConsoleCursorPosition (hout, scorepos);  
    	// снова красный цвет 
   	    SetConsoleTextAttribute (hout, yellowfontblacktext | FOREGROUND_INTENSITY | FOREGROUND_GREEN);

        std::cout << currentscore;
        
	    SetConsoleTextAttribute (hout, yellowfontblacktext);
    // вызовем функцию, которая запишет новый рекорд
	}
	
	else 
	{
	SetConsoleTextAttribute (hout, yellowfontblacktext);
	
	std::cout << bestscore;
	scorepos.Y++;
	SetConsoleCursorPosition (hout, scorepos);
    std::cout << previousscore;
    scorepos.Y++;
	SetConsoleCursorPosition (hout, scorepos);
    std::cout << currentscore;
    }
}
void drawfood ()
{
	SetConsoleCursorPosition (hout, foodpos);
	SetConsoleTextAttribute (hout, BACKGROUND_INTENSITY | yellowfontblacktext | BACKGROUND_BLUE/* | FOREGROUND_BLUE*/);
	std::cout << "\xB2\xB2\xB2";
	foodpos.Y ++;
	SetConsoleCursorPosition (hout, foodpos);
	std::cout << "\xB2\xB2\xB2";	
    foodpos.Y --;
}

bool foodeaten (int *amountofelements, int arraywithdirections [], unsigned int *currentscore, int *sleepmil)
{
    	if (k == 0) // на карте нету еды, печатаем
		{
			food (headpos);
			SetConsoleCursorPosition (hout, foodpos);
			SetConsoleTextAttribute (hout, BACKGROUND_INTENSITY | yellowfontblacktext | BACKGROUND_BLUE/* | FOREGROUND_BLUE*/);
		    std::cout << "\xB2\xB2\xB2";
			foodpos.Y ++;
			SetConsoleCursorPosition (hout, foodpos);
			std::cout << "\xB2\xB2\xB2";	
		    foodpos.Y --;

			// если clearfood() срабатывает, то вызовем функцию увелечения currentscore 
            k = 1;
            // если совпадают координаты
		    if (((headpos.X == foodpos.X) && (headpos.Y == foodpos.Y)) || ((headpos.X == foodpos.X + 1) && (headpos.Y == foodpos.Y)) || ((headpos.X == foodpos.X + 2) && (headpos.Y == foodpos.Y)) || ((headpos.X == foodpos.X + 3) && (headpos.Y == foodpos.Y)))
			{
				
				for (int i = *amountofelements - 2; i >= 0; i--)
			    {
				arraywithdirections [i+1] = arraywithdirections [i];
				if (i == 0)
				
					arraywithdirections [i] = arraywithdirections [i+1];
			    }
			clearfood (); 
			*(currentscore) += 2;
			if (*(sleepmil) > 50)
    		    *(sleepmil) -= 10;    
		    
			k = 0;
		    *(amountofelements) += 1;
		    return 1; // еду съели
			} 		   // }
		    else if (((headpos.X == foodpos.X) && (headpos.Y == foodpos.Y + 1)) || ((headpos.X == foodpos.X + 1) && (headpos.Y == foodpos.Y + 1)) || ((headpos.X == foodpos.X + 2) && (headpos.Y == foodpos.Y + 1)) || ((headpos.X == foodpos.X + 3) && (headpos.Y == foodpos.Y + 1)))
		    {
				for (int i = *amountofelements - 2; i >= 0; i--)
			    {
				arraywithdirections [i+1] = arraywithdirections [i];
				if (i == 0)
				
					arraywithdirections [i] = arraywithdirections [i+1];
			    }
		    clearfood ();
            
		    
				*(currentscore) += 2;
		    if (*(sleepmil) > 50)
		        *(sleepmil) -= 10;
		   
		    k = 0;
		    *(amountofelements) += 1;
		    return 1; // еду съели
		    }

		}
        
		else if (k == 1) // на карте есть еда, печатать не надо 
		{
		    if (((headpos.X == foodpos.X) && (headpos.Y == foodpos.Y)) || ((headpos.X == foodpos.X + 1) && (headpos.Y == foodpos.Y)) || ((headpos.X == foodpos.X + 2) && (headpos.Y == foodpos.Y)) || ((headpos.X == foodpos.X + 3) && (headpos.Y == foodpos.Y)))
			{
				//k = 0;
				for (int i = *amountofelements - 2; i >= 0; i--)
			    {
				arraywithdirections [i+1] = arraywithdirections [i];
				if (i == 0)
				
					arraywithdirections [i] = arraywithdirections [i+1];
			    }
			clearfood ();    
		    
		    *(currentscore) += 2;
			if (*(sleepmil) > 50)
			    *(sleepmil) -= 10;
			
			k = 0;
		    *(amountofelements) += 1;
		    return 1; // еду съели
		    }
		    else if (((headpos.X == foodpos.X) && (headpos.Y == foodpos.Y + 1)) || ((headpos.X == foodpos.X + 1) && (headpos.Y == foodpos.Y + 1)) || ((headpos.X == foodpos.X + 2) && (headpos.Y == foodpos.Y + 1)) || ((headpos.X == foodpos.X + 3) && (headpos.Y == foodpos.Y + 1)))
		    {
		    	for (int i = *amountofelements - 2; i >= 0; i--)
			    {
				arraywithdirections [i+1] = arraywithdirections [i];
				if (i == 0)
				
					arraywithdirections [i] = arraywithdirections [i+1];
			    }
			    
			clearfood ();    
		    *(currentscore) += 2;
			
			if (*(sleepmil) > 50)
			    *(sleepmil) -= 10;
			
			k = 0;
		    *(amountofelements) += 1;
		    return 1; // еду съели 
		    }
		}
        
		return 0; // еду не съели 
}

void clearfood ()
{
	SetConsoleCursorPosition (hout, foodpos);
	std::cout << "   ";
	foodpos.Y++;
	SetConsoleCursorPosition (hout, foodpos);
	std::cout << "   ";
	foodpos.Y--;
}

bool polygonrestriction ()
{
	if ((headpos.X <= 5) || (headpos.X >= 81) || (headpos.Y <= 3) || (headpos.Y >= 24))
	{            
		gameover ();	
        return 0;
	}
	else if ((headpos.X >= 42) && (headpos.X <= 44) && (headpos.Y >= 7) && (headpos.Y <= 20)) 
	{
		gameover ();	
        return 0;
	}	
	else 
	    return 1;	
}
void food (COORD headpos) 
{
		foodpos.X = rand () % 70 + 5; 
        foodpos.Y = rand () % 18 + 4;	
	    if ((foodpos.X >= 39) && (foodpos.X <= 43) && (foodpos.Y >= 7) && (foodpos.Y <= 20)) 
	    {
	        food (headpos);	   	
	    }
}

void polygon (WORD Attributes) // рисует стенки для змейки 
{
    SetConsoleTextAttribute (hout, FOREGROUND_RED | FOREGROUND_INTENSITY);
      
	COORD cursorpos;
    cursorpos.X = 4; cursorpos.Y = 3;


	for (cursorpos.X = 4; cursorpos.X <= 81; cursorpos.X ++)
	{
	    SetConsoleCursorPosition (hout,cursorpos);
		std::cout<<"\xB1";
    }
    cursorpos.X = 4;

    for (int i = 0; i <= 1; i ++)
	{
		if (i == 0)
		    cursorpos.X = 3; 
		else 
			cursorpos.X = 4; 
		
	for (cursorpos.Y = 3; cursorpos.Y <= 23; cursorpos.Y ++)
	    {
        	SetConsoleCursorPosition (hout,cursorpos);
	    	std::cout<<"\xB1"; 
	    }	
	}
 
	
	for (cursorpos.X = 3; cursorpos.X <= 82; cursorpos.X ++) // here gotta remember 
	{ 
		SetConsoleCursorPosition (hout,cursorpos);
		std::cout<<"\xB1";	
    }
    
    for (int i = 0; i <= 1; i ++)
	{
		if (i == 0)
		    cursorpos.X = 82; 
		else 
			cursorpos.X = 81; 
		
	    for (cursorpos.Y = 23; cursorpos.Y >= 3; cursorpos.Y --) // почему 23? 
	    { 
		    SetConsoleCursorPosition (hout,cursorpos);
		    std::cout<<"\xB1";	
    	}	
	}
    
    
	cursorpos.X = 42;
	for (cursorpos.Y = 7; cursorpos.Y <= 20; cursorpos.Y ++ )
	{
		SetConsoleCursorPosition (hout,cursorpos);
		std::cout << "\xB1\xB1";  
	} 
	// теперь нам нужны надписи под очки
	scoretablepos = {90,10};
	SetConsoleCursorPosition (hout, scoretablepos);
	SetConsoleTextAttribute (hout, yellowfontblacktext/*BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN*/);
	
	std::cout << "Best score:";
	scoretablepos.Y++;
	SetConsoleCursorPosition (hout, scoretablepos);
    std::cout << "Previous score:";
    scoretablepos.Y++;
	SetConsoleCursorPosition (hout, scoretablepos);
    std::cout << "Current score:";
    showscore (/*&previousscore, &bestscore, &currentscore*/);
}

void clear (int amountofelements, int arraywithdirections [], COORD cursorpos, COORD headpos)
{
    // ВЫСТАВИМ заливку пробелов
	SetConsoleTextAttribute (hout, Attributes); 
	SetConsoleTextAttribute (hout, BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN);
    headpos.X --;
	SetConsoleCursorPosition (hout, headpos); // выставляем курсор на коорд. головы 
    std::cout << "  ";
	cursorpos = headpos; 
	
	headpos.X ++;


	for (int i = amountofelements - 2; i >= 0; i--) //голова готова, поэтому <, начинаем с еденицы
	{
        
		
			if (arraywithdirections [i] == 1)
		    {
		 	cursorpos.Y --;
		 	SetConsoleCursorPosition (hout, cursorpos);
	        }
		 	
		    else if (arraywithdirections [i] == 2)
		    {
		 	cursorpos.X -= 2;
		 	SetConsoleCursorPosition (hout, cursorpos);
		    }
		    else if (arraywithdirections [i] == 3)
		    {
		 	cursorpos.Y ++;
		 	SetConsoleCursorPosition (hout, cursorpos);
		    }	
		    else if (arraywithdirections [i] == 4)
		    {
		 	cursorpos.X += 2;
		 	SetConsoleCursorPosition (hout, cursorpos);
	        }		
	std::cout << "  ";	
	}
}
bool intersection (int amountofelements, int arraywithX [], int arraywithY [])
{
	// проверяем на повторки так же с конца по идее
	for (int i = amountofelements - 2; i >= 0; i--)
	{
		for (int a = i - 1; a >= 0; a--)
		{
			if ((arraywithX [i] == arraywithX [a]) && (arraywithY [i] == arraywithY [a]))
			{
				
			    return 0;
			}   
		}
	}
	return 1;
}
 
 
void specialrestriction (double *keypressed, int *head_direction) 
{   // змейка не может ползти назад 
    if ((*(keypressed) != 77) && (*(head_direction) == 75))
        *head_direction = *keypressed; 
        
    else if ((*(keypressed) != 75) && (*(head_direction) == 77)) 
        *head_direction = *keypressed; 
    
    else  if  ((*(keypressed) != 72) && (*(head_direction) == 80))    
        *head_direction = *keypressed;
        
    else if  ((*(keypressed) != 80) && (*(head_direction) == 72))  
        *head_direction = *keypressed;
}
bool pause () 
{
	int a = 0;
	system ("cls");
	
	SetConsoleCursorPosition (hout, headpos);
	std::cout<<"PAUSED";
	while (a != 32)
	{
		if (kbhit())
		{
			int b = getch();
			if (b == 32) // конец паузы 
			    {
				a = b;
			    system ("cls");
				polygon (Attributes);
				drawfood ();
				return 1;
			    }
			
			else if (b == 27) // нажимаем escape 
			    {
				system ("cls");
				writescore ();
				return 0;    
	            }
		}

	}
	
}

bool contornot ()
{
    
	decisionpos = {26,18};
	SetConsoleCursorPosition (hout, decisionpos);
	std::cout << "do you want to play again?";
	decisionpos.Y += 3;
	SetConsoleCursorPosition (hout, decisionpos);
	std::cout << "1 - yep";
	decisionpos.X += 10;
	SetConsoleCursorPosition (hout, decisionpos);
	std::cout << "0 - nope";
	int decision = 0;
	while ((decision != 49) && (decision != 48) && (decision != 27))
	{
		if (kbhit())
		
			decision = getch();
	} 
	switch (decision)
	{
		case (49): // продолжаем играть 
			return 1;
			break;
		case (48): // закончили игру
			return 0;
			break;
		case (27):
		    return 0;
			break;	
	}
		
}
void gameover ()
{
	Sleep (1000);
	SetConsoleTextAttribute (hout, BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN);
	system ("cls");
	
	int arraygameover [10] [30] =
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	 1,0,1,1,0,1,1,1,0,1,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,1,1,
	 1,0,1,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,1,
	 1,0,1,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,0,0,0,1,1,1,0,1,1,1,1,1,
	 1,0,1,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,
	 1,0,0,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
	 cursorpos = {25,5};
	 SetConsoleCursorPosition (hout, cursorpos);
	 for (int a = 0; a < 10; a++)
	 {
	 	if (a != 0) 
	        std::cout<<std::endl;
	        cursorpos.Y++;
	        SetConsoleCursorPosition (hout, cursorpos);
		for (int b = 0; b < 28; b++) 
	    {
	    	if (arraygameover [a] [b] == 1)
			{
				SetConsoleTextAttribute (hout, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
				std::cout<<"\xB1";
			}
			else
			{   
			    SetConsoleTextAttribute (hout, BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
				std::cout<<" ";
		    }
		}
	 }
    //writescore (); // writescore () будет вызываться из другой функции   
	
	// теперь мы тут организуем, чтобы можно было сразу продолжить либо уж завершить игру 
	if (contornot ())
	{
		writescore ();
		currentscore = 0; // ужно обнулить, потому что глобальная пременная 
	    k = 0; // еда должна вновь появиться на поле 
		headcoord();
    }
	else 
	
	    writescore ();	
		
}

