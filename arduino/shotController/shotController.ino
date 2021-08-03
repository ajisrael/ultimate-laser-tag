// BOF ========================================================================
// Orig: 2021.07.31 - Alex Israels
// Func: Manages the firing and reloading mechanisms of the laser tag gun.
// Meth: On startup the Shot Controller board requests the user and gun profile
//       from the main board. Once these have been initialized the board then
//       waits for an interupt either from the trigger or the reload button.
//       The trigger starts the shoot() ISR and the reload button starts
//       the reload() ISR.
// Defn: 
//    Const:
//       EMPTY                 Sound code for indicating empty magazine
//       SHOT                  Sound code for indicating shot taken
//       FULL                  Sound code for indicating full ammo
//       RELOAD_BUTTON_PIN     Pin the reload button is connected to
//       RED_LED_PIN           Pin for red lead of RGB LED
//       GREEN_LED_PIN         Pin for green lead of RGB LED
//       BLUE_LED_PIN          Pin for blue lead of RGB LED
//       USE_RED_LED           Mask to determine if red LED should be used
//       USE_GREEN_LED         Mask to determine if green LED should be used
//       USE_BLUE_LED          Mask to determine if blue LED should be used
//       TEAM_PINS_LENGTH      Length of teamPins array
//    In Game:
//       INIT_FIRE_PACKET_LENGTH Inital length of firePacket
//    Test:
//       INITIALIZE            Initial value for all global variables
//       TEST_PLAYER_ID        playerId test value when testing in isolation
//       TEST_TEAM_ID          teamId test value when testing in isolation
//       TEST_DAMAGE           damage test value when testing in isolation
//       TEST_AMMO_COUNT       ammoCount test value when testing in isolation
//       TEST_MAX_AMMO         maxAmmo test value when testing in isolation
//       TEST_RELOAD_DELAY_MS  reloadDelay test value when testing in isolation
//       TEST_FIRING_DELAY_MS  firingDelay test value when testing in isolation
//       TEST_MUZZLE_DELAY_MS  muzzleDelay test value when testing in isolation
// Vars: 
//    Const:
//       empty           = Sound code for empty magazine
//       shot            = Sound code for shot taken
//       full            = Sound code for full ammo
//       reloadButtonPin = Pin the reload button is connected to
//       redLEDPin       = Pin red LED is connected to
//       greenLEDPin     = Pin green LED is connected to
//       blueLEDPin      = Pin blue LED is connected to
//       teamPinsLength  = Length of teamPins array
//    In Game:
//       playerId       = The player's id
//       teamId         = The id of the player's team 
//       teamPins       = The active pins for team color of muzzle flash
//       damage         = The damage dealt to another player when hit
//       ammoCount      = The current amount of ammo available
//       maxAmmo        = The maximum amount of ammo allowed for the current gun
//       reloadDelay    = The amount of time the gun delays from reloading
//       firingDelay    = The amount of time the gun delays from firing
//       firePacket     = Array of data sent in packet when trigger is pulled
//       firePacketLength = Length of firePacket
//    Timing:
//       reloadInterruptTime = timestamp of last reload()
//       shotInterruptTime   = timestamp of last shot()
//       isDelayed           = Keeps track of delay state of gun
//    Test:
//       isolatedTest   = Enables test vars when not connected to main board 
//       consoleDebug   = Enables console output for testing
//       initialValue   = Variable to hold value of INITIALIZE for comparison
//-----------------------------------------------------------------------------

// Definitions:

//    Init Constant variables:
//        Sound Codes:
#define EMPTY                0x00
#define SHOT                 0x01
#define FULL                 0x03
//        Pins:
#define RELOAD_BUTTON_PIN       2
#define SHOOT_BUTTON_PIN        3
#define RED_LED_PIN             4
#define GREEN_LED_PIN           7
#define BLUE_LED_PIN            8
//        Masks:
#define USE_RED_LED          0x01 // 0000 0001
#define USE_GREEN_LED        0x02 // 0000 0010
#define USE_BLUE_LED         0x04 // 0000 0100
//        Other:
#define TEAM_PINS_LENGTH        3

//    Init In Game variables:
#define INIT_FIRE_PACKET_LENGTH 3

//    Init Testing variables:
#define INITIALIZE             -1
#define TEST_PLAYER_ID         10
#define TEST_TEAM_ID         0x01
#define TEST_DAMAGE            10
#define TEST_AMMO_COUNT        10
#define TEST_MAX_AMMO          10
#define TEST_RELOAD_DELAY_MS 3000
#define TEST_FIRING_DELAY_MS 3000
#define TEST_MUZZLE_DELAY_MS 3000

//-----------------------------------------------------------------------------

// Constant Variables:
//    Sound Codes:
const byte empty  = EMPTY;
const byte shot   = SHOT;
const byte full   = FULL;

