#include "main.h"

#define     NO_OF_FLOORS    8
#define     NO_OF_BUTTONS   16
//#define     LED     PORTD
#define     ON      1
#define     OFF     0
#define     button_up   8
#define     button_down 9
#define     button_open 12
#define     button_close 13
#define     UP      0
#define     DOWN    1
#define     NOTHING 2
#define     OPEN_DOOR    3
#define     OUTSIDE_MODE 99
#define     INSIDE_MODE 100
#define     ELEVATOR_MODE 101
#define     OUTSIDE_ASK_UP_DOWN_MODE 102
#define     OPENING_DOOR 103
#define     PRE_UP 104
#define     CLOSING_DOOR 105
#define     PRE_DOWN 106
#define     LED_NORMAL_MODE 107
#define     LED_BLINKING_MODE 108
#define     PRE_NOTHING 109
#define     INIT_SYSTEM 110
#define     ENTER_PASSWORD 111
#define     CHECK_PASSWORD 112
#define     UNLOCK_DOOR 113
#define     WRONG_PASSWORD 114
#define     USING 115
#define     UNLOCK_DOOR_ADMIN 116
#define     USING_ADMIN 117
#define     SHOW_NEW_PASSWORD 118
#define     MAINTAINANCE_MODE 119
#define     MAINTAINANCE_MODE_USER 120


#define     CHANGE_VIEW_OUTSIDE_INSIDE_BUTTON 14
#define     CHANGE_VIEW_TO_ELEVATOR_BUTTON 12
#define     CHANGE_VIEW_FLOOR_UP 11
#define     CHANGE_VIEW_FLOOR_DOWN 15
#define     button_hold  3
#define     button_quick 7
#define     TIME_MOVING_BETWEEN_FLOOR 2000
#define     TIME_DOOR 2500
#define     TIME_FLOOR_WAITING 10000
#define     TIME_LED_BLINKING 500
#define     TIME_ENTER_PASSWORD_DELAY 500
#define     TIME_USER_USING_DELAY 1000
#define     TIME_ENTER_PASSWORD_WHEN_NOTHING 2000


int floor[NO_OF_FLOORS];
int inside_floor[NO_OF_FLOORS] = {0, 0, 0, 0, 0, 0, 0, 0};
int inside_is_clicked = 0;
int outside_up_floor[NO_OF_FLOORS] = {0, 0, 0, 0, 0, 0, 0, 0};
int outside_up_is_clicked = 0;
int outside_down_floor[NO_OF_FLOORS] = {0, 0, 0, 0, 0, 0, 0, 0};
int outside_down_is_clicked = 0;
int state;
int current_floor = 0;
int outside_asking_floor = 0;
int clear = 0;
void init_system(void);
void delay_ms(int value);
int start = 0;
// mode OUTSIDE_MODE for user outside, mode INSIDE_MODE for user inside
int mode = OUTSIDE_MODE;
int mode_led_for_floors = LED_NORMAL_MODE;
int animation;
//for password
unsigned char arrayMapOfNumber [16] = {'1', '2', '3', 'A', '4', '5', '6', 'B',
    '7', '8', '9', 'C', '*', '0', '#', 'D'};
unsigned char arrayMapOfPassword [4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};
unsigned char arrayAdminPassword[4] = {'A', 'B', 'C', 'D'};
unsigned char statusPassword = INIT_SYSTEM;
unsigned char arrayPassword[4];
int indexOfNumber = 0;
int timeDelay = 0;
unsigned char numberValue;
unsigned char randomNumberForNewPassword = 0;
//for maintainance
int maintainance_mode = 0;
//Bai tap chong rung nut nhan
char numberOfPushButton = 0;
unsigned char isButtonPressed(int index);
unsigned char x = 0, y = 0;
void check_state();
void Test();
void fsm_automatic();
void fsm_outside_inside();
void initial();
void add_floor();
void display_view_inside();
void checkBtn_outside();
void floor_mod();
void display_led_for_floors();
//for password
void App_PasswordDoor();
unsigned char CheckPassword();
int isAnyButtonClicked();
int isButtonEnter();

