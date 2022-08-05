#include <stdlib.h>
#include <stdio.h>
#include "framework.h"


#define GET(field, i, j) ((field)->data[((i)*((field)->width)) + (j)])
#define SET(field, i, j, x) ((field)->data[((i)*((field)->width)) + (j)] = (x))
#define GET_VIS(field, i, j) ((field)->visible[((i)*((field)->width)) + (j)])
#define SET_VIS(field, i, j, x) ((field)->visible[((i)*((field)->width)) + (j)] = (x))
#define INC(field, i, j) if(((field)->data[((i)*((field)->width)) + (j)]) > -1) {((field)->data[((i)*((field)->width)) + (j)] ++);}

typedef struct {
    long length;
    long width;
    int *data;
    int *visible; // -1 -> flagged; 0 -> not visible; 1 -> visible; 2 -> visible mine
    HWND *buttons;
} field_t;

int get_mines(field_t* board, int i, int j) {
    return GET(board, i, j);
}

void print_board(field_t *board, int hide){
    for(int i = 0; i < board->length; i ++){
        for(int j = 0; j < board->width; j++){
            if(GET(board, i, j) != -2){
                if (!hide || GET_VIS(board, i, j) == 1){
                    if(GET(board, i, j) != -1) { 
                        printf("%d ", GET(board, i, j)); 
                    }
                    else { printf("X "); }
                }
                else if (GET_VIS(board, i, j) == -1){ printf("X "); }
                else { printf("- "); }
            }
            else { printf("~ "); }
        }
        printf("\n");
    }
}


void initialize_board(field_t *board, int length, int width, HWND hWnd){
    board->length = length+2;
    board->width = width+2;
    board->data = (int*) malloc(sizeof(int) * board->length * board->width);
    board->visible = (int*) malloc(sizeof(int) * board->length * board->width);
    board->buttons = (HWND*)malloc(sizeof(HWND) * board->length * board->width);
    for(int i = 0; i < board->length; i ++){
        for(int j = 0; j < board->width; j++){
            if(i == 0 || i == board->length-1 || j == 0 || j == board->width-1){ SET(board, i , j, -2); }
            else{ SET(board, i, j, 0); }
        }
    }
    for(int i = 0; i < board->length; i ++){
        for(int j = 0; j < board->width; j++){
            SET_VIS(board, i, j, 0);
        }
    }
    for (int i = 0; i < board->length; i++)
    {
        for (int j = 0; j < board->width; j++)
        {
            board->buttons[((i) * (board)->width) + (long)(j)] = CreateWindow(
                L"BUTTON",  // Predefined class; Unicode assumed 
                L"",      // Button text 
                WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | BS_PUSHBUTTON,  // Styles 
                100 + 25 * j,         // x position 
                50 + 25 * i,         // y position 
                25,        // Button width
                25,        // Button height
                hWnd,     // Parent window
                (HMENU)(1000 + j + (i * board->width)),
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);      // Pointer not needed.
                
        }
    }
}

//For now just purely random
void set_mines(field_t *board, int n){
    for(int i = 0; i < n; i ++){
        int loc = rand() % (board->length * board->width);
        if(board->data[loc] > -1){ board->data[loc] = -1; }
        else { i --; }   
    }
}

void set_hints(field_t* board) {
    for (int i = 0; i < board->length; i++) {
        for (int j = 0; j < board->width; j++) {
            if (GET(board, i, j) == -1) {
                INC(board, i + 1, j + 1);
                INC(board, i + 1, j);
                INC(board, i + 1, j - 1);
                INC(board, i - 1, j - 1);
                INC(board, i - 1, j);
                INC(board, i - 1, j + 1);
                INC(board, i, j - 1);
                INC(board, i, j + 1);
            }
        }
    }
}

