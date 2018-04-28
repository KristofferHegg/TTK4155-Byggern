/*
--------------------------------------------------------------------------------
Company: Gruppe 20
Engineer: Kristoffer, Mats og Axel

Create Date: 13.11.2017 
Design Name: Node 3 
Module Name:
Project Name: Byggern 2017
Target Devices: Mbed LPC1768
Tool versions: Mbed OS 5
Description: Prototype for node 3. Implemented Serial communication with PC, 
             PWM, CAN, Pin change interrupt, Timer compare interrupt, 
             Linked List, ect.

Dependencies:

Revision:
Revision 0.01 - File Created
Aditional Coments:
--------------------------------------------------------------------------------
*/
#include "mbed.h"
#include <string> 
#include "List.h"

using namespace std;

/*Peripherals Declaration*/
Serial pc(USBTX, USBRX);
CAN can1(p9, p10);
CAN can2(p30, p29);

InterruptIn myButton(p22);
AnalogIn myPotentiometer(p20);

DigitalOut myLed1(p21);
BusOut myLeds(LED1, LED2, LED3, LED4);
PwmOut myLed2(p23);

Ticker ticker;

/*Function Declaration*/
void toggle_led(void);
void led_bus_led_pwm(void);
void can_recieve(void);
void can_handle(void);
void pc_recieve(void);
int pc_handle(char *pc_msg, int cmd);

/*Global Variable Declarations*/
CANMessage can_msg; //msg.id, msg.data[x], msg.len
volatile bool can_flag = false;

char pc_buffer[32] = {'\0'};
volatile int pc_buffer_counter = 0;
volatile bool pc_flag = false;

List high_score;

/*
--------------------------------------------------------------------------------
Function: Main.
Description: Enables handling of CAN and Serial messages. 
Parameters: int cmd - Approved command to be performed 
Global: bool can_flag - CAN Rx interrupt flag
        bool pc_flag - Serial port Rx interupt flag
Returns: Infinite loop.
See Also: <can_handle>, <pc_handle>
--------------------------------------------------------------------------------
*/

int main(){
    
    myButton.mode(PullUp); //Internal pull up resistor.
    wait(0.1); //Recomended wait time. 
    myButton.fall(&toggle_led); //Attach function to pin INT.
    
    pc.attach(&pc_recieve); //Attach function to Rx INT
    
    ticker.attach(&led_bus_led_pwm, 5); //Attach function to timer INT.
    
    myLed2.period(1.0); //Set PWM period to 1 second.
    
    can1.attach(&can_recieve, CAN::RxIrq); //Attach function to can INT.
    
    high_score.AddNode("Kristoffer"); //Initial Registered Players
    high_score.AddNode("Mats");
    high_score.AddNode("Axel");
    
    pc.printf("Player Configuration\r\n");
    pc.printf("Add - Player\r\n");
    pc.printf("Delete - Player\r\n");
    pc.printf("Print - Players\r\n");
    
    //high_score.PrintList();
    
    int cmd = 0;
    
    while(1){
        if(can_flag){
            can_handle();   
        }
        
        if(pc_flag){
            cmd = pc_handle(pc_buffer, cmd);
        }
    }
}
/*
--------------------------------------------------------------------------------
Function: Toggle led and transmit CAN message.
Description: Toggle led and send CANMessage when button is pressed 
             (pin change interrupt).
Parameters: 
Global: Altered: DigitalOut myLed1 - Light On/Off. 
Returns: void
See Also: <main>
--------------------------------------------------------------------------------
*/
void toggle_led(void){
    myLed1 = !myLed1; 
    
    char data[4] = {'B', 'O', 'O', 'M'};
    can2.write(CANMessage(30, data, 4)); //int adress, char *data, char len.   
}
/*
--------------------------------------------------------------------------------
Function: Edit BusOut, PWM and transmit CAN message. 
Description: Analog input edit 4-bit value of BusOut and pulsewith of pwm 
             when ticker is acivated (timer compare interrupt). Transmitt 
             CANmessage that contains potentiometer value (0-100%). 
Parameters: float ain - Analog input from potentiometer
Global: Altered: BusOut myleds - 4-bit LED representing the values 0-15.
        Altered: PwmOut myLed2 - Blinky w/ pulsewith modulation. 
Returns: void
See Also: <main>
--------------------------------------------------------------------------------
*/
void led_bus_led_pwm(void){
    float ain = myPotentiometer;
    myLeds = (15*ain);
    myLed2.pulsewidth(ain);
    
    char data[3];
    data[0] = ain*100;
    data[1] = ain*100;
    can2.write(CANMessage(20, data , 2));        
}
/*
--------------------------------------------------------------------------------
Function: Interupt Routine to recieve CAN message from can1.
Description: Read CAN message and set flag when there is a message available for
             reading (CAN::RxIrq).
Parameters: 
Global: Altered: CANMessage can_msg - recieved message.
        Altered: bool can_flag - CAN Rx interrupt flag.
Returns: void
See Also: <main>, <can_handle>
--------------------------------------------------------------------------------
*/
void can_recieve(void){
    if(can1.read(can_msg)){ 
        can_flag = true; //Have to read msg to reset CAN::RxIrq.
    }
}
/*
--------------------------------------------------------------------------------
Function: Handle CAN message.
Description: Performs different operations depending on the CAN message id. 
Parameters: 
Global: CANMessage can_msg - recieved message.
        Altered: bool can_flag - CAN Rx interrupt flag.
Returns: void
See Also: <main>, <can_recieve>
--------------------------------------------------------------------------------
*/
void can_handle(void){
    switch(can_msg.id){
        case 10:
            break;
        case 20: 
            printf("Adress: %d", can_msg.id);
            printf(" - Data: ");
            for(int i = 0; i < can_msg.len; i++){
                printf("%d - ", can_msg.data[i]);    
            }
            printf("Lenght: %d\r\n", can_msg.len);
            break; 
        case 30:
            printf("Adress: %d - Data: %s - Lenght: %d\r\n", can_msg.id, can_msg.data, can_msg.len); //Print string
            break;
        default: printf("Invalid Address!\r\n"); break;      
    }        
    can_flag = false;          
}

