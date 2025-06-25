/*
--------------------------------------------------------------------------------
-- PROJECT: ALTO FPGA Miner. Arduino Firmware for Control & Monitoring
--------------------------------------------------------------------------------
-- AUTHORS: Olivier FAURIE <olivier.faurie.hk@gmail.com>
-- LICENSE: 
-- WEBSITE: https://github.com/olivierHK
--------------------------------------------------------------------------------
*/

#include <float.h>


////////////////////////////////////////SYSMON I2C DEFINE///////////////////////////////////////////////////
#define SDA_PORT PORTB
#define SDA_PIN 3 // = B3
#define SCL_PORT PORTB
#define SCL_PIN 4 // = B4

#define I2C_TIMEOUT 1000
#define I2C_FASTMODE 1

#include <SoftI2CMaster.h>


#define I2C_PCA_nRST_CTRL_PIN       8 //is 5
#define I2C_PCA_7BITADDR            0x74 //set as default as 0x74. 
#define PCA_CH_SELECT               0x02 //Sysmon is on Channel 1.

#define I2C_SYSMON_SLR0_7BITADDR    0x32 // 011_0010 (I2C measured is 0V, as Vp/Vn tied to ground)
#define I2C_SYSMON_SLR1_7BITADDR    0x30 // 011_0000 (I2C measured is 0V, as Vp/Vn tied to ground)
#define I2C_SYSMON_SLR2_7BITADDR    0x43 // 100_0011 (I2C measured is 0V, as Vp/Vn tied to ground)
#define I2C_SYSMON_SLR3_7BITADDR    0x41 // 100_0001 (I2C measured is 0V, as Vp/Vn tied to ground)

#define DRP_SYSMON_TEMP_8BITADDR    0x00
#define DRP_SYSMON_VCCINT_8BITADDR  0x01
#define DRP_SYSMON_VCCAUX_8BITADDR  0x02
#define DRP_SYSMON_VCCBRAM_8BITADDR 0x06

#define DRP_SYSMON_TEMP_MIN_8BITADDR    0x24
#define DRP_SYSMON_VCCINT_MIN_8BITADDR  0x25
#define DRP_SYSMON_VCCAUX_MIN_8BITADDR  0x26
#define DRP_SYSMON_VCCBRAM_MIN_8BITADDR 0x27

#define DRP_SYSMON_TEMP_MAX_8BITADDR    0x20
#define DRP_SYSMON_VCCINT_MAX_8BITADDR  0x21
#define DRP_SYSMON_VCCAUX_MAX_8BITADDR  0x22
#define DRP_SYSMON_VCCBRAM_MAX_8BITADDR 0x23

#define DRP_SYSMON_FLAG_REGISTER_8BITADDR 0x3F

#define DRP_SYSMON_RESET_8BITADDR    0x03

//reading back SLR Temperature
uint16_t TEMP_SLR0   =0, TEMP_SLR0_MIN   =0xFFFF, TEMP_SLR0_MAX   =0;
uint16_t TEMP_SLR1   =0, TEMP_SLR1_MIN   =0xFFFF, TEMP_SLR1_MAX   =0;
uint16_t TEMP_SLR2   =0, TEMP_SLR2_MIN   =0xFFFF, TEMP_SLR2_MAX   =0;
//reading back SLR VCCINT
uint16_t VCCINT_SLR0 =0, VCCINT_SLR0_MIN =0xFFFF, VCCINT_SLR0_MAX =0;
uint16_t VCCINT_SLR1 =0, VCCINT_SLR1_MIN =0xFFFF, VCCINT_SLR1_MAX =0;
uint16_t VCCINT_SLR2 =0, VCCINT_SLR2_MIN =0xFFFF, VCCINT_SLR2_MAX =0;
//reading back SLR VCCAUX
uint16_t VCCAUX_SLR0 =0, VCCAUX_SLR0_MIN =0xFFFF, VCCAUX_SLR0_MAX =0;
uint16_t VCCAUX_SLR1 =0, VCCAUX_SLR1_MIN =0xFFFF, VCCAUX_SLR1_MAX =0;
uint16_t VCCAUX_SLR2 =0, VCCAUX_SLR2_MIN =0xFFFF, VCCAUX_SLR2_MAX =0;
//reading back SLR VCCBRAM
uint16_t VCCBRAM_SLR0=0, VCCBRAM_SLR0_MIN=0xFFFF, VCCBRAM_SLR0_MAX=0;
uint16_t VCCBRAM_SLR1=0, VCCBRAM_SLR1_MIN=0xFFFF, VCCBRAM_SLR1_MAX=0;
uint16_t VCCBRAM_SLR2=0, VCCBRAM_SLR2_MIN=0xFFFF, VCCBRAM_SLR2_MAX=0;

uint16_t I2C_SYSMON_STATUS = 0x0000;
uint16_t I2C_SYSMON_FLAG_SLR0 = 0x0000;
uint16_t I2C_SYSMON_FLAG_SLR1 = 0x0000;
uint16_t I2C_SYSMON_FLAG_SLR2 = 0x0000;
bool     monitor_SYSMON_FPGA1 = false;
/////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <Wire.h>

////////////SIPO Pin enumeration////////////
#define SIPO_latchPin   4				  // SIPO LAtch Pin. Udate SIPO chip output when asserted Low. 
#define SIPO_clockPin   2				  // SIPO Serial clock out.
#define SIPO_dataOutPin 3				  // SIPO Serial data out.
#define SIPO_nEO        13                // Hi-Z the SIPO output and 1V8 Level shifter output if High
////////////////////////////////////////////

////////////PISO Pin enumeration//////////// 
#define PISO_loadPin    8				  // PISO Laod Pin. Update PISO chip Input registers when asserted LOW.
#define PISO_dataInPin  7				  // PISO serial data input.
#define PISO_clockPin   6				  // PISO Serial clock out.
////////////////////////////////////////////

//set the Rd/Wr registers as 32 bits containers.
uint32_t SIPO_REG = 0;
uint32_t PISO_REG = 0;

uint16_t PM_READ_STATUS_REG[2] = {0};
uint16_t PM_READ_STATUS_ADDR = 0x79;

