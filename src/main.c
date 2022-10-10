/**
  ******************************************************************************
  * ECE 362 Final Project - TicTacToe with scoring
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include "lcd.h"

char tictactoe[9];
char history[16];
char queue[2];
char offset;
int  qin;
int  qout;
int  turn = 0;
int  state = 0;
const char message_O[] = "Player O's turn!";
const char message_X[] = "Player X's turn!";
const char message[] = "Press a number from 1-9";
const char win_O[] = "Player O wins!";
const char win_X[] = "Player X wins!";
const char reset_message[] = "Push 0 to reset the board";
const char font[] = {
        [' '] = 0x00,
        ['0'] = 0x3f,
        ['1'] = 0x06,
        ['2'] = 0x5b,
        ['3'] = 0x4f,
        ['4'] = 0x66,
        ['5'] = 0x6d,
        ['6'] = 0x7d,
        ['7'] = 0x07,
        ['8'] = 0x7f,
        ['9'] = 0x67,
        ['A'] = 0x77,
        ['B'] = 0x7c,
        ['C'] = 0x39,
        ['D'] = 0x5e,
        ['*'] = 0x49,
        ['#'] = 0x76,
        ['.'] = 0x80,
        ['?'] = 0x53,
        ['b'] = 0x7c,
        ['r'] = 0x50,
        ['g'] = 0x6f,
        ['i'] = 0x10,
        ['n'] = 0x54,
        ['u'] = 0x1c,
};

void setup_spi1();
void setup_gpiob();
void setup_tim7();
void draw_grid();
int  get_cols();
void insert_queue(int);
void update_hist(int);
void set_row();
void display_char();
int  getkey();
int  check_winner();
void reset();

void setup_spi1() { // from lab 8
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5
                    | GPIO_MODER_MODER7);
    GPIOA->MODER |= GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1
                    | GPIO_MODER_MODER7_1;
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5 | GPIO_AFRL_AFRL7);
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
    GPIOA->MODER |= GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_2 | SPI_CR2_DS_1
              | SPI_CR2_DS_0;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void setup_gpiob() {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~(GPIO_MODER_MODER7 | GPIO_MODER_MODER6 | GPIO_MODER_MODER5
                    | GPIO_MODER_MODER4 | GPIO_MODER_MODER3 | GPIO_MODER_MODER2
                    | GPIO_MODER_MODER1 | GPIO_MODER_MODER0);
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR7 | GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR5
                    | GPIO_PUPDR_PUPDR4);
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR7_1 | GPIO_PUPDR_PUPDR6_1
                  | GPIO_PUPDR_PUPDR5_1 | GPIO_PUPDR_PUPDR4_1;
    GPIOB->MODER |= GPIO_MODER_MODER3_0 | GPIO_MODER_MODER2_0
                  | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER0_0;
}

void setup_tim7()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    RCC->APB1ENR &= ~(RCC_APB1ENR_TIM6EN);
    TIM7->PSC = 4800-1;
    TIM7->ARR = 10-1;
    TIM7->DIER |= TIM_DIER_UIE;
    TIM7->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] |= 1<<(TIM7_IRQn);
}

void draw_grid() {
    int i;
    for(i = 0; i < 11; i++) // left vertical bar
        LCD_DrawChar(80, 80+(i*16), BLACK, WHITE, 124, 16, 0);
    for(i = 0; i < 11; i++) // right vertical bar
        LCD_DrawChar(80+(4*16), 80+(i*16), BLACK, WHITE, 124, 16, 0);
    for(i = 0; i < 11; i++) // top horizontal bar
        LCD_DrawChar(80-(3*16)+(i*16), 80+(3*16), BLACK, WHITE, 45, 16, 0);
    for(i = 0; i < 11; i++) // bottom horizontal bar
        LCD_DrawChar(80-(3*16)+(i*16), 80+(7*16), BLACK, WHITE, 45, 16, 0);
}

int get_cols()
{
    return (GPIOB->IDR >> 4 & 0xf);
}

void insert_queue(int n)
{
    queue[qin] = n | 0x80;
    qin ^= 1;
}

void update_hist(int cols)
{
    int row = offset & 3;
    for(int i=0; i < 4; i++) {
        history[4*row+i] = (history[4*row+i]<<1) + ((cols>>i)&1);
        if(history[4*row+i] == 0x1)
            insert_queue(4*row+i);
    }
}

void set_row()
{
    GPIOB->BSRR |= 0xf0000 | (1 << (offset & 0x3));
}

void TIM7_IRQHandler()
{
    display_char();
    update_hist(get_cols());
    offset = (offset + 1) & 0x7;
    set_row();
    TIM7->SR &= ~TIM_SR_UIF;
}

void display_char() {
    //int key = getkey();
    //char ch = font[key];
    //LCD_DrawChar(0, 0, BLACK, WHITE, ch, 16, 0);
}

int getkey()
{
    while(1) {
        asm volatile("wfi");
        if(queue[qout] == 0)
            continue;
        else
            break;
    }
    char copy = queue[qout];
    queue[qout] = 0;
    qout ^= 1;
    copy &= 0x7f;
    if(copy == 0)
        return '1';
    if(copy == 1)
        return '2';
    if(copy == 2)
        return '3';
    if(copy == 3)
        return 'A';
    if(copy == 4)
        return '4';
    if(copy == 5)
        return '5';
    if(copy == 6)
        return '6';
    if(copy == 7)
        return 'B';
    if(copy == 8)
        return '7';
    if(copy == 9)
        return '8';
    if(copy == 10)
        return '9';
    if(copy == 11)
        return 'C';
    if(copy == 12)
        return '*';
    if(copy == 13)
        return '0';
    if(copy == 14)
        return '#';
    if(copy == 15)
        return 'D';
}

void play(char key) {
    char text;
    if(turn == 0) {
        text = 'X';
        LCD_DrawString(0, 0, BLACK, WHITE, message_O, 16, 0);
    }
    else {
        text = 'O';
        LCD_DrawString(0, 0, BLACK, WHITE, message_X, 16, 0);
    }
    LCD_DrawString(0, 16, BLACK, WHITE, message, 16, 0);
    LCD_DrawString(0, 32, BLACK, WHITE, reset_message, 16, 0);
    while(1) {
        if(key == '7') {
            LCD_DrawChar(48, 224, BLACK, WHITE, text, 16, 0);
            tictactoe[0] = text;
            break;
        }
        else if(key == '8') {
            LCD_DrawChar(112, 224, BLACK, WHITE, text, 16, 0);
            tictactoe[1] = text;
            break;
        }
        else if(key == '9') {
            LCD_DrawChar(176, 224, BLACK, WHITE, text, 16, 0);
            tictactoe[2] = text;
            break;
        }
        else if(key == '4') {
            LCD_DrawChar(48, 160, BLACK, WHITE, text, 16, 0);
            tictactoe[3] = text;
            break;
        }
        else if(key == '5') {
            LCD_DrawChar(112, 160, BLACK, WHITE, text, 16, 0);
            tictactoe[4] = text;
            break;
        }
        else if(key == '6') {
            LCD_DrawChar(176, 160, BLACK, WHITE, text, 16, 0);
            tictactoe[5] = text;
            break;
        }
        else if(key == '1') {
            LCD_DrawChar(48, 96, BLACK, WHITE, text, 16, 0);
            tictactoe[6] = text;
            break;
        }
        else if(key == '2') {
            LCD_DrawChar(112, 96, BLACK, WHITE, text, 16, 0);
            tictactoe[7] = text;
            break;
        }
        else if(key == '3') {
            LCD_DrawChar(176, 96, BLACK, WHITE, text, 16, 0);
            tictactoe[8] = text;
            break;
        }
        else if(key == '0') {
            reset();
            break;
        }
    }
    //for(int i = 0; i < 9; i++)
    //    LCD_DrawChar(i*16, 300, BLACK, WHITE, tictactoe[i], 16, 0);
}

int check_winner() {
    int win = 0;
    if((tictactoe[0] == tictactoe[1] == tictactoe[2])  // bottom row
     || (tictactoe[3] == tictactoe[4] == tictactoe[5]) // middle row
     || (tictactoe[6] == tictactoe[7] == tictactoe[8]) // top row
     || (tictactoe[0] == tictactoe[3] == tictactoe[6]) // left col
     || (tictactoe[1] == tictactoe[4] == tictactoe[7]) // middle col
     || (tictactoe[2] == tictactoe[5] == tictactoe[8]) // right col
     || (tictactoe[0] == tictactoe[4] == tictactoe[8]) // top right bot left
     || (tictactoe[2] == tictactoe[4] == tictactoe[6])) win = 1; // top left bot right
    if((win == 1) && (turn == 0)) LCD_DrawString(0, 48, BLACK, WHITE, win_O, 16, 0);
    else if((win == 1) && (turn == 1)) LCD_DrawString(0, 48, BLACK, WHITE, win_X, 16, 0);
    turn ^= 1;
    //LCD_DrawChar(10*16, 300, BLACK, WHITE, win+48, 16, 0);
    return win;
}

void reset() {
    for(int i = 0; i < 9; i++) {
        tictactoe[i] = i+48;
    }
    LCD_DrawChar(48, 224, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(112, 224, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(176, 224, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(48, 160, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(112, 160, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(176, 160, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(48, 96, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(112, 96, BLACK, WHITE, ' ', 16, 0);
    LCD_DrawChar(176, 96, BLACK, WHITE, ' ', 16, 0);
}

int main(void)
{
    setup_spi1();
    setup_gpiob();
    LCD_Init();
    LCD_Clear(WHITE);
    draw_grid();
    setup_tim7();
    for(;;) {
        reset();
        while(1) {
            char key = getkey();
            play(key);
            if(check_winner()) {
                //reset();
                break;
            }
        }
    }
}