//    Pins:
const byte reloadButtonPin = RELOAD_BUTTON_PIN;
const byte shootButtonPin  = SHOOT_BUTTON_PIN;
const byte redLEDPin   = RED_LED_PIN;
const byte greenLEDPin = GREEN_LED_PIN;
const byte blueLEDPin  = BLUE_LED_PIN;
const byte teamPinsLength = TEAM_PINS_LENGTH;

//-----------------------------------------------------------------------------

// In Game Variables:
//    Player Data:
byte playerId = INITIALIZE;
byte teamId = INITIALIZE;
byte teamPins[] = {INITIALIZE, INITIALIZE, INITIALIZE};

//    Gun Profile:
byte damage             = INITIALIZE;
byte maxAmmo            = INITIALIZE;
volatile byte ammoCount = INITIALIZE;
unsigned short reloadDelay = INITIALIZE;
unsigned short firingDelay = INITIALIZE;
unsigned short muzzleDelay = INITIALIZE;
byte firePacket[] = {playerId, teamId, damage};
byte firePacketLength = INIT_FIRE_PACKET_LENGTH;

//-----------------------------------------------------------------------------

// Timing Variables:
volatile unsigned long reloadInterruptTime = INITIALIZE;
volatile unsigned long shotInterruptTime   = INITIALIZE;
volatile unsigned long muzzleFlashTime     = INITIALIZE;
volatile bool isDelayed = false;

//-----------------------------------------------------------------------------

// Test Variables
bool isolatedTest = true;
bool consoleDebug = true;
byte initialValue = INITIALIZE;

//-----------------------------------------------------------------------------

void setup()
//-----------------------------------------------------------------------------
// Func:  Setup code for initialization of variables/state of the board
// Meth:  Initializes Shot Controller global variables with 
//        initializeShotController() method and notifies main board if 
//        successful or not. Followed by initialization of reload feature
//        through initializeReload() method and initialization of shoot feature
//        through initializeShoot() method.
//-----------------------------------------------------------------------------
{
  if (consoleDebug) Serial.begin(9600); // Initialize Serial Monitor if debug
  initializeShotController();           // Init global vars and notify state
  initializeReload();                   // Init reload feature
}

void loop() 
//-----------------------------------------------------------------------------
// Func:  Manages delay when ISR's aren't running
// Meth:  Checks the ammount of time that has passed since an interrupt has
//        been triggered. If that time is greater than the delay then the delay
//        flag (isDelayed) is turned off.
//-----------------------------------------------------------------------------
{
  if (millis() - reloadInterruptTime > reloadDelay) // Check if gun should be 
  {                                                 // delayed since last reload
    isDelayed = false;                              // Turns off delay flag
  }

  if (millis() - muzzleFlashTime > muzzleDelay)
  {
    turnOffMuzzleFlash();
  }
}

void initializeShotController()
//-----------------------------------------------------------------------------
// Func:  Initializes global variables for Shot Controller and notifies the
//        main board if initialization succeeded or failed.
// Meth:  Asks the main board for player data and sets corresponding variables.
//        Executes same process for gun profile data. Then builds the fire
//        packet once all variables have been set. Finally, checks if variables
//        have been correctly set and sends state back to main board.
//-----------------------------------------------------------------------------
{
  receivePlayerData();
  receiveGunProfile();
  buildFirePacket();
  notifyShotControllerState();
}

void receivePlayerData()
//-----------------------------------------------------------------------------
// Func:  Asks main board for player data and sets corresponding variables.
// Meth:  First checks if Shot Contoller board is connected to main board and
//        sets corresponding variables.
//-----------------------------------------------------------------------------
{
  if (isolatedTest)             // Check for testing in isolation
  {
    playerId = TEST_PLAYER_ID;  // Set global variables with test values
    teamId = TEST_TEAM_ID;    
  } 
  else                          // Otherwise call main board for data
  {
    if (consoleDebug) Serial.println("Requesting player data from main board.");
  }

  setTeamLEDPins(teamId);       // Set pins for muzzle flash based on team

  if (consoleDebug)
  {
    Serial.print("Team Id: ");
    Serial.print(teamId);
    Serial.print(" + R:");
    Serial.print(teamPins[0]);
    Serial.print(" G:");
    Serial.print(teamPins[1]);
    Serial.print(" B:");
    Serial.println(teamPins[2]);
  }  
}