uint16_t PM_READ_VIN_REG = 0;
uint16_t PM_READ_VIN_ADDR = 0x88;

uint16_t PM_READ_IIN_REG = 0;
uint16_t PM_READ_IIN_ADDR = 0x89;

uint16_t PM_READ_PIN_REG = 0;
uint16_t PM_READ_PIN_ADDR = 0x97;

uint16_t PM_READ_VOUT_REG[2] = {0};
uint16_t PM_READ_VOUT_ADDR = 0xD4;

uint16_t PM_READ_IOUT_REG[2] = {0};
uint16_t PM_READ_IOUT_ADDR = 0x8C;

uint16_t PM_READ_POUT_REG[2] = {0};
uint16_t PM_READ_POUT_ADDR = 0x96;

uint16_t PM_READ_TOUT_REG[2] ={0};
uint16_t PM_READ_TOUT_ADDR = 0x8D;

double ADC_array[4];
double ADC_array_min[4]={ DBL_MAX, DBL_MAX, DBL_MAX, DBL_MAX };
double ADC_array_max[4]={ -DBL_MAX, -DBL_MAX, -DBL_MAX, -DBL_MAX };

bool monitor_PM_FPGA1= false;


//Protection en, Min/MAx values declaration
//ADC channels
uint16_t ptt_ADC_array_en_nReset_Shutdown[4]={ 0x0000,  0x000, 0x000, 0x000};
double ptt_ADC_array_min[4]={ 11.8, 4.80, 3.10, 1.60 };
double ptt_ADC_array_max[4]={ 12.20, 5.20, 3.50, 2.00 };
//SLR Temperature
uint16_t ptt_TEMP_SLR_en_nReset_Shutdown   =0x0000;
uint16_t ptt_TEMP_SLR_MIN   =0xFFFF, ptt_TEMP_SLR_MAX   =0;
//SLR VCCINT
uint16_t ptt_VCCINT_SLR_en_nReset_Shutdown =0x0000;
uint16_t ptt_VCCINT_SLR_MIN =0xFFFF, ptt_VCCINT_SLR_MAX =0;
//SLR VCCAUX
uint16_t ptt_VCCAUX_SLR_en_nReset_Shutdown =0x0000;
uint16_t ptt_VCCAUX_SLR_MIN =0xFFFF, ptt_VCCAUX_SLR_MAX =0;
//SLR VCCBRAM
uint16_t ptt_VCCBRAM_SLR_en_nReset_Shutdown=0x0000;
uint16_t ptt_VCCBRAM_SLR_MIN=0xFFFF, ptt_VCCBRAM_SLR_MAX=0;




void setup() {

	pinMode(SIPO_latchPin, OUTPUT);	
	digitalWrite(SIPO_latchPin, LOW);
	//Make the the SIPO_nEO is set as an output and is high. 
	pinMode(SIPO_nEO, INPUT_PULLUP);// 
	pinMode(SIPO_nEO, OUTPUT);      //
	digitalWrite(SIPO_nEO, HIGH);   //

	//Make the the PISO Laod Pin is set as an output and is high.	
	pinMode(PISO_loadPin, INPUT_PULLUP);//
	pinMode(PISO_loadPin, OUTPUT);      //
	digitalWrite(PISO_loadPin, HIGH);   //
	pinMode(PISO_clockPin, INPUT_PULLUP);//
    pinMode(PISO_clockPin, OUTPUT);
    digitalWrite(PISO_clockPin, HIGH);
    pinMode(PISO_dataInPin, INPUT);

	
	//set serial port.
	Serial.begin(500000);
	Serial.setTimeout(3000);

	//set PMBus port on the I2C port;
     Wire.begin();
     Wire.setClock(400000);
     sendBytePMBus(0x60, 0x03);//Clear TPS Fault for clean start.


    //run a power sequence startup.
	SIPO_REG = SIPO_PowerStartupSequence();
	
	//read the inputs
	PISO_REG = PISO_Read();

	/////////////////////////Init the I2C Sysmon Bus and PCA Multiplexer////////////////
    ///////////////////////////To be done after the FPGA to be Powered//////////////////
    //////init the I2C  bitbang for Sysmon.////////////////////////////
    if (!i2c_init()){ // Initialize everything and check for bus lockup
        //Serial.println("I2C init failed");
    	I2C_SYSMON_STATUS=0x8000;
    }	
    //////////////reset the I2C bus Multiplexer chip PCA///////////////
    pinMode(I2C_PCA_nRST_CTRL_PIN, OUTPUT);
    digitalWrite(I2C_PCA_nRST_CTRL_PIN, LOW);    
    delay(100);
    digitalWrite(I2C_PCA_nRST_CTRL_PIN, HIGH);
    delay(100);
    /////////////////////////////////////////////////////////////////// 

    ////////////////////////////////////////////////////////////////////////////////////
} 



