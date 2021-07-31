// BOF ========================================================================
// Orig: 2021.07.31 - Alex Israels
// Func: Manages the firing and reloading mechanisms of the laser tag gun.
// Meth: On startup the Shot Controller board requests the user and gun profile
//       from the main board. Once these have been initialized the board then
//       waits for an interupt either from the trigger or the reload button.
//       The trigger starts the initiateShot() ISR and the reload button starts
//       the reload() ISR.
// Defn: INITIALIZE            Initial value for all global variables
//       TEST_PLAYER_ID        playerId test value when testing in isolation
//       TEST_TEAM_ID          teamId test value when testing in isolation
//       TEST_DAMAGE           damage test value when testing in isolation
//       TEST_AMMO_COUNT       ammoCount test value when testing in isolation
//       TEST_MAX_AMMO         maxAmmo test value when testing in isolation
//       TEST_RELOAD_DELAY_MS  reloadDelay test value when testing in isolation
//       INIT_FIRE_PACKET_SIZE Inital size of firePacket
// Vars: playerId     = The player's id
//       teamId       = The id of the player's team
//       damage       = The damage dealt to another player when hit
//       ammoCount    = The current amount of ammo available
//       maxAmmo      = The maximum amount of ammo allowed for the current gun
//       reloadDelay  = The amount of time the gun delays from reloading
//       firingDelay  = The amount of time the gun delays from firing
//       firePacket   = Array of data sent in packet when trigger is pulled
//       isolatedTest = Enables test variables when not connected to main board 
//       consoleDebug = Enables console output for testing
//       initialTestValue = Variable to hold value of INITIALIZE for comparison
//-----------------------------------------------------------------------------

// Definitions:

//  - Initialization of Constant variables
#define FULL                 0x03
#define RELOAD_BUTTON_PIN       2

//  - Initialization of In Game variables
#define INIT_FIRE_PACKET_SIZE   3

//  - Testing
#define INITIALIZE             -1
#define TEST_PLAYER_ID         10
#define TEST_TEAM_ID           10
#define TEST_DAMAGE            10
#define TEST_AMMO_COUNT        10
#define TEST_MAX_AMMO          10
#define TEST_RELOAD_DELAY_MS 10000
#define TEST_FIRING_DELAY_MS 1000

//-----------------------------------------------------------------------------

// Constant Variables
const byte full = FULL;

const byte reloadButtonPin = RELOAD_BUTTON_PIN;

//-----------------------------------------------------------------------------

// In Game Variables
byte playerId = INITIALIZE;
byte teamId = INITIALIZE;
byte damage = INITIALIZE;
volatile byte ammoCount = INITIALIZE;
byte maxAmmo = INITIALIZE;

volatile bool isDelayed = false;

volatile short reloadInterruptTime = INITIALIZE;

short reloadDelay = INITIALIZE;
short firingDelay = INITIALIZE;

byte firePacket[] = {playerId, teamId, damage};
byte firePacketSize = INIT_FIRE_PACKET_SIZE;

//-----------------------------------------------------------------------------

// Test Variables
bool isolatedTest = true;
bool consoleDebug = true;
byte initialTestValue = INITIALIZE;

//-----------------------------------------------------------------------------

void setup() 
{
  if (consoleDebug) Serial.begin(9600);
  initializeShotController();
  initializeReload();
}

void loop() 
{
  if (millis() - reloadInterruptTime > reloadDelay)
  {
    isDelayed = false; 
  }
}

void initializeShotController() 
{
  receivePlayerData();
  receiveGunProfile();
  buildFirePacket();
  notifyShotControllerState();
}

void receivePlayerData() 
{
  if (isolatedTest) 
  {
    playerId = TEST_PLAYER_ID;
    teamId = TEST_TEAM_ID;
  } 
  else 
  {
    if (consoleDebug) Serial.println("Requesting player data from main board.");
  }
}

void receiveGunProfile() 
{
  if (isolatedTest) 
  {
    damage = TEST_DAMAGE;
    ammoCount = TEST_AMMO_COUNT;
    maxAmmo = TEST_MAX_AMMO;
    reloadDelay = TEST_RELOAD_DELAY_MS;
    firingDelay = TEST_FIRING_DELAY_MS;
  } 
  else 
  {
    if (consoleDebug) Serial.println("Requesting gun profile from main board.");
  }
}

void buildFirePacket() 
{
  firePacket[0] = playerId;
  firePacket[1] = teamId;
  firePacket[2] = damage;
  if (consoleDebug) Serial.println("Fire packet built.");
}

void notifyShotControllerState() 
{
  if (wasInitializationSuccessful()) 
  {
    if (consoleDebug) Serial.println("Shot Controller Initialization: SUCCESSFUL.");
  } 
  else 
  {
    if (consoleDebug) Serial.println("Shot Controller Initialization: FAILED.");
  }
}
 
bool wasInitializationSuccessful() 
{
  bool wasSuccessful = true;
  
  if      (playerId    == initialTestValue) { wasSuccessful = false; }
  else if (teamId      == initialTestValue) { wasSuccessful = false; }
  else if (damage      == initialTestValue) { wasSuccessful = false; }
  else if (ammoCount   == initialTestValue) { wasSuccessful = false; }
  else if (maxAmmo     == initialTestValue) { wasSuccessful = false; }
  else if (reloadDelay == initialTestValue) { wasSuccessful = false; }
  else if (firingDelay == initialTestValue) { wasSuccessful = false; }
  
  for (byte i = 0; i < firePacketSize; i++) 
  {
    if (firePacket[i]  == initialTestValue) { wasSuccessful = false; }
  }

  return wasSuccessful;
}

void initializeReload()
{
  pinMode(reloadButtonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(reloadButtonPin), reload, FALLING);
  if (consoleDebug) Serial.println("Reload initialized");
}

void reload() 
{
  if (isDelayed == false)
  {
    ammoCount = maxAmmo;
    notifyFullMagazine();
    delayForReload();
  }
  else
  {
    if (consoleDebug) Serial.println("Currently Delayed");
  }
}

void notifyFullMagazine()
{
  playSound(full);
  if (consoleDebug) Serial.println("Magazine is full");
}

void delayForReload()
{
  isDelayed = true;
  reloadInterruptTime = millis();
}

void playSound(byte soundCode)
{
  if (consoleDebug)
  {
    Serial.print("Sending sound code: ");
    Serial.println(soundCode);
  }
}

// EOF ========================================================================