void main(void) {
    init_system();
    delay_ms(500);
    initial();
    delay_ms(500);
    while (1) {
        scan_key_matrix();
                if (maintainance_mode == 0) {
                    fsm_automatic();
                    if (statusPassword == USING)
                        fsm_outside_inside();
                }
                App_PasswordDoor();
        DisplayLcdScreen();
    }
}

void delay_ms(int value) {
    int i, j;
    for (i = 0; i < value; i++)
        for (j = 0; j < 160; j++);
}

void init_system(void) {
    TRISB = 0x00; //setup PORTB is output
    PORTB = 0x00; //setup PORTB is output
    TRISD = 0x00;
    init_lcd();
    LcdClearS();
    //    LED = 0x00;
    init_interrupt();
    delay_ms(1000);
    init_timer0(4695); //dinh thoi 1ms
    init_timer1(9390); //dinh thoi 2ms
    init_timer3(46950); //dinh thoi 10ms
    SetTimer0_ms(10);
    SetTimer1_ms(10);
    SetTimer3_ms(50); //Chu ky thuc hien viec xu ly input,proccess,output
    setTimer_for_dung_yen(50);
    setTimer_for_open_door(50);
    init_key_matrix();
}

void initial() {
    current_floor = 0;
    state = NOTHING;
    LcdPrintStringS(0, 0, "STARTING!!!");
    DisplayLcdScreen();
    start = 1;
}

void floor_mod() {
    if (current_floor >= NO_OF_FLOORS) {
        current_floor = NO_OF_FLOORS;
    } else if (current_floor <= 0) {
        current_floor = 0;
    }
}

void clear_inside() {
    int i;
    for (i = 0; i < NO_OF_FLOORS; i++) {
        inside_floor[i] = 0;
    }
}

unsigned char isButtonPressed(int index) {
    if (key_code[index] == 1 || (key_code[index] >= 20 && key_code[index] % 2 == 1))
        return 1;
    else
        return 0;
}

int check_all_in_use() {
    int i;
    for (i = 0; i < NO_OF_FLOORS; i++) {
        if (inside_floor[i] == 1 || outside_down_floor[i] == 1 || outside_up_floor[i] == 1) {
            return 1;
        }
    }
    return 0;
}

int check_max_all_floor() {
    int i;
    for (i = NO_OF_FLOORS - 1; i > current_floor; i--) {
        if (outside_up_floor[i] > 0 || outside_down_floor[i] > 0 || inside_floor[i] > 0) {
            return i;
            break;
        }
    }
    return current_floor;
}

int check_outside_down_floor_remain() {
    int i;
    for (i = current_floor - 1; i >= 0; i--) {
        if (outside_down_floor[i] > 0) {
            return 1;
            break;
        }
    }
    return 0;
}

int check_inside_down_floor_remain() {
    int i;
    for (i = current_floor - 1; i >= 0; i--) {
        if (inside_floor[i] > 0) {
            return 1;
            break;
        }
    }
    return 0;
}

int check_current_floor_is_clicked() {
    int i;
    for (i = 0; i < 3; i++) {
        if (isButtonPressed(i) && current_floor == i + 1) {
            return 1;
            break;
        }
    }
    for (i = 4; i < 7; i++) {
        if (isButtonPressed(i) && current_floor == i) {
            return 1;
            break;
        }
    }
    for (i = 8; i < 9; i++) {
        if (isButtonPressed(i) && current_floor == i - 1) {
            return 1;
            break;
        }
    }
    if (isButtonPressed(13) && current_floor == 0) {
        return 1;
    }
    return 0;
}

