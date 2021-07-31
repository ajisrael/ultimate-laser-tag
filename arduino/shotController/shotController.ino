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

// Definitions
#define INITIALIZE             -1
#define TEST_PLAYER_ID         10
#define TEST_TEAM_ID           10
#define TEST_DAMAGE            10
#define TEST_AMMO_COUNT        10
#define TEST_MAX_AMMO          10
#define TEST_RELOAD_DELAY_MS 1000
#define TEST_FIRING_DELAY_MS 1000
#define INIT_FIRE_PACKET_SIZE   3

//-----------------------------------------------------------------------------

// Real Variables
byte playerId = INITIALIZE;
byte teamId = INITIALIZE;
byte damage = INITIALIZE;
byte ammoCount = INITIALIZE;
byte maxAmmo = INITIALIZE;

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
}
void loop() 
{
}
// EOF ========================================================================
