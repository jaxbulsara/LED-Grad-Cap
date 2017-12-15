#include <SmartMatrix3.h>

/* ----------------- */
/* Smartmatrix stuff */
/* ----------------- */
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = 100 * (255 / 100);    // 100 - full brightness

const int defaultScrollOffset = 6;
const rgb24 defaultBackgroundColor = {0, 0, 0}; // black

// counting variables
int i, j;

// Teensy 3.x has the LED on pin 13
const int ledPin = 13;

// current time variable
int currentMillis;

// serial message variables
String message [2][10];
int commandQueue [] = {0, 0};

String textOptions [100];

// initialize functions
void serialParse();
bool checkMode(String command, String mode);
void scrollText(String text);
void thankYouText();
void playGif(int index);

// setup function - runs once
void setup() {
	// initialize led on Teensy
	pinMode(ledPin, OUTPUT);

	// begin serial channels
	// pc - Serial
	// bluetooth - Serial1
	Serial.begin(9600);
	Serial1.begin(9600);

	// setup matrix
	matrix.addLayer(&backgroundLayer);
    matrix.addLayer(&scrollingLayer);
    matrix.addLayer(&indexedLayer);
    matrix.begin();

    matrix.setBrightness(defaultBrightness);

    scrollingLayer.setOffsetFromTop(defaultScrollOffset);

    backgroundLayer.enableColorCorrection(true);

	// clear screen
    backgroundLayer.fillScreen(defaultBackgroundColor);
    backgroundLayer.swapBuffers();
}

// loop function - runs continuously
void loop() {
	currentMillis = millis();

	// check if bluetooth serial is sending anything
	// "+" indicates a new command
	if(Serial1.available() or Serial.available()) {
		digitalWrite(ledPin, HIGH);
		while (Serial.available()) {
			char temp = char(Serial.read());

			// check if new character is a "+" and first character is '+'
			if (temp != '+' and char(message[0][commandQueue[0]].charAt(0)) != '+') {
				// do nothing
				;
			} else if (temp == '+' and char(message[0][commandQueue[0]].charAt(0)) == '+') {
				// increment command queue to move to next command
				commandQueue[0]++;
				message[0][commandQueue[0]] += temp;
			} else {
				message[0][commandQueue[0]] += temp;
			}
		}

		while (Serial1.available()) {
			char temp = char(Serial1.read());

			// check if new character is a "+" and first character is '+'
			if (temp != '+' and char(message[1][commandQueue[1]].charAt(0)) != '+') {
				// do nothing
				;
			} else if (temp == '+' and char(message[1][commandQueue[1]].charAt(0)) == '+') {
				// increment command queue to move to next command
				commandQueue[1]++;
				message[1][commandQueue[1]] += temp;
			} else {
				message[1][commandQueue[1]] += temp;
			}

			// wait 2ms for available() to catch up to bluetooth serial pin
			currentMillis = millis();
			while(millis() - currentMillis < 2) {;}
		}
		digitalWrite(ledPin, LOW);
	} else {
		// parse serial messages and then clear message variable
		serialParse();
	}


}

void serialParse() {
	for (i = 0; i < 2; i++) {
		for (j = 0; j <= commandQueue[i]; j++) {
			if (message[i][j] != "") {
				// set to lowercase
				message[i][j].toLowerCase();

				// check command type
				if (checkMode(message[i][j], "scrolltext")) {

				}

				message[i][j] = "";
			}
		}

		// reset commandQueue
		commandQueue[i] = 0;
	}
}

bool checkMode(String command, String mode) {
	if (command.indexOf(mode) > 0) {
		return 1;
	} else {return 0;}
}