void check_state() {
    if (state == NOTHING) {
        if ((current_floor != 0) && (flag_timer_dung_yen == 1)) {
            state = DOWN;
            SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR);
        } else if (check_max_all_floor() > current_floor) {
            state = UP;
            SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR);
        } else if (check_outside_down_floor_remain() == 1) {
            state = DOWN;
            SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR);
        } else if (check_inside_down_floor_remain() == 1) {
            state = DOWN;
            SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR);
        } else if ((mode == OUTSIDE_MODE|| mode==INSIDE_MODE) && check_current_floor_is_clicked() == 1 && check_max_all_floor() == current_floor) {
            state = PRE_NOTHING;
            animation = 1;
            start = 1;
            setTimer_for_open_door(TIME_DOOR);
        } else if (isButtonPressed(button_hold)) {
            state = PRE_NOTHING;
            animation = 1;
            start = 1;
        }   
    } else if (state == UP) {
        if (outside_down_floor[current_floor]==1 && current_floor == check_max_all_floor())
        {
            setTimer_for_open_door(TIME_DOOR);
            state = PRE_NOTHING;
            animation=1;
            start=1;
            clear_inside();
        }
        else if (outside_up_floor[current_floor] == 1 || inside_floor[current_floor] == 1) {
            setTimer_for_open_door(TIME_DOOR);
            state = PRE_UP;
            //            mode = OPENING_DOOR;
            animation = 1;
            start = 1;
        } else if (current_floor == check_max_all_floor()) {
            setTimer_for_open_door(TIME_DOOR);
            state = NOTHING;
            clear_inside();
        }

    } else if (state == DOWN) {
        if (outside_down_floor[current_floor] == 1 || inside_floor[current_floor] == 1) {
            setTimer_for_open_door(TIME_DOOR);
            state = PRE_DOWN;
            //            mode = OPENING_DOOR;
            animation = 1;
            start = 1;
        } 
        else if (current_floor == 0) {
            state = NOTHING;
            clear_inside();
        }
    }
}

void Display_moving() {
    LcdClearS();
    LcdPrintStringS(0, 0, "STATE:       ");
    if (state == UP)
        LcdPrintStringS(0, 8, "UP");
    else if (state == DOWN)
        LcdPrintStringS(0, 8, "DOWN");
    else if (state == PRE_UP)
        LcdPrintStringS(0, 8, "UP");
    else if (state == PRE_DOWN)
        LcdPrintStringS(0, 8, "DOWN");
    else
        LcdPrintStringS(0, 8, "NOTHING");

    LcdPrintStringS(1, 0, "CUR_Floor:     ");
    LcdPrintNumS(1, 10, current_floor);
}

void Display_Open() {
    LcdClearS();
    LcdPrintStringS(0, 0, "OPENING DOOR!!!     ");
}

void Display_Close() {
    LcdClearS();
    LcdPrintStringS(0, 0, "CLOSING DOOR!!!     ");
}