void update_text(field_t* board) {
    for (int i = 0; i < board->length; i++) {
        for (int j = 0; j < board->width; j++) {
            if(GET_VIS(board, i, j) == 1){
                switch (GET(board, i, j)) {
                case -2:
                    SetWindowText(board->buttons[((i) * ((board)->width)) + (j)], L"X");
                    break;
                case -1:
                    SetWindowText(board->buttons[((i) * ((board)->width)) + (j)], L"*");
                    break;
                case 0:
                    SetWindowText(board->buttons[((i) * ((board)->width)) + (j)], L"");
                    break;
                default: {
                    WCHAR* pwcsMineCt;
                    pwcsMineCt = (WCHAR*)malloc(sizeof(WCHAR));
                    _itow_s(GET(board, i, j), pwcsMineCt, sizeof(WCHAR), 10);
                    SetWindowText(board->buttons[((i) * ((board)->width)) + (j)], pwcsMineCt);
                }
                }
            }
        }
    }
}

WCHAR* inttowidec(int n) {
    WCHAR* pwcsMineCt;
    pwcsMineCt = (WCHAR*)malloc(sizeof(WCHAR));
    _itow_s(n, pwcsMineCt, sizeof(WCHAR), 10);
    return pwcsMineCt;
}

int explore(field_t *board, int x, int y){
    if(!GET_VIS(board, x, y)){
        if(GET(board, x, y) == 0){
            SET_VIS(board, x, y, 1);
            return 1 + 
                explore(board, x-1, y) + explore(board, x+1, y) + explore(board, x, y+1) + 
                explore(board, x, y-1) + explore(board, x-1, y-1) + explore(board, x+1, y+1)
                + explore(board, x-1, y+1) + explore(board, x+1, y-1) ;
        }
        else if (GET(board, x, y) > 0){
            SET_VIS(board, x, y, 1);
            return 1;
        }
        else {
            return 0;
        }      
    }
    else { return 0; }
}

void find_starting_area(field_t *board){
    int x = -1;
    int y = -1;
    int max = -1;
    for(int i = 0; i < board->length; i ++){
        for(int j = 0; j < board->width; j ++){
            if(GET(board, i, j) == 0 && !GET_VIS(board, i, j)){
                int cur = explore(board, i, j);
                if(cur > max){
                    x = i;
                    y = j;
                    max = cur;
                }
            }
        }
    }
    for(int i = 0; i < board->length; i ++){
        for(int j = 0; j < board->width; j++){
            SET_VIS(board, i, j, 0);
        }
    }

    explore(board, x, y);
    update_text(board);
}


int select_tile(field_t *board, int x, int y){
    if(x > 0 && x < board->width - 1 && y > 0 && y < board->length - 1){
        if(GET_VIS(board, y, x) == 0){         
            HWND button = board->buttons[(y * board->width) + x]; 

            if(GET(board, y, x) == -1) {
                SET_VIS(board, y, x, 2);
                return -1; 
            }
            else if(GET(board, y, x) == 0){
                explore(board, y, x);
                return 0;
            }
            else {
                SET_VIS(board, y, x, 1);
                return 0;
            }
        }
        return 0;
    }
    return -2;
}

//Returns:
//0: No changes made
//1: Tile flagged
//2: Tile unflagged
//-2: Given out of bounds tile location
int flag_tile(field_t *board, int x, int y){
    if(x > 0 && x < board->width - 1 && y > 0 && y < board->length -1){
        if(GET_VIS(board, y, x) == 0) { 
            SET_VIS(board, y, x, -1); 
            return 1;
        }
        else if(GET_VIS(board, y, x) == -1) { 
            SET_VIS(board, y, x, 0); 
            return 2;
        }
        return 0;
    }
    return -2;
}

//Returns 1 if board is solved and 0 if it is not
int check_win(field_t *board){
    int all_space_visible = 1;
    int all_mines_flagged = 1;
    for(int i = 0; i < board->length; i ++){
        for(int j = 0; j < board->width; j ++){
            if((all_space_visible) && (GET(board, i, j) > -1) && (GET_VIS(board, i, j) != 1)){
                all_space_visible = 0;
            }
            if(all_mines_flagged && (GET(board, i, j) == -1) && (GET_VIS(board, i, j) != -1)){
                all_mines_flagged = 0;
            }

            //found misflag
            if (GET(board, i, j) != -1 && GET_VIS(board, i, j) == -1) {
                return 0;
            }
        }
    }
    return (all_space_visible || all_mines_flagged);
}




