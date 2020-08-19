#include <SD.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/*********************************************************************************
 * Global variables
 ********************************************************************************/
//LEDs
#define LED_PIN 8
#define BOARD_LED 13

int led_pin = 8; //led pin number
unsigned char sio_rx_buffer[32] = {0}; //buffer for data arriving from the PC
unsigned char sio_tx_buffer[32] = {0}; //buffer for data sent to the PC
unsigned char rf_rx_buffer[32] = {0}; //buffer for data arriving from the RF channel
bool rf_comm_active = false;
RF24 radio(9, 10); // CE pin 9, CSN pin 10

/********************************************************************************
 *Sub Functions
 ********************************************************************************/

//Function getCRC returns the CRC8 value for the given buffer
//with the polynomial 0x91
int getCRC(unsigned char * buff, int buff_len)
{
  unsigned char  CRC7_POLY = 0x91;

  unsigned char i, j, crc = 0;

  for (i = 0; i < buff_len; i++)
  {
    crc ^= buff[i];
    for (j = 0; j < 8; j++)
    {
      if (crc & 1)
        crc ^= CRC7_POLY;
      crc >>= 1;
    }
  }
  return crc;
}


//Function int2strAddress converts an integer value to a string value
//for the nrf24 setup string
void int2strAddress(unsigned char int_addr, byte (*address)[6])
{
  //note: byte is always 6 element long
  int temp = int_addr;

  //init address
  byte * add = *address;
  for (int i = 0; i<5; i++)
  {
    add[i] ='0';
  }

  int index = 4;

  while (temp > 0 && index >= 0)
  {
    char digit = temp % 10;
    temp = temp / 10;
    add[index] = '0' + digit;
    index--;
  }

}

//Function handleSerialComm performs all serial coomunication related operations
void handleSerialComm()
{
  //static variables
  static bool msg_header_flag = false;
  static bool msg_len_flag = false;
  static unsigned char msg_len = 0;
  static unsigned char msg_counter = 0;

  unsigned char val = 0;


  if (Serial.available() > 0 ) //if there's data in the serial buffer  handle it
  {
    //read a byte of data
    val  = Serial.read();
    
    if (!msg_header_flag && val == 255) //try to read a message header
    {
      sio_rx_buffer[msg_counter] = val;
      msg_header_flag = true;
      msg_counter++;
      return;
    }

    if(!msg_len_flag && msg_header_flag) //read the message header
    {
      sio_rx_buffer[msg_counter] = val;
      msg_len = val;
      msg_len_flag = true;
      msg_counter++;
      return;
    }

    if(msg_len_flag && msg_header_flag && msg_len > 0) //if both flags are read - handle the data
    {
       sio_rx_buffer[msg_counter] = val;
       msg_counter++;
       if (msg_counter >= msg_len + 2)
          { 
            //Check which kind of message was received (setup or data and handle accordingly)
            //read the message id
            unsigned char msg_id = sio_rx_buffer[2];
 
            switch(msg_id)
            {
              case 253: //received a setup data message
                 {
                    //check setup request validity
                   unsigned char tx_address = sio_rx_buffer[3];
                   unsigned char rx_address = sio_rx_buffer[4];
                   unsigned char rf_channel = sio_rx_buffer[5];
                   unsigned char power = sio_rx_buffer[6];
                   unsigned char msg_crc = sio_rx_buffer[7];
  
                   if (msg_crc == getCRC(sio_rx_buffer, 7))
                   {
                      //init comm
                      radio.begin();
                      //set data rate
                      radio.setDataRate(RF24_250KBPS); //RF24_250KBPS, RF24_1MBPS
                      //set tx address value
                      byte address[6] = "00000"; 
                      int2strAddress(tx_address, &address);
                      
                      radio.openWritingPipe(address);
                      //set rx setup - Enable ack messages
                      radio.setAutoAck(true);
                      radio.enableAckPayload();
                      //set retries  - 5 x 250 [us] = 0.75 [ms] delay between re-tries
                      //5 retries. Max tranmission time = 0.75 x retries + t_propagation ~= 3.75 [ms]
                      radio.setRetries(5,5); 
                      //set channel value - limit at 125
                      if (rf_channel > 125) rf_channel = 125;
                      radio.setChannel(rf_channel);
                      //set power - range 0 - 3
                      switch(power)
                      {
                        case 0:
                          radio.setPALevel(RF24_PA_MIN);
                          break;
                        case 1:
                          radio.setPALevel( RF24_PA_LOW);
                          break;
                        case 2:
                          radio.setPALevel(RF24_PA_HIGH);
                          break;
                        default:
                          radio.setPALevel(RF24_PA_MAX);
                          break;
                      }
                      //set radio to tx mode
                      radio.stopListening();

                      //mark rf_comm as active
                      rf_comm_active = true;
  
                      //send setup ACK message
                      sio_tx_buffer[0] = 255;
                      sio_tx_buffer[1] = 3;
                      sio_tx_buffer[2] = 254;
                      sio_tx_buffer[3] = 1; //This is always true - redundant(?)
                      sio_tx_buffer[4] = getCRC(sio_tx_buffer,4); 
                      Serial.write(sio_tx_buffer, 5);   
                      memset(sio_tx_buffer, 0, 32*sizeof(unsigned char)); // reset sio_tx_buffer
                   }
                 }
                 break;
              case 254: //reserved value
                //do nothing
                break;
              case 255: //reserved value
                //do nothing
                break;
              default: //all other case
              {
                //Send data to the rf channel only if rf channel was set-up
                if (rf_comm_active == true)
                {
                  //send data from rx buffer to rf channel
                  radio.write(sio_rx_buffer, 32); 
                  //a full 32 bytes frame is sent to the rf channel. If the message is sent with msg_len and msg_len < 32
                  //then the last bytes in the nrf24 protocol (layer 2) are padded with the last value in the sio_rx_buffer
                  //at sio_rx_buffer[msg_len-1]
                  
                }
                
                break;
              }
            }

            //Reset all variables
            msg_header_flag = false;
            msg_len_flag = false;
            msg_len = 0;
            msg_counter = 0;
            
            //reset rx buffer
            memset(sio_rx_buffer, 0, 32*sizeof(unsigned char));
          }
      }
   }
    
}
   

//Function handleRFComm performs all RF communication related operations
void handleRFComm() 
{
  int ack_msg_len = 0; 
  
  //if rf_comm_active start listening for an ACK message
  if (rf_comm_active == true) //rf channel is active
  {
    //check if ACK data is availible
    if (radio.isAckPayloadAvailable())
    { 
      
      //read ACK data - This reads the full ack message
      radio.read(rf_rx_buffer,32); 

      //check message length field
      ack_msg_len  = rf_rx_buffer[1];
      if (ack_msg_len > 30) ack_msg_len = 30; //30 is the maximum message size

      //send the ACK data to the serial port
      Serial.write(rf_rx_buffer, ack_msg_len + 2);

      //clear the rf rx buffer
      memset(rf_rx_buffer, 0, 32*sizeof(unsigned char));
    }
    
  }
  
  return;
}
/********************************************************************************
 * Main Functions
 *******************************************************************************/

void setup() {
  //setup serial channel - 115200, 8 bits, 1 stopbit, no parity
  Serial.begin(115200, SERIAL_8N1);

  //init led pin
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BOARD_LED, LOW);

}

void loop() {

  //handle serial interfaces
  handleSerialComm();

  //handle RF interface
  handleRFComm();
  
}