// the loop routine runs over and over again forever:
void loop() {
  double sensorValue=0;

  for(int i=0; i<=3; i++)
  {
    //sensorValue = double(analogRead(i));

    sensorValue= read_ADC(i);
    ADC_array[i]= display_ADC_Volt(sensorValue, i);
    //Serial.print(display_ADC_Volt(sensorValue, i));
    //Serial.print("V, ");

    if (ADC_array[i] > ADC_array_max[i])
    {
    	ADC_array_max[i] = ADC_array[i];
    }

    if (ADC_array[i] < ADC_array_min[i])
    {
    	ADC_array_min[i] = ADC_array[i];
    }


  } 

  //Read PISO(input) chip.
  PISO_REG = PISO_Read();


  //SMBUS Readback.
  if (monitor_PM_FPGA1)
  {
	  PM_READ_VIN_REG = readWordPMBus(0x60, PM_READ_VIN_ADDR);
	  PM_READ_IIN_REG = readWordPMBus(0x60, PM_READ_IIN_ADDR);
	  PM_READ_PIN_REG = readWordPMBus(0x60, PM_READ_PIN_ADDR);

	  //CHA (VCCINT) Telemetry & Status
	  writeBytePMBus(0x60, 0x00, 0x00);
	  PM_READ_STATUS_REG[0] = readWordPMBus(0x60, PM_READ_STATUS_ADDR);

	  PM_READ_VOUT_REG[0] = readWordPMBus(0x60, PM_READ_VOUT_ADDR);
	  PM_READ_IOUT_REG[0] = readWordPMBus(0x60, PM_READ_IOUT_ADDR);
	  PM_READ_POUT_REG[0] = readWordPMBus(0x60, PM_READ_POUT_ADDR);
	  PM_READ_TOUT_REG[0] = readWordPMBus(0x60, PM_READ_TOUT_ADDR);

	  //CHB (VCCINTIO_BRAM) Telemetry & Status
	  writeBytePMBus(0x60, 0x00, 0x01);
	  PM_READ_STATUS_REG[1] = readWordPMBus(0x60, PM_READ_STATUS_ADDR);

	  PM_READ_VOUT_REG[1] = readWordPMBus(0x60, PM_READ_VOUT_ADDR);
	  PM_READ_IOUT_REG[1] = readWordPMBus(0x60, PM_READ_IOUT_ADDR);
	  PM_READ_POUT_REG[1] = readWordPMBus(0x60, PM_READ_POUT_ADDR);
	  PM_READ_TOUT_REG[1] = readWordPMBus(0x60, PM_READ_TOUT_ADDR);
	}  


	//I2C Sysmon Readback.
  	if (monitor_SYSMON_FPGA1)
  	{
    	//select channel on the I2C multiplexer for Sysmon (Ch. 1, 0010b, so 0x2). Checking is correct channel first.
    	if(!update_I2C_PCA_CH_Select(I2C_PCA_7BITADDR, PCA_CH_SELECT) )
    	{
    	    //Serial.println("Update PCA Channel failed");
    	    I2C_SYSMON_STATUS= (I2C_SYSMON_STATUS | 0x4000);
    	}
    	else
    	{
    	    //reading back SLR Temperature
    	    TEMP_SLR0 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_TEMP_8BITADDR);
    	    TEMP_SLR1 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_TEMP_8BITADDR);
    	    TEMP_SLR2 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_TEMP_8BITADDR);

    	    //reading back SLR VCCINT
    	    VCCINT_SLR0 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCINT_8BITADDR);
    	    VCCINT_SLR1 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCINT_8BITADDR);
    	    VCCINT_SLR2 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCINT_8BITADDR);
    	
    	    //reading back SLR VCCAUX
    	    VCCAUX_SLR0 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCAUX_8BITADDR);
    	    VCCAUX_SLR1 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCAUX_8BITADDR);
    	    VCCAUX_SLR2 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCAUX_8BITADDR);
    	    
    	    //reading back SLR VCCBRAM
    	    VCCBRAM_SLR0 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCBRAM_8BITADDR);
    	    VCCBRAM_SLR1 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCBRAM_8BITADDR);
    	    VCCBRAM_SLR2 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCBRAM_8BITADDR); 

    	    /////////////////////////update MIN/MAX temp///////////////////////////////
    	    TEMP_SLR0_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_TEMP_MIN_8BITADDR);
    	    TEMP_SLR1_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_TEMP_MIN_8BITADDR);
    	    TEMP_SLR2_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_TEMP_MIN_8BITADDR);

    	    TEMP_SLR0_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_TEMP_MAX_8BITADDR);
    	    TEMP_SLR1_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_TEMP_MAX_8BITADDR);
    	    TEMP_SLR2_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_TEMP_MAX_8BITADDR);
    	    /////////////////////////////////////////////////////////////////////////// 

    	    // SLR1, SLR2 Min/Max Non-Functionnal on VU9P sysmon.
    	    ////////////////////////////////////update MIN/MAX VCCINT//////////////////////////////
    	    VCCINT_SLR0_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCINT_MIN_8BITADDR);
    	    //VCCINT_SLR1_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCINT_MIN_8BITADDR);
    	    //VCCINT_SLR2_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCINT_MIN_8BITADDR);
    	    //
    	    VCCINT_SLR0_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCINT_MAX_8BITADDR);
    	    //VCCINT_SLR1_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCINT_MAX_8BITADDR);
    	    //VCCINT_SLR2_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCINT_MAX_8BITADDR);
    	    //////////////////////////////////////////////////////////////////////////////////////
    	    //
    	    ////////////////////////////////////update MIN/MAX VCCAUX//////////////////////////////
    	    VCCAUX_SLR0_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCAUX_MIN_8BITADDR);
    	    //VCCAUX_SLR1_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCAUX_MIN_8BITADDR);
    	    //VCCAUX_SLR2_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCAUX_MIN_8BITADDR);
    	    //
    	    VCCAUX_SLR0_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCAUX_MAX_8BITADDR);
    	    //VCCAUX_SLR1_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCAUX_MAX_8BITADDR);
    	    //VCCAUX_SLR2_MAX = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCAUX_MAX_8BITADDR);
    	    ///////////////////////////////////////////////////////////////////////////////////////
    	    //
    	    //////////////////////////////////////update MIN/MAX VCCBRAM////////////////////////////////
    	    VCCBRAM_SLR0_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCBRAM_MIN_8BITADDR);
    	    //VCCBRAM_SLR1_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCBRAM_MIN_8BITADDR);
    	    //VCCBRAM_SLR2_MIN = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCBRAM_MIN_8BITADDR);
    	    //
    	    VCCBRAM_SLR0_MAX =dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_VCCBRAM_MAX_8BITADDR);
    	    //VCCBRAM_SLR1_MAX =dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_VCCBRAM_MAX_8BITADDR);
    	    //VCCBRAM_SLR2_MAX =dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_VCCBRAM_MAX_8BITADDR);
    	    ////////////////////////////////////////////////////////////////////////////////////////////

    	    ///////////////////////////////////////update SYSMON Flag///////////////////////////////////////////////////
    	    I2C_SYSMON_FLAG_SLR0 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_FLAG_REGISTER_8BITADDR);
			I2C_SYSMON_FLAG_SLR1 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_FLAG_REGISTER_8BITADDR);
			I2C_SYSMON_FLAG_SLR2 = dBytes_I2C_Sysmon_Read( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_FLAG_REGISTER_8BITADDR);
			////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	}
    }	








    /////////////UART RX/TX address and command decoding///////////////////////////////
    int index = 0;
    bool newData = false;
    char Data_Received[9]= {'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B' , '\0'};
    char cmd[]  = {'0', '\0'};
    char addr[] = {'0','0','0', '\0'};
    char data[] = {'0','0','0','0', '\0'};

    while( Serial.available() > 0 )// > 0 && index < 7)
    {
	
    	Data_Received[index] = Serial.read();
    	delay(1);
    	index++;
    	newData = true;
    }

    memcpy(cmd , Data_Received  , 1);
    memcpy(addr, Data_Received+1, 3);
    memcpy(data, Data_Received+4, 4);

    char* ptr;
    uint16_t dataUint16t = strtol (data, &ptr, 16);
    //Serial.println(Data_Received);
    //char Data_Received[6] = {""0""}; //used for storing received character
    //for( int i = 0; i<= 5; i++)
    //{
    //Data_Received = Serial.readString();
    	//Data_Received[i] = Serial.read();   // Read the Data
    	//Serial.println(Data_Received);
    	//Serial.println(Data_Received[i], HEX);
    //}	
    //delay(1);

    if (newData == true)
    {
    	newData = false;
	    if ( addr[0] == '0') //0x000 Info sector
	    	if( addr[1] == '0') //0x000 Info sector
	    		Serial.println("ALTO+ ARD_FIRMWARE V1.0");
	    	if( addr[1] == '1') //0x010 enable PM BUS monitor section
	    	{
	    		if (data[3] == '1')
	    			monitor_PM_FPGA1 = true;
	    		else
	    			monitor_PM_FPGA1 = false;	    	
	    		Serial.println(1);
	    	}
	    	if( addr[1] == 'A') //0x0110 enable I2C Sysmon monitor section
	    	{
	    		if (data[3] == '1')
	    			monitor_SYSMON_FPGA1 = true;
	    		else
	    			monitor_SYSMON_FPGA1 = false;	    	
	    		Serial.println(1);
	    	}

	    else if (addr[0] == '1')// 0x100 ADC Sector
	    {
	    	if (strcmp(Data_Received,"01000000") == 0)
	    		Serial.println(ADC_array[0]);
		    else if (strcmp(Data_Received,"01010000") == 0)
		    	Serial.println(ADC_array[1]);
		    else if (strcmp(Data_Received,"01020000") == 0)
		    	Serial.println(ADC_array[2]);
		    else if (strcmp(Data_Received,"01030000") == 0)
		    	Serial.println(ADC_array[3]);
		    else if (strcmp(Data_Received,"01040000") == 0)
		    	Serial.println("0");
		    else if (strcmp(Data_Received,"01050000") == 0)
		    	Serial.println(ADC_array_min[0]);
		    else if (strcmp(Data_Received,"01060000") == 0)
		    	Serial.println(ADC_array_min[1]);
		    else if (strcmp(Data_Received,"01070000") == 0)
		    	Serial.println(ADC_array_min[2]);
		    else if (strcmp(Data_Received,"01080000") == 0)
		    	Serial.println(ADC_array_min[3]);
		    else if (strcmp(Data_Received,"01090000") == 0)
		    	Serial.println(ADC_array_max[0]);
		    else if (strcmp(Data_Received,"010A0000") == 0)
		    	Serial.println(ADC_array_max[1]);
		    else if (strcmp(Data_Received,"010B0000") == 0)
		    	Serial.println(ADC_array_max[2]);
		    else if (strcmp(Data_Received,"010C0000") == 0)
		    	Serial.println(ADC_array_max[3]);
		    else if (strcmp(Data_Received,"11FF0000") == 0){
		    	ADC_array_min[0]=DBL_MAX;
		    	ADC_array_min[1]=DBL_MAX;
		    	ADC_array_min[2]=DBL_MAX;
		    	ADC_array_min[3]=DBL_MAX;
				ADC_array_max[0]=-DBL_MAX;
				ADC_array_max[1]=-DBL_MAX;
				ADC_array_max[2]=-DBL_MAX;
				ADC_array_max[3]=-DBL_MAX;
		    	Serial.println("0");
		    }
		}    
	    else if (addr[0] == '3')//0x300 SIPO (Output sector).
	    {
	    	if (cmd[0] == '0')//rd output
	    	{
	    		Serial.println(SIPO_REG, BIN);
	    	}
	    	else if (cmd[0] == '1') //wr output
	    	{ //wr
	    		SIPO_REG = SIPO_write(strtol(data, NULL, 16));
	    		Serial.println(SIPO_REG, BIN);
	    	}
	    }	
	    else if (strcmp(Data_Received,"04000000") == 0)//0x400 PISO (Input sector).
	        Serial.println(PISO_REG, BIN);
	    else if (addr[0] == '5')//0x500 PMBus sector.
	    {
			if (strcmp(Data_Received,"15000003") == 0)
			{	
	    		sendBytePMBus(0x60, 0x03);//Clear TPS Fault
	    		Serial.println(1);
	    	}
			if (strcmp(Data_Received,"05000088") == 0)
			{	
	    		Serial.println(PM_READ_VIN_REG, HEX);
	    	}	
	    	if (strcmp(Data_Received,"05000089") == 0)
			{	
	    		Serial.println(PM_READ_IIN_REG, HEX);
	    	}	
	    	if (strcmp(Data_Received,"05000097") == 0)
			{	
	    		Serial.println(PM_READ_PIN_REG, HEX);
	    	}	
	    	if (strcmp(Data_Received,"05000079") == 0)
			{	
	    		Serial.println(PM_READ_STATUS_REG[0], BIN);
	    	}	
	    	if (strcmp(Data_Received,"050000D4") == 0)
			{	
	    		Serial.println(PM_READ_VOUT_REG[0], HEX);
	    	}	
	    	if (strcmp(Data_Received,"0500008C") == 0)
			{	
	    		Serial.println(PM_READ_IOUT_REG[0], HEX);
	    	}	
	    	if (strcmp(Data_Received,"05000096") == 0)
			{	
	    		Serial.println(PM_READ_POUT_REG[0], HEX);
	    	}	
	    	if (strcmp(Data_Received,"0500008D") == 0)
			{	
	    		Serial.println(PM_READ_TOUT_REG[0], HEX);
	    	}
	    }		
	    else if (addr[0] == '6')//0x600 PMBus sector.
	    {
			if (strcmp(Data_Received,"16000003") == 0)
			{	
	    		sendBytePMBus(0x60, 0x03);//Clear TPS Fault
	    		Serial.println(1);
	    	}
			if (strcmp(Data_Received,"06000088") == 0)
			{	
	    		Serial.println(PM_READ_VIN_REG, HEX);
	    	}	
	    	if (strcmp(Data_Received,"06000089") == 0)
			{	
	    		Serial.println(PM_READ_IIN_REG, HEX);
	    	}	
	    	if (strcmp(Data_Received,"06000097") == 0)
			{	
	    		Serial.println(PM_READ_PIN_REG, HEX);
	    	}	
	    	if (strcmp(Data_Received,"06000079") == 0)
			{	
	    		Serial.println(PM_READ_STATUS_REG[1], BIN);
	    	}	
	    	if (strcmp(Data_Received,"060000D4") == 0)
			{	
	    		Serial.println(PM_READ_VOUT_REG[1], HEX);
	    	}	
	    	if (strcmp(Data_Received,"0600008C") == 0)
			{	
	    		Serial.println(PM_READ_IOUT_REG[1], HEX);
	    	}	
	    	if (strcmp(Data_Received,"06000096") == 0)
			{	
	    		Serial.println(PM_READ_POUT_REG[1], HEX);
	    	}	
	    	if (strcmp(Data_Received,"0600008D") == 0)
			{	
	    		Serial.println(PM_READ_TOUT_REG[1], HEX);
	    	}	
	    }
	    else if (addr[0] == 'A')//0xA00 IC2 SYSMON sector FPGA1.
	    {
			if (strcmp(Data_Received,"0AF00000") == 0)
			{	
	    		Serial.println(I2C_SYSMON_FLAG_SLR0, BIN); //I2C Sysmon Fault
	    	}
	    	if (strcmp(Data_Received,"0AF10000") == 0)
			{	
	    		Serial.println(I2C_SYSMON_FLAG_SLR1, BIN); //I2C Sysmon Fault
	    	}
	    	if (strcmp(Data_Received,"0AF20000") == 0)
			{	
	    		Serial.println(I2C_SYSMON_FLAG_SLR2, BIN); //I2C Sysmon Fault
	    	}
			if (strcmp(Data_Received,"1AFE0000") == 0)
			{	
	    		update_I2C_PCA_CH_Select(I2C_PCA_7BITADDR, PCA_CH_SELECT);
	    		Word_I2C_Sysmon_Write( I2C_SYSMON_SLR0_7BITADDR, DRP_SYSMON_RESET_8BITADDR, 0x00);//Write 0x00 to 0x03 reset the Sysmon.
	    		Word_I2C_Sysmon_Write( I2C_SYSMON_SLR1_7BITADDR, DRP_SYSMON_RESET_8BITADDR, 0x00);//Write 0x00 to 0x03 reset the Sysmon.
	    		Word_I2C_Sysmon_Write( I2C_SYSMON_SLR2_7BITADDR, DRP_SYSMON_RESET_8BITADDR, 0x00);//Write 0x00 to 0x03 reset the Sysmon.
	    		Serial.println(1);
	    	}
			if (strcmp(Data_Received,"0AFF0000") == 0)
			{	
	    		Serial.println(I2C_SYSMON_STATUS, BIN); //I2C Sysmon Fault
	    	}
	    	if (strcmp(Data_Received,"1AFF0000") == 0)
			{	
	    		I2C_SYSMON_STATUS    = 0x0000;//Clear I2C Sysmon Fault
	    		I2C_SYSMON_FLAG_SLR0 = 0x0000;
				I2C_SYSMON_FLAG_SLR1 = 0x0000;//Clear FLag resiter
				I2C_SYSMON_FLAG_SLR2 = 0x0000;
	    		Serial.println(1);
	    	}
			if (strcmp(Data_Received,"0A000000") == 0)
			{	
	    		Serial.println(TEMP_SLR0, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000001") == 0)
			{	
	    		Serial.println(TEMP_SLR0_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000002") == 0)
			{	
	    		Serial.println(TEMP_SLR0_MAX, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000003") == 0)
			{	
	    		Serial.println(VCCINT_SLR0, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000004") == 0)
			{	
	    		Serial.println(VCCINT_SLR0_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000005") == 0)
			{	
	    		Serial.println(VCCINT_SLR0_MAX, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000006") == 0)
			{	
	    		Serial.println(VCCAUX_SLR0, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000007") == 0)
			{	
	    		Serial.println(VCCAUX_SLR0_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000008") == 0)
			{	
	    		Serial.println(VCCAUX_SLR0_MAX, HEX);
	    	}
	        if (strcmp(Data_Received,"0A000009") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR0, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A00000A") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR0_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A00000B") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR0_MAX, HEX);
	    	}	
	    	if (strcmp(Data_Received,"0A000010") == 0)
			{	
	    		Serial.println(TEMP_SLR1, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000011") == 0)
			{	
	    		Serial.println(TEMP_SLR1_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000012") == 0)
			{	
	    		Serial.println(TEMP_SLR1_MAX, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000013") == 0)
			{	
	    		Serial.println(VCCINT_SLR1, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000014") == 0)
			{	
	    		Serial.println(VCCINT_SLR1_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000015") == 0)
			{	
	    		Serial.println(VCCINT_SLR1_MAX, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000016") == 0)
			{	
	    		Serial.println(VCCAUX_SLR1, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000017") == 0)
			{	
	    		Serial.println(VCCAUX_SLR1_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000018") == 0)
			{	
	    		Serial.println(VCCAUX_SLR1_MAX, HEX);
	    	}
	        if (strcmp(Data_Received,"0A000019") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR1, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A00001A") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR1_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A00001B") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR1_MAX, HEX);
	    	}
	        if (strcmp(Data_Received,"0A000020") == 0)
			{	
	    		Serial.println(TEMP_SLR2, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000021") == 0)
			{	
	    		Serial.println(TEMP_SLR2_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000022") == 0)
			{	
	    		Serial.println(TEMP_SLR2_MAX, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000023") == 0)
			{	
	    		Serial.println(VCCINT_SLR2, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000024") == 0)
			{	
	    		Serial.println(VCCINT_SLR2_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000025") == 0)
			{	
	    		Serial.println(VCCINT_SLR2_MAX, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000026") == 0)
			{	
	    		Serial.println(VCCAUX_SLR2, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000027") == 0)
			{	
	    		Serial.println(VCCAUX_SLR2_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A000028") == 0)
			{	
	    		Serial.println(VCCAUX_SLR2_MAX, HEX);
	    	}
	        if (strcmp(Data_Received,"0A000029") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR2, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A00002A") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR2_MIN, HEX);
	    	}
	    	if (strcmp(Data_Received,"0A00002B") == 0)
			{	
	    		Serial.println(VCCBRAM_SLR2_MAX, HEX);
	    	}			
	
	    }
	    else if (addr[0] == 'E')//0xE00 Protection sector FPGA1.
	    {
			if (strcmp(addr,"E00") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
                      ptt_ADC_array_en_nReset_Shutdown[0] = dataUint16t; //(dataUint16t & 0b0000000000000011);
                      Serial.println(1);    		
	    		}
	    		else{
	    			Serial.println(ptt_ADC_array_en_nReset_Shutdown[0], BIN); }
	    	}
	    	if (strcmp(addr,"E01") == 0)
			{	
				if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_min[0]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{		
	    		Serial.println(ptt_ADC_array_min[0], BIN); }
	    	}
	    	if (strcmp(addr,"E02") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_max[0]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_max[0], BIN);} 
	    	}
	    	if (strcmp(addr,"E03") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
                      ptt_ADC_array_en_nReset_Shutdown[1] = dataUint16t; //(dataUint16t & 0b0000000000000011);
                      Serial.println(1);    		
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_en_nReset_Shutdown[1], BIN); }
	    	}
	    	if (strcmp(addr,"E04") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_min[1]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_min[1], BIN); }
	    	}
	    	if (strcmp(addr,"E05") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_max[1]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_max[1], BIN); }
	    	}
	    	if (strcmp(addr,"E06") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
                      ptt_ADC_array_en_nReset_Shutdown[2] = dataUint16t; //(dataUint16t & 0b0000000000000011);
                      Serial.println(1);    		
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_en_nReset_Shutdown[2], BIN); }
	    	}
	    	if (strcmp(addr,"E07") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_min[2]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_min[2], BIN); }
	    	}
	    	if (strcmp(addr,"E08") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_max[2]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_max[2], BIN); }
	    	}
	    	if (strcmp(addr,"E09") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
                      ptt_ADC_array_en_nReset_Shutdown[3] = dataUint16t; //(dataUint16t & 0b0000000000000011);
                      Serial.println(1);    		
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_en_nReset_Shutdown[3], BIN); }
	    	}
	    	if (strcmp(addr,"E0A") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_min[3]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_min[3], BIN); }
	    	}
	    	if (strcmp(addr,"E0B") == 0)
			{	
	    		if(cmd[0] == '1')
	    		{
	    			char *eptr;
	    			ptt_ADC_array_max[3]= strtod(data, &eptr);
	    			Serial.println(1);
	    		}
	    		else{
	    		Serial.println(ptt_ADC_array_max[3], BIN); }
	    	}
	    }	
	    else
	    	Serial.println( Data_Received);
    }//end of if

  



}





















int read_ADC(int pin_ID)
{
  return analogRead(pin_ID);
}


double display_ADC_Volt(double sensorValue, int pin_ID)
{
  double ADC_coeff=0;
  if(pin_ID==0)//1.8V. 
  {
    ADC_coeff=0.00322;    
  }
  if(pin_ID==1)//3.3V. 
  {
    ADC_coeff=0.0064453;    
  }
  if(pin_ID==2)//5V. 
  {
    ADC_coeff=0.0064453;    
  }
  if(pin_ID==3)//12V. 
  {
    ADC_coeff=0.018369;    
  }
  
  return(sensorValue*ADC_coeff);
}


// SIPO Byte shiftout function. to be called as many time as SIPO cheap are present.
void shiftOut(int mySIPO_dataOutPin, int mySIPO_clockPin, byte myDataOut) {
	// This shifts 8 bits out MSB first,
	//on the rising edge of the clock,
	int i=0;
	int pinState;
	pinMode(mySIPO_clockPin, OUTPUT);
	pinMode(mySIPO_dataOutPin, OUTPUT);
	//clear everything out just in case to
	//prepare shift register for bit shifting
	digitalWrite(mySIPO_dataOutPin, 0);
	digitalWrite(mySIPO_clockPin, 0);
	for (i=7; i>=0; i--)  {
			digitalWrite(mySIPO_clockPin, 0);
			if ( myDataOut & (1<<i) ) {
			pinState= 1;
			}
			else {
			pinState= 0;
			}
			//Sets the pin to HIGH or LOW depending on pinState
			digitalWrite(mySIPO_dataOutPin, pinState);
			//register shifts bits on upstroke of clock pin
			digitalWrite(mySIPO_clockPin, 1);
			//zero the data pin after shift to prevent bleed through
			digitalWrite(mySIPO_dataOutPin, 0);
	}
	//stop shifting
	digitalWrite(mySIPO_clockPin, 0);
}


////////////////////Function to run for having a full power supply sequnce (exept VCCint and VCCIO)///////////////////////
uint32_t SIPO_PowerStartupSequence()
{
	unsigned SIPO_dataStartupArray[10]= {0x0000};
	unsigned SIPO_State=0;
	//	Startup sequence.
	SIPO_dataStartupArray[9] = 0xAF20; //11100000
	SIPO_dataStartupArray[8] = 0xAF20; //11111111
	SIPO_dataStartupArray[7] = 0xAF20; //11111110
	SIPO_dataStartupArray[6] = 0xAF00; //11111100
	SIPO_dataStartupArray[5] = 0xAB00; //11111000
	SIPO_dataStartupArray[4] = 0xA300; //11110000
	SIPO_dataStartupArray[3] = 0xA100; //11100000
	SIPO_dataStartupArray[2] = 0xA000; //11000000
	SIPO_dataStartupArray[1] = 0x8000; //10000000
	SIPO_dataStartupArray[0] = 0x0000; //00000000*/
	
	for (int j = 0; j < 8; j++) 
	{
		//Temp byte data to be sent.
		byte SIPO_data;
		
		//load Highest Byte
		SIPO_data = ( (SIPO_dataStartupArray[j]>>8) & 0xff );
		//ground SIPO_latchPin and hold low for as long as you are transmitting
		digitalWrite(SIPO_latchPin, 0);
		//move 'em out
		shiftOut(SIPO_dataOutPin, SIPO_clockPin, SIPO_data);
		//load Lowest Byte
		SIPO_data = ( (SIPO_dataStartupArray[j]>>0) & 0xff );
		//move 'em out
		shiftOut(SIPO_dataOutPin, SIPO_clockPin, SIPO_data);
		//return the latch pin high to signal chip that it
		digitalWrite(SIPO_latchPin, 1);
		//Should be only for first two Byte, to init correctly the output as we don't know power up state of the SIPO.
		digitalWrite(SIPO_nEO, LOW);
		delay(500);

		SIPO_State = SIPO_dataStartupArray[j];
	}
	return (  (SIPO_State>>0) & 0x0000FFFF);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////Function to run to read PISO inputs///////////////////////
uint32_t PISO_Read()
{
  byte incoming;
  uint32_t PISO_State=0;
  // Write pulse to load pin
  digitalWrite(PISO_loadPin, LOW);
  delayMicroseconds(5);
  digitalWrite(PISO_loadPin, HIGH);
  delayMicroseconds(5);

  // Get data from 74HC165
  //digitalWrite(PISO_clockPin, HIGH);
  //digitalWrite(PISO_clockEnablePin, LOW);
  incoming = shiftIn165(PISO_dataInPin, PISO_clockPin, MSBFIRST);
  //digitalWrite(PISO_clockEnablePin, HIGH);
  // Print to serial monitor
  PISO_State= (PISO_State<<8) + (incoming);

  incoming = shiftIn165(PISO_dataInPin, PISO_clockPin, MSBFIRST);
  PISO_State= (PISO_State<<8) + (incoming);

  incoming = shiftIn165(PISO_dataInPin, PISO_clockPin, MSBFIRST);
  PISO_State= (PISO_State<<8) + (incoming);
  
  //delay(200);
  return PISO_State;
}

////////////////////Function to run for having a full power supply sequnce /////////////////////////////////////////////
uint32_t SIPO_write(unsigned DataWrite)
{
	unsigned SIPO_State=0;

	byte SIPO_data;	
	//load Highest Byte
	SIPO_data = ( (DataWrite>>8) & 0xff );
	//ground SIPO_latchPin and hold low for as long as you are transmitting
	digitalWrite(SIPO_latchPin, 0);
	//move 'em out
	shiftOut(SIPO_dataOutPin, SIPO_clockPin, SIPO_data);
	//load Lowest Byte
	SIPO_data = ( (DataWrite>>0) & 0xff );
	//move 'em out
	shiftOut(SIPO_dataOutPin, SIPO_clockPin, SIPO_data);
	//return the latch pin high to signal chip that it
	digitalWrite(SIPO_latchPin, 1);
	//Should be only for first two Byte, to init correctly the output as we don't know power up state of the SIPO.
	digitalWrite(SIPO_nEO, LOW);
	delay(500);

	SIPO_State = DataWrite;

	return (  (SIPO_State>>0) & 0x0000FFFF);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////Shift in fucntion for the PISO chip/////////////
uint8_t shiftIn165(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
  uint8_t value = 0;
  uint8_t i;

  for (i = 0; i < 8; ++i) {
    digitalWrite(clockPin, LOW);
    if (bitOrder == LSBFIRST) {
      value |= digitalRead(dataPin) << i;
    }
    else {
      value |= digitalRead(dataPin) << (7 - i);
    }
    digitalWrite(clockPin, HIGH);
  }
  return value;
}
//////////////////////////////////////////////////////////////////////////////



/////////////////////////////Read Word(16-Bits) PMBus function//////////////////////////////
uint16_t readWordPMBus(int address, int command)
{
	int WordSizeByte = 2;//number of byte to be returned by the TPS chip
	uint16_t readWord = 0;

	Wire.beginTransmission(address); // start transmission
	Wire.write(command);
	Wire.endTransmission(false); // don't send a stop condition
	int l = Wire.requestFrom(address, WordSizeByte); // request expected number of bytes to be returned
	
	if ( l = WordSizeByte)
	{
		for(int i = 0 ; i < WordSizeByte; i++)
		{
			readWord += (Wire.read() << (8*i));
			delay(1);
		}
	}	
	while(l>0 && Wire.available()) // Flush buffer if more than 2 Bytes returned.
	{
		delay(1);
		//__asm__("nop");
	}
	delay(1);
	return readWord;
}
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////Write Word(16-Bits) PMBus function//////////////////////////////
void writeBytePMBus(int address, byte command, byte data)
{
	int WordSizeByte = 2;//number of byte to be returned by the TPS chip
	uint16_t readWord = 0;

	Wire.beginTransmission(address);
	Wire.write(command);
	Wire.write(data);
	Wire.endTransmission();
}

/////////////////////////////Send byte(8-Bits)Send Byte PMBus function//////////////////////////////
void sendBytePMBus(int address, byte command)
{
	Wire.beginTransmission(address);
	Wire.write(command);
	Wire.endTransmission();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////





//Read the selected channel on the I2C multiplexer. Each Board have it's own Multiplexer. Each FPGA have 2 I2C idependant bus (I2C Ctrl(Ch.0 Not used, and Sysmon(Ch.1, used)).
bool  read_compare_I2C_PCA_CH(uint8_t I2C_PCA_7BitAddr, uint8_t ch_Select)
{
    //////////////////////////ReadBack PDA for checking//////////////////////////////
    i2c_start((I2C_PCA_7BitAddr<<1)|I2C_READ);
    uint8_t rdBck_Val = i2c_read(true); // read one byte and send NAK to terminate
    i2c_stop(); // send stop condition
    
    //Serial.print("PCA Channel: 0x");
    //Serial.println(rdBck_Val,HEX);
    if (rdBck_Val == ch_Select){
        return true;
    }
    else
        return false;
    /////////////////////////////////////////////////////////////////////////////////   
}



//Write the selected channel on the I2C multiplexer. Each Board have it's own Multiplexer. Each FPGA have 2 I2C idependant bus (I2C Ctrl(Ch.0 Not used, and Sysmon(Ch.1, used)).
bool  write_I2C_PCA_CH_Select(uint8_t I2C_PCA_7BitAddr, uint8_t ch_Select)
{
    ///////////////Write the PDA control register to enable Sysmon Channel///////////
    if (!i2c_start((I2C_PCA_7BitAddr<<1)|I2C_WRITE)) { // start read at slave address.
        i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x4000;
        //Serial.println("I2C device busy");
        return false;}
    
    else{
        if (!i2c_write(ch_Select)){ // send memory address
        	i2c_stop();
        	I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x4000;
			return false;}

        i2c_stop(); // send stop condition
        /////////////////////////////////////////////////////////////////////////////////

        //////////////////////////ReadBack PDA for checking//////////////////////////////
        if (!i2c_start((I2C_PCA_7BitAddr<<1)|I2C_READ)){
        	i2c_stop();
        	I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x4000;
			return false;}

        uint8_t rdBck_Val = i2c_read(true); // read one byte and send NAK to terminate
        i2c_stop(); // send stop condition
        
        //Serial.println(rdBck_Val);
        if (rdBck_Val == ch_Select){
            return true;
        }
        else
            return false;
        ///////////////////////////////////////////////////////////////////////////////// 
    }    
}


//Write the selected channel on the I2C multiplexer. Each Board have it's own Multiplexer. Each FPGA have 2 I2C idependant bus (I2C Ctrl(Ch.0 Not used, and Sysmon(Ch.1, used)).
bool  update_I2C_PCA_CH_Select(uint8_t I2C_PCA_7BitAddr, uint8_t ch_Select)
{
    //select channel on the I2C multiplexer for Sysmon (Ch. 1, 0010b, so 0x2). Checking is correct channel first.
    if(!read_compare_I2C_PCA_CH(I2C_PCA_7BitAddr, ch_Select))
    {    
        //Serial.println("I2C PCA CH Select incorrect. rewriting it");
        if(!write_I2C_PCA_CH_Select(I2C_PCA_7BitAddr, ch_Select))
        {
            //Serial.println("write I2C PCA CH Select failed. reseting I2C bus");
            digitalWrite(I2C_PCA_nRST_CTRL_PIN, LOW);    
            delay(100);
            digitalWrite(I2C_PCA_nRST_CTRL_PIN, HIGH);
            delay(100);
            if(!write_I2C_PCA_CH_Select(I2C_PCA_7BitAddr, ch_Select))
            {
                //Serial.println("cannot reset I2C bus");
                return false;
            }    
        }
    }
    return true;    
}    


uint16_t dBytes_I2C_Sysmon_Read( uint8_t I2C_Slave_Address, uint8_t Sysmon_Address)
{
    //Start with a "write" transaction at the I2C DRP Slave address.
    if (!i2c_start(I2C_Slave_Address<<1)|I2C_WRITE)
    {
        i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0002;
        //Serial.println("I2C device busy");
        return 0x0000;
    }   

    //Send I2C DRP Read Command. LSB to MSB.
    if (!i2c_write(0x00)){              //DRP LSB data. 0x00 as it's a read.
    	i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0002;
        return 0x0000;}
    if (!i2c_write(0x00)){              //DRP MSB data. 0x00 as it's a read.  
    	i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0002;
        return 0x0000;}   
    if (!i2c_write(Sysmon_Address)){ //[7:0], DRP address LSB
    	i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0002;
        return 0x0000;}
    if (!i2c_write(0x04)){              //[7:6]00b        [5:2] JTAG READ COMMAND ( 0001b)        [1:0], DRP Address MSbs (default is 00b).    
    	i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0002;
        return 0x0000;}                             

    // I2C restart command for trigerring a reading
    if (!i2c_rep_start((I2C_Slave_Address<<1)|I2C_READ)){ 
    	i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0002;
        return 0x0000;}  
    
    // I2C readback 2x Bytes
    uint8_t data0 = i2c_read(false);//NACK, as Still need a byte.
    uint8_t data1 = i2c_read(true); //ACK, as last Byte to get.

    // send stop condition to finish the I2C whole read transaction.
    i2c_stop(); 

    //uint16_t data = (data1<<8 | data0);
    //Serial.println(data, HEX);

    //return formatted 16-bit data
    return ( data1<<8 | data0);
    
}

bool Word_I2C_Sysmon_Write( uint8_t I2C_Slave_Address, uint8_t Sysmon_Address, uint16_t wr_data16b)
{
    //Start with a "write" transaction at the I2C DRP Slave address.
    if (!i2c_start(I2C_Slave_Address<<1)|I2C_WRITE)
    {
        i2c_stop();
        I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0001;
        //Serial.println("I2C device busy");
        return false;
    }  
	
	//Send I2C DRP Write Command. LSB to MSB
    uint8_t wr_data8b = (uint8_t)(wr_data16b & 0x00FF);
    if( !i2c_write(wr_data8b)){              //DRP LSB data. 0x00 as it's a read.
    	I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0001;
    	i2c_stop();
    	return false;}

    wr_data8b = (uint8_t)( (wr_data16b>>8) & 0x00FF);
    if( !i2c_write(wr_data8b)){              //DRP MSB data. 0x00 as it's a read. 
    	I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0001;
    	i2c_stop();
    	return false;}

    if( !i2c_write(Sysmon_Address)){ //[7:0], DRP address LSB
    	I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0001;
    	i2c_stop();
    	return false;}

    if( !i2c_write(0x08)){              //[7:6]00b        [5:2] JTAG WRITE COMMAND ( 0010b)        [1:0], DRP Address MSbs (default is 00b).                                 
    	I2C_SYSMON_STATUS = I2C_SYSMON_STATUS | 0x0001;
    	i2c_stop();
    	return false;}
    // send stop condition to finish the I2C whole read transaction.
    i2c_stop();

    //uint16_t data = (data1<<8 | data0);
    //Serial.println(data, HEX);

    //return formatted 16-bit data
    return true;
    
}