void fsm_automatic() {
    check_state();
    display_led_for_floors();
    if (check_all_in_use() == 1) {
        setTimer_for_dung_yen(TIME_FLOOR_WAITING);
    }

    switch (state) {
        case UP:
            if (flag_timer3 == 1) {
                current_floor++;
                SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR);
            }
            floor_mod();
            break;
        case DOWN:
            if (flag_timer3 == 1) {
                current_floor--;
                SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR);
            }
            floor_mod();
            break;
        case PRE_UP:
            if (isButtonPressed(button_hold)) {
                setTimer_for_open_door(TIME_DOOR);
        setTimer_for_dung_yen(TIME_FLOOR_WAITING);
                animation = 1;
                start = 1;
            }
            if (isButtonPressed(button_quick) && start == 1) {
                setTimer_for_open_door(200);
            }
            if (flag_timer_open_door == 1 && start == 1) {
                //                mode = CLOSING_DOOR;
                animation = 2;
                setTimer_for_open_door(TIME_DOOR);
                start = 0;
            } else if (flag_timer_open_door == 1 && start == 0) {
                if (outside_up_floor[current_floor] == 1)
                    outside_up_floor[current_floor] = 0;
                if (inside_floor[current_floor] == 1)
                    inside_floor[current_floor] = 0;
                //                mode = OUTSIDE_MODE;
                animation = 0;
                SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR + 3000);
                state = UP;
            }
            break;
        case PRE_DOWN:
            if (isButtonPressed(button_hold)) {
                setTimer_for_open_door(TIME_DOOR);
        setTimer_for_dung_yen(TIME_FLOOR_WAITING);
                animation = 1;
                start = 1;
            }
            if (isButtonPressed(button_quick)) {
                setTimer_for_open_door(200);
            }
            if (flag_timer_open_door == 1 && start == 1) {
                //                mode = CLOSING_DOOR;
                animation = 2;
                start = 0;
                setTimer_for_open_door(TIME_DOOR);
            } else if (flag_timer_open_door == 1 && start == 0) {
                if (outside_down_floor[current_floor] == 1)
                    outside_down_floor[current_floor] = 0;
                if (inside_floor[current_floor] == 1)
                    inside_floor[current_floor] = 0;
                //                mode = OUTSIDE_MODE;
                animation = 0;
                state = DOWN;
                SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR + 3000);
            }
            break;
        case PRE_NOTHING:
            if (isButtonPressed(button_hold)) {
                setTimer_for_open_door(TIME_DOOR);
        setTimer_for_dung_yen(TIME_FLOOR_WAITING);
                animation = 1;
                start = 1;
            }
            if (isButtonPressed(button_quick) && start == 1) {
                setTimer_for_open_door(200);
            }
            if (flag_timer_open_door == 1 && start == 1) {
                //                mode = CLOSING_DOOR;
                animation = 2;
                setTimer_for_open_door(TIME_DOOR);
                start = 0;
            } else if (flag_timer_open_door == 1 && start == 0) {
                inside_floor[current_floor] = 0;
                outside_down_floor[current_floor] = 0;
                outside_up_floor[current_floor] = 0;
                //                mode = OUTSIDE_MODE;
                animation = 0;
                SetTimer3_ms(TIME_MOVING_BETWEEN_FLOOR + 3000);
                state = NOTHING;
            }
            break;
        case NOTHING:
            break;
        default:
            break;
    }
}

void checkBtn_changeTo_elevator_mode() {
    if (isButtonPressed(CHANGE_VIEW_TO_ELEVATOR_BUTTON)) {
        if (mode != ELEVATOR_MODE)
            mode = ELEVATOR_MODE;
        else
            mode = OUTSIDE_MODE;
    }
}

void checkBtn_changeTo_outside_inside_mode() {
    if (isButtonPressed(CHANGE_VIEW_OUTSIDE_INSIDE_BUTTON)) {
        if (mode == OUTSIDE_MODE)
            mode = INSIDE_MODE;
        else
            mode = OUTSIDE_MODE;
    }
}

void checkBtn_inside() {
    int i;
    for (i = 0; i < 3; i++) {
        if (isButtonPressed(i)) {
            inside_floor[i + 1] = 1;
            inside_is_clicked = 1;
        }
    }
    for (i = 4; i < 7; i++) {
        if (isButtonPressed(i)) {
            inside_floor[i] = 1;
            inside_is_clicked = 1;
        }
    }
    for (i = 8; i < 9; i++) {
        if (isButtonPressed(i)) {
            inside_floor[i - 1] = 1;
            inside_is_clicked = 1;
        }
    }
    if (isButtonPressed(13)) {
        inside_floor[0] = 1;
        inside_is_clicked = 1;
    }
}

void checkBtn_outside() {
    int i;
    for (i = 0; i < 3; i++) {
        if (isButtonPressed(i)) {
            mode = OUTSIDE_ASK_UP_DOWN_MODE;
            outside_asking_floor = i + 1;
            break;
        }
    }
    for (i = 4; i < 7; i++) {
        if (isButtonPressed(i)) {
            mode = OUTSIDE_ASK_UP_DOWN_MODE;
            outside_asking_floor = i;
            break;
        }
    }
    for (i = 8; i < 9; i++) {
        if (isButtonPressed(i)) {
            mode = OUTSIDE_ASK_UP_DOWN_MODE;
            outside_asking_floor = i - 1;
            break;
        }
    }
    if (isButtonPressed(13)) {
        mode = OUTSIDE_ASK_UP_DOWN_MODE;
        outside_asking_floor = 0;
    }
}

