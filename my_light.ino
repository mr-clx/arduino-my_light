#define PIN_EXT_LED1 5
#define PIN_EXT_LED2 6

#define PIN_LIGHT A0
#define PIN_PIR 2


int power_state=0; // current external leds state
int power_state_pwr=0; // current PWR level for powered on pins (changed in function)
int power_pwr_max = 230; // max pwr level

const int power_on_pwr_step = 5;  // PWR step size for power_on
const int power_off_pwr_step = 1; // PWR step size for power_off

int state_light=0;
uint32_t tick_last_light=0;
const int light_treshold_on = 30; // when it's night?

int state_pir=0;
uint32_t tick_last_pir=0;

const int turn_on_for_sec = 60; // for which time turn on power?


void setPowerPwr(uint8_t level) {
	analogWrite(PIN_EXT_LED1, level);
	analogWrite(PIN_EXT_LED2, level);
}


void setup()
{
	/* add setup code here */
	Serial.begin(9600);

	Serial.println("MyLight simple controller v0.1 alpha build 20150802-3 by Odinchenko Aleksey (Aleksey.clx@gmail.com)");
	Serial.print("PIR Sensor pin: "); Serial.println(PIN_PIR);
	Serial.print("Light Sensor analog pin: "); Serial.println(PIN_LIGHT);
	Serial.print("Power pin1:\t"); Serial.println(PIN_EXT_LED1);
	Serial.print("Power pin2:\t"); Serial.println(PIN_EXT_LED2);
	Serial.print("Turn off timeout[sec]: "); Serial.println(turn_on_for_sec);
	
	//pin setup
	pinMode(PIN_PIR, INPUT_PULLUP);
	pinMode(PIN_LIGHT, INPUT_PULLUP);

	pinMode(PIN_EXT_LED1, OUTPUT);
	pinMode(PIN_EXT_LED2, OUTPUT);

	
	Serial.println("Ready");
}

uint32_t tick;
uint32_t tick_last_turned_on=0;
uint32_t tick_last_turned_off=0;

void loop()
{
	tick = millis();
	
	state_pir = digitalRead(PIN_PIR);
	state_light = analogRead(PIN_LIGHT>>2);

	// leds are turned off 
	// : check for light sensor and PIR
	if (!power_state) {
		if ((!power_state_pwr || state_light>light_treshold_on)
		&& state_pir) {
			tick_last_pir = tick;
			tick_last_light = tick;
			tick_last_turned_on=tick;
			power_state = 1;
			Serial.println("Light + PIR => Turning on");
		}
	} else {
		// turned on. check PIR sensor for movement
		if (state_pir) {
			// while moving detected - no change power!
			if (abs(tick-tick_last_pir)>5000) Serial.println("PIR raised -> no change state");
			tick_last_pir = tick;
			// delay(500);
		} else {
			// time to power off ?
			if (abs(tick-max(tick_last_turned_on, tick_last_pir))>=turn_on_for_sec*1000UL){
				power_state=0;
				tick_last_turned_off = tick;
				Serial.println("No PIR during timeout => Turning off");
			}
		}
	}

	// set power level
	int pwrTarget = power_state*power_pwr_max;
	if (power_state_pwr!=pwrTarget) {
		if (power_state_pwr<pwrTarget) {
			power_state_pwr+=power_on_pwr_step;
			if (power_state_pwr>pwrTarget) power_state_pwr = pwrTarget;
		}
		else if (power_state_pwr>pwrTarget) {
			power_state_pwr-=power_off_pwr_step;
			if (power_state_pwr<0) power_state_pwr = 0;
		}
		setPowerPwr(power_state_pwr);
	}
		
	
	// put power state to leds
	delay(25);
}