/*
--------------------------------------------------------------------------------
Function: Interupt Routine to read in data from serial port.
Description: Reads serial bus and store characters in buffer or check for valid 
             command at enter
Parameters: char pc_buffer_char - The character read by the serial bus.
Global: Altered: char pc_buffer - Serial Rx buffer.
        Altered: int pc_buffer_counter - Number of characters in buffer.
        Altered: bool pc_flag - Serial port Rx interupt flag.
Returns: void
See Also: <main>, <pc_handle> 
--------------------------------------------------------------------------------
*/
void pc_recieve(void){  
    char pc_buffer_char = pc.getc();
    
    switch(pc_buffer_char){
        case 127:
            pc_buffer[pc_buffer_counter] = '\0';
            if(pc_buffer_counter > 0){
                pc_buffer_counter--;
            }
            pc_buffer[pc_buffer_counter] = '\0';
            break;
        case '\r':  
            pc_flag = true;
            break;
        default:
            pc_buffer[pc_buffer_counter] = pc_buffer_char;
            pc_buffer_counter++; 
            break;     
    }
}
/*
--------------------------------------------------------------------------------
Function: Handle pc command.
Description: Tests the string against possible commands. Performs command and / 
             or returns results used to handle the next command.
Parameters: char *pc_msg - Pointer to string, containing the command, we want to test.
            int cmd - Result of command (Add, Delete or Print player(s)).
Global: Altered:
Returns: int cmd
See Also: <main>, <pc_recieve>, <List.h & List.cpp>
--------------------------------------------------------------------------------
*/
int pc_handle(char *pc_msg, int cmd){
        
    if((strcmp(pc_msg,"Add") == 0)|| cmd == 1){
        if(cmd == 1){
            cmd = 0;
            high_score.AddNode(pc_msg);
        }else{
            cmd = 1;
            pc.printf("Add Player\r\n");     
        }
    }else if(strcmp(pc_msg,"Delete") == 0 || cmd == 2){
        if(cmd == 2){
            cmd = 0;
            high_score.DeleteNode(pc_msg);
        }else{
            cmd = 2;
            pc.printf("Delete Player\r\n");
        } 
    }else if(strcmp(pc_msg,"Print") == 0){
        high_score.PrintList();
    }else{
        printf("Invalid: ");
        for(int i=0; i <= pc_buffer_counter; i++){
            pc.printf("%c", pc_buffer[i]);
        }
        printf("\r\n");
    }
    
    for(int i=0; i <= pc_buffer_counter; i++){
        pc_buffer[i] = '\0';
    }
    
    pc_buffer_counter = 0;
    pc_flag = false;
    
    return cmd;    
}