void checkBtn_outside_asking() {
    if (isButtonPressed(0)) {
        outside_up_floor[outside_asking_floor] = 1;
        outside_up_is_clicked = 1;
        mode = OUTSIDE_MODE;
    } else if (isButtonPressed(1)) {
        outside_down_floor[outside_asking_floor] = 1;
        outside_down_is_clicked = 1;
        mode = OUTSIDE_MODE;
    }
}

void display_view_outside() {
    int k;
    LcdClearS();
    for (k = 0; k < NO_OF_FLOORS; k++)
        LcdPrintNumS(0, k, outside_up_floor[k]);
    LcdPrintStringS(0, 9, "CF: ");
    LcdPrintNumS(0, 12, current_floor);
    for (k = 0; k < NO_OF_FLOORS; k++)
        LcdPrintNumS(1, k, outside_down_floor[k]);
    LcdPrintStringS(1, 9, "CS:");
    if (state == UP)
        LcdPrintStringS(1, 12, "UP");
    else if (state == DOWN)
        LcdPrintStringS(1, 12, "DOWN");
    else if (state == PRE_UP)
        LcdPrintStringS(1, 12, "UP");
    else if (state == PRE_DOWN)
        LcdPrintStringS(1, 12, "DOWN");
    else
        LcdPrintStringS(1, 12, "NOTH");
}

void display_view_inside() {
    int k;
    LcdClearS();
    LcdPrintStringS(0, 0, "INSIDE,  CF:");
    LcdPrintNumS(0, 12, current_floor);
    for (k = 0; k < NO_OF_FLOORS; k++)
        LcdPrintNumS(1, k, inside_floor[k]);
    LcdPrintStringS(1, 9, "CS:");
    if (state == UP)
        LcdPrintStringS(1, 12, "UP");
    else if (state == DOWN)
        LcdPrintStringS(1, 12, "DOWN");
    else if (state == PRE_UP)
        LcdPrintStringS(1, 12, "UP");
    else if (state == PRE_DOWN)
        LcdPrintStringS(1, 12, "DOWN");
    else
        LcdPrintStringS(1, 12, "NOTH");
}

void display_view_outside_asking() {
    LcdClearS();
    LcdPrintStringS(0, 0, "FLOOR: ");
    LcdPrintNumS(0, 8, outside_asking_floor);
    LcdPrintStringS(1, 0, "1: UP, 2: DOWN");
}

void display_led_for_floors() {
    if (mode_led_for_floors == LED_NORMAL_MODE)
        switch (current_floor) {
            case 0:
                PORTB = 0b00000001;
                break;
            case 1:
                PORTB = 0b00000010;
                break;
            case 2:
                PORTB = 0b00000100;
                break;
            case 3:
                PORTB = 0b00001000;
                break;
            case 4:
                PORTB = 0b00010000;
                break;
            case 5:
                PORTB = 0b00100000;
                break;
            case 6:
                PORTB = 0b01000000;
                break;
            case 7:
                PORTB = 0b10000000;
                break;
            default:
                PORTB = 0b00000000;
                break;
        } else
        if (flag_timer1 == 1) {
        switch (current_floor) {
            case 0:
                PORTB = 0b00000001;
                break;
            case 1:
                PORTB = 0b00000010;
                break;
            case 2:
                PORTB = 0b00000100;
                break;
            case 3:
                PORTB = 0b00001000;
                break;
            case 4:
                PORTB = 0b00010000;
                break;
            case 5:
                PORTB = 0b00100000;
                break;
            case 6:
                PORTB = 0b01000000;
                break;
            case 7:
                PORTB = 0b10000000;
                break;
            default:
                PORTB = 0b00000000;
                break;
        }
        SetTimer1_ms(TIME_LED_BLINKING);
    } else
        PORTB = 0x00;
}

