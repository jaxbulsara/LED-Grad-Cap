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

// counting variable
int i;

// Teensy 3.x has the LED on pin 13
const int ledPin = 13;

// current time variable
int currentMillis;

// serial message variables
String message [] = {"", ""};

// initialize functions
void log_pc(), log_bt(), serial_parse();

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
	// convert message to lowercase for parsing
	if(Serial1.available() or Serial.available()) {
		while (Serial.available()) {
			message[0] += char(Serial.read());
		}

		while (Serial1.available()) {
			message[1] += char(Serial1.read());
		}
	}

	// parse serial messages and then clear message variable
	serial_parse();
}

void log_pc() {
	Serial.println(message[0]);
}

void log_bt() {
	Serial1.println(message[1]);
}

void serial_parse() {
	for (i = 0; i < 1; i++) {
		if (message[i] != "") {
			// set to lowercase
			message[i].toLowerCase();

			Serial.println(message[i]);

			message[i] = "";
		}
	}
}