void setTeamLEDPins(byte currentTeamId)
//-----------------------------------------------------------------------------
// Func:  Sets the pins to use for the muzzle flash based on the team color
// Meth:  Resets the teamPins array, then uses the USE_<color>_LED masks to 
//        determine the pins that should be enabled to flash the correct color
//        out the muzzle when a shot is taken.
//-----------------------------------------------------------------------------
{
  for (int i = 0; i < teamPinsLength; i++) // Reset each teamPin index
  { 
    teamPins[i] = INITIALIZE;              // back to its initial value
  }
  if (currentTeamId & USE_RED_LED)  { teamPins[0] = redLEDPin;   } // set red
  if (currentTeamId & USE_GREEN_LED){ teamPins[1] = greenLEDPin; } // set green
  if (currentTeamId & USE_BLUE_LED) { teamPins[2] = blueLEDPin;  } // set blue
}

void receiveGunProfile() 
//-----------------------------------------------------------------------------
// Func:  Asks main board for gun profile and sets corresponding variables.
// Meth:  First checks if Shot Contoller board is connected to main board and
//        sets corresponding variables.
//-----------------------------------------------------------------------------
{
  if (isolatedTest)                     // Check for testing in isolation
  {
    damage = TEST_DAMAGE;               // Set global variables with 
    ammoCount = TEST_AMMO_COUNT;        // test values
    maxAmmo = TEST_MAX_AMMO;
    reloadDelay = TEST_RELOAD_DELAY_MS;
    firingDelay = TEST_FIRING_DELAY_MS;
    muzzleDelay = TEST_MUZZLE_DELAY_MS;
  } 
  else                                  // Otherwise call main board for data
  {
    if (consoleDebug) Serial.println("Requesting gun profile from main board.");
  }
}

void buildFirePacket() 
//-----------------------------------------------------------------------------
// Func:  Builds the firePacket array with global variables.
// Meth:  Sets each index of the firePacket array with correct global variable.
//-----------------------------------------------------------------------------
{
  firePacket[0] = playerId;                              // Set player id,
  firePacket[1] = teamId;                                // team id, and damage
  firePacket[2] = damage;                                // for firePacket
  if (consoleDebug) Serial.println("Fire packet built.");
}

void notifyShotControllerState()
//-----------------------------------------------------------------------------
// Func:  Checks if initializtion was successful and notifies main board.
// Meth:  Calls wasInitializationSuccessful() to check if initializtion was 
//        successful and notifies main board.
//-----------------------------------------------------------------------------
{
  if (wasInitializationSuccessful())  // Check if initialization was successful
  {                                   // Notify main board of success
    if (consoleDebug) Serial.println("Shot Controller Initialization: SUCCESSFUL.");
  } 
  else 
  {                                   // Notify main board of failure
    if (consoleDebug) Serial.println("Shot Controller Initialization: FAILED.");
  }
}
 
bool wasInitializationSuccessful() 
//-----------------------------------------------------------------------------
// Func:  Checks if initializtion was successful.
// Meth:  Checks if any of the variables set by receivePlayerData() and 
//        receiveGunProfile() are still at their initial test value. If any one
//        of them are not set to a new value then returns false otherwise true.
// Retn:  wasSuccessful = boolean value indicating success of initialization.
//-----------------------------------------------------------------------------
{
  bool wasSuccessful = true;  // Will be set to false if any values aren't set
  
  // Check if any vars from receivePlayerData() + receiveGunProfile() are still
  // set to their intial value on boot
  if      (playerId    == initialValue) { wasSuccessful = false; }
  else if (teamId      == initialValue) { wasSuccessful = false; }
  else if (damage      == initialValue) { wasSuccessful = false; }
  else if (ammoCount   == initialValue) { wasSuccessful = false; }
  else if (maxAmmo     == initialValue) { wasSuccessful = false; }
  else if (reloadDelay == initialValue) { wasSuccessful = false; }
  else if (firingDelay == initialValue) { wasSuccessful = false; }
  
  // Check if fire packet was built properly
  for (byte i = 0; i < firePacketLength; i++) 
  {
    if (firePacket[i]  == initialValue) { wasSuccessful = false; }
  }

  return wasSuccessful;       // return state of initialization
}

void initializeReload()
//-----------------------------------------------------------------------------
// Func:  Sets pin mode for reload button and attaches the reload() ISR to the
//        reload button.
// Meth:  Sets the pin mode for the reload button as a PULLUP. With the PULLUP
//        set, the interupt is attached to only trigger on the falling edge 
//        when the reload button breaks the connection.
//-----------------------------------------------------------------------------
{
  pinMode(reloadButtonPin, INPUT_PULLUP);                // Set button pin mode

  attachInterrupt(                                       // Set reload() ISR to
    digitalPinToInterrupt(reloadButtonPin),              // exec on FALLING
    reload, FALLING);                                    // edge
  
  if (consoleDebug) Serial.println("Reload initialized");
}