void fsm_outside_inside() {
    checkBtn_changeTo_elevator_mode();
    checkBtn_changeTo_outside_inside_mode();
    if (animation == 0) {
        mode_led_for_floors = LED_NORMAL_MODE;
    } else if (animation == 1) {
        mode_led_for_floors = LED_BLINKING_MODE;
    } else if (animation == 2) {
        mode_led_for_floors = LED_BLINKING_MODE;
    }
    switch (mode) {
        case OUTSIDE_MODE:
            checkBtn_outside();
            display_view_outside();
            break;
        case OUTSIDE_ASK_UP_DOWN_MODE:
            checkBtn_outside_asking();
            display_view_outside_asking();
            break;
        case INSIDE_MODE:
            checkBtn_inside();
            display_view_inside();
            break;
        case ELEVATOR_MODE:
            if (animation == 0) {
                Display_moving();
            } else if (animation == 1) {
                Display_Open();
            } else if (animation == 2) {
                Display_Close();
            }
            break;
        default:
            break;
    }
}

int isAnyButtonClicked() {
    int i;
    for (i = 0; i < 16; i++) {
        if (isButtonPressed(i)) {
            numberValue = arrayMapOfNumber[i];
            return 1;
        }
    }
    return 0;
}

int isButtonEnter() {
    if (isButtonPressed(12))
        return 1;
    else return 0;
}

unsigned char CheckPassword() {
    unsigned char i, j;
    unsigned result = 1;
    for (i = 0; i < 5; i++) {
        result = 1;
        for (j = 0; j < 4; j++) {
            if (arrayPassword[j] != arrayMapOfPassword[i][j])
                result = 0;
        }
        if (result == 1)
            return (i + 1);
    }

}

unsigned char CheckAdmin() {
    unsigned char j;
    unsigned result = 1;
    for (j = 0; j < 4; j++) {
        if (arrayPassword[j] != arrayAdminPassword[j])
            result = 0;
    }
    return result;
}

