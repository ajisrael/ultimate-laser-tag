// BOF ========================================================================
// Orig: 2021.08.07 - Alex Israels
// Func: Test board for reading IR packets from Shot Controller.
// Meth: Waits for a data packet from Shot Controller board and stores it in an
//       array <rawPacket>. Parses that data and stores it in another array
//       <firePacket>. Then verifies if the data was recieved correctly by
//       comparing firePacket against testFirePacket and setting the indicator
//       LED as appropriate.
// Note: This code assumes that the Shot Controller has isolatedTest = true, as
//       it uses the same TEST_<variable name> definitions for the comparison
//       between firePacket and testFirePacket.
// Defn: INITIALIZE              Initial value for global variables
//       TEST_PLAYER_ID          Value of playerId that should be received
//       TEST_TEAM_ID            Value of teamId that should be received
//       TEST_DAMAGE             Value of damage that should be received
//       INIT_FIRE_PACKET_LENGTH Length of fire packets
// Vars: receivingData = Bool for when board is receiving data
//       parsingData   = Bool for when board is parsing data
//       verifyingData = Bool for when board is printing data
//-----------------------------------------------------------------------------


#define INITIALIZE             -1
#define TEST_PLAYER_ID         10
#define TEST_TEAM_ID         0x01
#define TEST_DAMAGE            10
#define INIT_FIRE_PACKET_LENGTH 3

bool receivingData = false;
bool parsingData = false;
bool verifyingData = false;

byte rawPacket[4] = {INITIALIZE, INITIALIZE, INITIALIZE, INITIALIZE};

byte firePacket[] = {INITIALIZE, INITIALIZE, INITIALIZE};
byte firePacketLength = INIT_FIRE_PACKET_LENGTH;

byte testFirePacket[] = {TEST_PLAYER_ID, TEST_TEAM_ID, TEST_DAMAGE};
byte testFirePacketLength = INIT_FIRE_PACKET_LENGTH;

void setup()
//-----------------------------------------------------------------------------
// Func:  
// Meth:  
//-----------------------------------------------------------------------------
{
}

void loop()
//-----------------------------------------------------------------------------
// Func:  
// Meth:  
//-----------------------------------------------------------------------------
{
  while (receivingData) { receiveData(); }
  while (parsingData)   { parseData();   }
  while (verifyingData) { verifyData();  }
}

void receiveData(){}
void parseData(){}
void verifyData(){}

// EOF ========================================================================