void reload() 
//-----------------------------------------------------------------------------
// Func:  ISR: Reloads the gun and sets the delay flag to true to prevent quick
//        firing after reload.
// Meth:  First checks if gun is not delayed, then reloads the ammo. Followed
//        by notifying the main board that the magazine is now full and then
//        delaying the gun.
// Voli:  isDelayed           - reload() : set isDelayed to true
//        reloadInterruptTime - reload() : capture timestamp
//        ammoCount           - reload() : set ammoCount to maxAmmo
//-----------------------------------------------------------------------------
{
  if (isDelayed == false) // Make sure gun is not delayed
  {
    isDelayed = true;               // Set delay flag
    reloadInterruptTime = millis(); // Get ISR timestamp for loop()
    ammoCount = maxAmmo;            // Reload ammo
    playSound(full);                // Notify main board magazine is full
    if (consoleDebug) Serial.println("Magazine is full");
  }
}

void initializeShoot()
//-----------------------------------------------------------------------------
// Func:  Sets pin mode for shoot button and attaches the shoot() ISR to the
//        shoot button.
// Meth:  Sets the pin mode for the shoot button as a PULLUP. With the PULLUP
//        set, the interupt is attached to only trigger on the falling edge 
//        when the shoot button breaks the connection.
//-----------------------------------------------------------------------------
{
  pinMode(shootButtonPin, INPUT_PULLUP);                // Set button pin mode

  attachInterrupt(                                      // Set shoot() ISR to
    digitalPinToInterrupt(shootButtonPin),              // exec on FALLING
    shoot, FALLING);                                    // edge
  
  if (consoleDebug) Serial.println("Shoot initialized");
}

void shoot()
//-----------------------------------------------------------------------------
// Func:  ISR: Shoots a laser beam with encoded data when trigger is pressed.
// Meth:  First checks if gun is not delayed, then sets timing variables.
//        Proceeds to check for ammo. If there is none, then plays empty sound.
//        Otherwise, calls fireShot().
// Voli:  isDelayed           - shoot() : set isDelayed to true
//        shotInterruptTime   - shoot() : capture timestamp
//        ammoCount           - shoot() : decrement for shot      
//-----------------------------------------------------------------------------
{
  if (isDelayed == false)
  {
    isDelayed = true;
    shotInterruptTime = millis();
    if (ammoCount == 0)
    {
      playSound(empty);
      if (consoleDebug) Serial.println("Magazine is empty");
    }
    else
    {
      fireShot();
      if (consoleDebug) Serial.println("Shot fired");
    }
  }
}

void fireShot()
//-----------------------------------------------------------------------------
// Func:  Transmits IR data, immitates gun fire, flashes the muzzle light, and
//        decrements the ammoCount.
// Meth:  Calls other methods to execute the functionality.     
//-----------------------------------------------------------------------------
{
  xmitPacket();
  immitateGunFire();
  if (ammoCount > 0) { ammoCount--; }
}

void xmitPacket(){}

void immitateGunFire()
//-----------------------------------------------------------------------------
// Func:  Immitates gun fire by playing the shot sound and flashing the muzzle.
// Meth:  Tells the main board to play the shot sound and flashes the muzzle
//        by calling flashMuzzleLight() method.
//-----------------------------------------------------------------------------
{
  playSound(shot);
  flashMuzzleLight();
}

void flashMuzzleLight()
//-----------------------------------------------------------------------------
// Func:  Flashes the muzzle light with the color of the player's team.
// Meth:  Loops through teamPins array. Any values that are not -1 are active
//        and the LED for that pin is turned on. The time is then captured to
//        later turn off the muzzle flash inside the loop() function.
//-----------------------------------------------------------------------------
{
  for (int i = 0; i < teamPinsLength; i++) // For each LED in muzzle
  {
    if (teamPins[i] != initialValue)       // If it is active
    {
      digitalWrite(teamPins[i], HIGH);     // Turn it on
    }
  }
  muzzleFlashTime = millis();              // Capture time to turn off later
}

void turnOffMuzzleFlash()
//-----------------------------------------------------------------------------
// Func:  Turns off all active muzzle LEDs.
// Meth:  Loops through teamPins array. Any values that are not -1 are active
//        and the LED for that pin is turned off.
//-----------------------------------------------------------------------------
{
  for (int i = 0; i < teamPinsLength; i++) // For each LED in muzzle
  {
    if (teamPins[i] != initialValue)       // If it is active
    {
      digitalWrite(teamPins[i], LOW);      // Turn it off
    }
  }
}

void playSound(byte soundCode)
//-----------------------------------------------------------------------------
// Func:  Sends a sound code to the main board to be played.
// Meth:  Sends a sound code to the main board to be played.
//-----------------------------------------------------------------------------
{
  if (consoleDebug)
  {
    Serial.print("Sending sound code: ");
    Serial.println(soundCode);
  }
}

// EOF ========================================================================