void App_PasswordDoor() {
    switch (statusPassword) {
        case INIT_SYSTEM:
            LcdClearS();
            LcdPrintStringS(0, 0, "PRESS * FOR PASS");
            if (isButtonEnter()) {
                indexOfNumber = 0;
                timeDelay = 0;
                statusPassword = ENTER_PASSWORD;
                LcdClearS();
            }
            break;
        case ENTER_PASSWORD:
            LcdPrintStringS(0, 0, "ENTER PASSWORD  ");
            timeDelay++;
            if (isAnyButtonClicked()) {
                LcdPrintStringS(1, indexOfNumber, "*");
                arrayPassword [indexOfNumber] = numberValue;
                indexOfNumber++;
                timeDelay = 0;
            }
            if (indexOfNumber >= 4)
                statusPassword = CHECK_PASSWORD;
            if (timeDelay >= TIME_ENTER_PASSWORD_DELAY)
                statusPassword = INIT_SYSTEM;
            break;
        case CHECK_PASSWORD:
            timeDelay = 0;
            if (CheckAdmin() == 1)
                statusPassword = UNLOCK_DOOR_ADMIN;
            else if (CheckPassword())
                statusPassword = UNLOCK_DOOR;
            else
                statusPassword = WRONG_PASSWORD;
            LcdClearS();
            break;
        case UNLOCK_DOOR:
            timeDelay++;
            LcdPrintStringS(0, 0, "YOU CAN ENTER   ");
            LcdPrintStringS(1, 0, "PRESS * TO USE  ");
            if (timeDelay >= TIME_USER_USING_DELAY)
                statusPassword = INIT_SYSTEM;
            if (isButtonEnter()) {
                timeDelay = 0;
                statusPassword = USING;
            }
            break;
        case UNLOCK_DOOR_ADMIN:
            timeDelay++;
            LcdPrintStringS(0, 0, "YOU ARE ADMIN   ");
            LcdPrintStringS(1, 0, "PRESS * TO USE  ");
            if (timeDelay >= TIME_USER_USING_DELAY)
                statusPassword = INIT_SYSTEM;
            if (isButtonEnter()) {
                timeDelay = 0;
                statusPassword = USING_ADMIN;
            }
            break;
        case USING:
            if (state == NOTHING) {
                timeDelay++;
            } else {
                timeDelay = 0;
            }
            if (maintainance_mode == 1) {
                statusPassword = MAINTAINANCE_MODE_USER;
                timeDelay = 0;
            }
            if (timeDelay >= TIME_ENTER_PASSWORD_WHEN_NOTHING / 2)
                statusPassword = INIT_SYSTEM;
            break;
        case USING_ADMIN:
            timeDelay++;
            LcdPrintStringS(0, 0, "OPTS: 1.ERROR 2.");
            LcdPrintStringS(1, 0, "REGISTER 3.ELEVA");
            if (isAnyButtonClicked()) {
                timeDelay = 0;
            }
            if (isButtonPressed(0)) {
                statusPassword = MAINTAINANCE_MODE;
                timeDelay = 0;
                LcdClearS();
            } else if (isButtonPressed(1)) {
                statusPassword = SHOW_NEW_PASSWORD;
                LcdClearS();
                timeDelay = 0;
                indexOfNumber = 0;
                randomNumberForNewPassword++;
                if (randomNumberForNewPassword > 3)
                    randomNumberForNewPassword = 0;
            } else if (isButtonPressed(2)) {
                timeDelay = 0;
                indexOfNumber = 0;
                statusPassword = USING;
            }
            if (timeDelay >= TIME_USER_USING_DELAY)
                statusPassword = INIT_SYSTEM;
            break;
        case SHOW_NEW_PASSWORD:
            timeDelay++;
            LcdPrintStringS(0, 0, "YOUR PASSWORD IS");
            if (indexOfNumber == 0) {
                LcdPrintCharS(1, 0, arrayMapOfPassword[randomNumberForNewPassword][0]);
                LcdPrintCharS(1, 1, arrayMapOfPassword[randomNumberForNewPassword][1]);
                LcdPrintCharS(1, 2, arrayMapOfPassword[randomNumberForNewPassword][2]);
                LcdPrintCharS(1, 3, arrayMapOfPassword[randomNumberForNewPassword][3]);
            } else
                indexOfNumber = 1;
            if (timeDelay >= TIME_ENTER_PASSWORD_DELAY) {
                statusPassword = USING_ADMIN;
                indexOfNumber = 0;
            }
            break;
        case MAINTAINANCE_MODE:
            timeDelay++;
            if (maintainance_mode == 0)
                LcdPrintStringS(0, 0, "STATE: FINE     ");
            else
                LcdPrintStringS(0, 0, "STATE: BAO TRI  ");
            LcdPrintStringS(1, 0, "PRESS * TO TURN ");
            if (isButtonEnter()) {
                if (maintainance_mode == 0)
                    maintainance_mode = 1;
                else
                    maintainance_mode = 0;
                statusPassword = USING_ADMIN;
            }
            if (timeDelay >= TIME_ENTER_PASSWORD_DELAY / 2)
                statusPassword = USING_ADMIN;
            break;
        case MAINTAINANCE_MODE_USER:
            timeDelay++;
            LcdPrintStringS(0, 0, "THANG MAY DANG  ");
            LcdPrintStringS(1, 0, "BAO TRI         ");
            if (timeDelay >= TIME_ENTER_PASSWORD_DELAY)
                statusPassword = INIT_SYSTEM;
            break;
        case WRONG_PASSWORD:
            timeDelay++;
            LcdPrintStringS(0, 0, "PASSWORD WRONG  ");
            if (timeDelay >= TIME_ENTER_PASSWORD_DELAY / 2)
                statusPassword = INIT_SYSTEM;
            break;
        default:
            break;

    }
}