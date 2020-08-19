#include<SPI.h>
#include<nRF24L01.h>
#include<RF24.h>

/*********************************************************************
 * Global variables
 *********************************************************************/

//led pin definitions
#define LED_PIN 8
#define BOARD_LED 13

//rf channel
RF24 radio(9, 10); // CE pin 9, CSN pin 10
byte address[6] = "00001";

//ack message
unsigned char ack_msg[32] = {0};
unsigned char rx_buff[32] = {0};

//data flags
bool batt_up_down = true;
short vel_up = 0;
short vel_left = 0;
short pitch = 0;
short roll = 0;
short heading = 0;

/*********************************************************************
 * Helper functions
 ********************************************************************/

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

/* The purose of this short arduino programs is to provide ACK responses
 *  to input RF message over a nrf24 connection...
 */

void setup() {

  //setup serial channel - 115200, 8 bits, 1 stopbit, no parity
  Serial.begin(115200, SERIAL_8N1);
  
  //init led pin for power indication
  pinMode(LED_PIN, OUTPUT);
  pinMode(BOARD_LED, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BOARD_LED, HIGH);

  //define an ack message
  ack_msg[0] = 255; //msg header
  ack_msg[1] = 23; //msg length
  ack_msg[2] = 11; //msg id
  ack_msg[3] = 1; //sequence number - keep as const for now
  ack_msg[4] = 75; //batttery status
  vel_up = 1638;//velocity up value = 50.0 => 1638
  ack_msg[5] = vel_up; 
  ack_msg[6] = vel_up >> 8;
  vel_left = -1638;
  ack_msg[7] = vel_left; //velocity left value = -50.0 => - 1638
  ack_msg[8] = vel_left >> 8;
  ack_msg[9] = 0; //velocity forward value = 0.0 => 0
  ack_msg[10] = 0;
  heading = 100;
  ack_msg[11] = heading; //heading value = 100.0 => 18204
  ack_msg[12] = heading >> 8;
  ack_msg[13] = 16400; //altitude value = 500.5 => 16400
  ack_msg[14] = 16400 >> 8;
  pitch = -1000;
  ack_msg[15] = pitch; //pitch value = -100.0 => -18204
  ack_msg[16] = pitch >> 8;
  roll = 3000;
  ack_msg[17] = roll; //roll value = 21.0 => 3823
  ack_msg[18] = roll >> 8;
  ack_msg[19] = 100; //eng1 status
  ack_msg[20] = 50; //eng2 status
  ack_msg[21] = 20; //eng3 status
  ack_msg[22] = 100; //eng4 status
  ack_msg[23] = 0; //fault status 
  ack_msg[24] = getCRC(ack_msg,23); //calc the CRC value

  //init all rf channel setup
  radio.begin();
  radio.setDataRate(RF24_250KBPS); //RF24_250KBPS
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.openReadingPipe(1,address);
  radio.setRetries(1,5); 
  radio.setChannel(0); //defualt value
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
  radio.writeAckPayload(1, ack_msg, 32*sizeof(unsigned char));
  
}

void loop() {

  //listen to arriving messages and respond with ack message
  if (radio.available())
  {
    //turn on LED
    digitalWrite(LED_PIN, HIGH);

    //read data to clear the buffer - TODO...
    radio.read(rx_buff,32);
    delay(5); //Need to wait some time between readin data from rf device and writing ack message data
    //This should be at least > max_transmissio_time

    //print out received data - used for debug only - TODO
    Serial.write(rx_buff[0]);
    Serial.write(rx_buff[1]);
    Serial.write(rx_buff[2]);
    Serial.write(rx_buff[3]);
    Serial.write(rx_buff[4]);
    Serial.write(rx_buff[5]);
    Serial.write(rx_buff[6]); //NOTE: The bytes which were not 
    Serial.write(rx_buff[7]); // filled with last received value

    //update status message - see that the data changes on the other side
    //update the battery status
    if (batt_up_down)
    {
      ack_msg[4]+=1;
      if (ack_msg[4] > 100) batt_up_down = false;
    }
    else
    {
      ack_msg[4]-=1;
      if (ack_msg[4] == 0) batt_up_down = true;
    }

    //upate velocity status
    vel_up+=100;
    ack_msg[5] = vel_up; 
    ack_msg[6] = vel_up >> 8;
    vel_left+=100;
    ack_msg[7] = vel_left;
    ack_msg[8] = vel_left >> 8;

    //update attitude
    heading+=10;
    ack_msg[11] = heading; 
    ack_msg[12] = heading >> 8;
    pitch+=10;;
    ack_msg[15] = pitch; 
    ack_msg[16] = pitch >> 8;
    roll+=10;
    ack_msg[17] = roll; 
    ack_msg[18] = roll >> 8;

    //re-calc CRC value
    ack_msg[24] = getCRC(ack_msg,23); //calc the CRC value

    //clear input buffer
    memset(rx_buff, 0, 32*sizeof(unsigned char)); // reset sio_tx_buffer

    //pupdate ack message content
    radio.writeAckPayload(1, ack_msg, 32*sizeof(unsigned char));


  }
  
}
