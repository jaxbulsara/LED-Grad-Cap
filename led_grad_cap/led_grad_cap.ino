#include <SmartMatrix3.h>
#include <SD.h>
#include "GIFDecoder.h"
#include "FilenameFunctions.h"

#define DISPLAY_TIME_SECONDS 10 // for gif
#define ENABLE_SCROLLING  1		// for sd error

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
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer2, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = 100 * (255 / 100);    // 100 - full brightness

const int defaultScrollOffset = 4;
const rgb24 defaultBackgroundColor = {0, 0, 0}; // black

// gif player
/* template parameters are maxGifWidth, maxGifHeight, lzwMaxBits
 *
 * The lzwMaxBits value of 12 supports all GIFs, but uses 16kB RAM
 * lzwMaxBits can be set to 10 or 11 for small displays, 12 for large displays
 * All 32x32-pixel GIFs tested work with 11, most work with 10
 */
GifDecoder<kMatrixWidth, kMatrixHeight, 12> decoder;

// Chip select for SD card on the SmartMatrix Shield
#define SD_CS BUILTIN_SDCARD

#define GIF_DIRECTORY "/gifs/"

int numGifs;

void screenClearCallback(void) {
  backgroundLayer.fillScreen({0,0,0});
}

void updateScreenCallback(void) {
  backgroundLayer.swapBuffers();
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  backgroundLayer.drawPixel(x, y, {red, green, blue});
}

// scrolling text content
String textOptions [100] = {
	"University of Central Florida",
	"Burnett Honors College",
	"College of Engineering and Computer Science",
	"Mechanical Engineering but I do this for fun.",
	"UCF Chess Club",
	"UCF SAE BAJA!!",
	"Built on Teensy 3.5",
	"HC-06 Bluetooth Module",
	"Smartmatrix3",
	"LED Matrix from Adafruit",
	"Hi mom!",
	"Hey person behind me",
	"DO IT, just DO IT! Dont let your dreams be dreams.",
	"Gr8 b8, m8. I rel8, str8 appreci8, and congratul8.",
	"Has anyone really been far even as decided to use even go want to do look more like?",
	"I sexually Identify as an Attack Helicopter.",
	"I can no longer resist the pizza.",
	"Whats a pupper? A small doggo. Whats a doggo? A big ol pupper.",
	"To be fair, you have to have a very high IQ to understand Rick and Morty.",
	"Seminole High School",
	"Sanford Middle School",
	"Goldsboro Elementary School"
};

// thanking text content
String thankOptions [100] = {
	"My parents, Tushar and Anila Bulsara.",
	"/r/me_irl",
	"My sister, Jenisha Bulsara",
	"My grandmother, Saraswatiben Mistry",
	"My friend, Kyle Mueller",
	"The real MVP, Amber Morgan",
	"All my friends and family",
	"Chinmaya Mission",
	"Richard Lamberty",
	"Chris Delacruz",
	"Wyatt Brooks",
	"Caroline Kamm",
	"Jay Scofield",
	"Jane Cooper",
	"UCF BAJA",
	"UCF Chess Club22",
	"Saurabh Sudesh",
	"Ishika Khondaker",
	"Aldwin DeGuzman",
	"Tahira Tasnim",
	"The internet (rip net neutrality)",
	"Alex Koohyar",
	"Evan Rapp",
	"Michell Zhong",
	"Susan Brennan",
	"Doug Hernandez",
	"Melody Sweigert",
	"Mindy Craft, for making sure I got through IB.",
	"Trung Vong",
	"Manhar Dalal",
	"Jennifer Vigilante",
	"Jasper Zaporteza",
	"Tyler Hudson",
	"Tyler Clark",
	"Nick Chau",
	"Sammy Wohl",
	"Arnold Banner",
	"Rafael Rosado",
	"Rob Tukdarian",
	"Dom Cinefro",
	"Sam Johnson",
	"Vicente Porcar",
	"Alex (Cheese) Voce",
	"Daniel Healy",
	"My senior design team: Therese Salas, Hayden Bonnen, Ryan Kraft. And our advisor: Dr. Shawn Putnam",
	"Kurt Stresau",
	"Natasha Jones",
	"Ryker Chute",
	"Kyle Fryman",
	"Jenna Famiglietti and her family",
	"Lynda.com",
	"Jeffrey Kauffman",
	"Don Harper for running the Innovation Lab, where I made this hat",
	"The UCF Library",
	"The study room in the honors college",
	"Parking lot B6 for always having parking",

};

// counting variables
int i, j, k;

// Teensy 3.x has the LED on pin 13
const int ledPin = 13;

// current time variable
unsigned long currentMillis;

// serial message variables
String message [2][10];
int commandQueue [] = {0, 0};

// random selection variables
int status = 0;
int modeToPlay;
int indexToPlay;

// randText - between 0 and the number of text options available
int numTextOptions = 0;
int randText;
int isScrolling = false;

// randGif - which gif to play
int randGif;
unsigned long gifEndMillis;
bool gifPlaying = false;

// int randThanks - which person to thank
int numThanks = 0;
int randthanks;
bool isThanking = false;

// initialize functions
void serialParse();
bool checkCommand(String command, String mode);
void randomSelector();
int scrollText(int index);	// index 0
int playGif(int index);	// index 1
int thankYouText(int index);		// index 2

// function indexing modes
int playMode(int mode, int index) {
	typedef int (*f) (int);

	static f funcs[] = {scrollText, playGif, thankYouText};

	return funcs[mode](index);
}

bool modeToggles [] = {1, 1, 1, 0};
int numModes;
int numOptionsInMode [4];
int queue [10];

// setup function - runs once
void setup() {
	// initialize led on Teensy
	pinMode(ledPin, OUTPUT);

	// GIF setup
	decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);

	// begin serial channels
	// pc - Serial
	// bluetooth - Serial1
	Serial.begin(9600);
	Serial1.begin(9600);

	// setup matrix
	matrix.addLayer(&backgroundLayer);
    matrix.addLayer(&scrollingLayer);
	matrix.addLayer(&scrollingLayer2);
    matrix.addLayer(&indexedLayer);
    matrix.begin();

    matrix.setBrightness(defaultBrightness);

    scrollingLayer.setOffsetFromTop(defaultScrollOffset);

    backgroundLayer.enableColorCorrection(true);

	backgroundLayer.fillScreen(defaultBackgroundColor);
	backgroundLayer.swapBuffers();

	// initialize the SD card at full speed
    pinMode(SD_CS, OUTPUT);
    if (!SD.begin(SD_CS)) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("No SD card", -1);
#endif
        Serial.println("No SD card");
        while(1);
    }

	// Determine how many animated GIF files exist
    numOptionsInMode[1] = enumerateGIFFiles(GIF_DIRECTORY, false);

    if(numOptionsInMode[1] < 0) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("No gifs directory", -1);
#endif
        Serial.println("No gifs directory");
        while(1);
    }

    if(!numOptionsInMode[1]) {
#if ENABLE_SCROLLING == 1
        scrollingLayer.start("Empty gifs directory", -1);
#endif
        Serial.println("Empty gifs directory");
        while(1);
    }

	// random selection setup
	// random seed
	randomSeed(analogRead(33));

	// count number of non empty text contents
	i = 0;
	while (textOptions[i++] != "") {numOptionsInMode[0]++;}

	i = 0;
	while (thankOptions[i++] != "") {numOptionsInMode[2]++;}

	// count number of modes
	numModes = sizeof(modeToggles) / sizeof(bool);
}

// loop function - runs continuously
void loop() {
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

	randomSelector();
}

void serialParse() {
	/*
	available commands:
		+scroll:{0, 1} - toggle scrollText
		+gif:{0, 1} - toggle playGif
		+thank:{0, 1} - toggle thank
		+addText:{text to add} - adds text to textOptions
		+addThank:{thank to add} - adds text to thankOptions
		+queue:{scroll, gif, thank},{index} - queues the following to be played next
		+repeat:{scroll, gif, thank},{index} - plays the following continuously
		+status - returns which mode and index is playing
		+clear - clears display until +begin is sent
		+begin - starts randomSelector
	*/

	int colon, comma;
	for (i = 0; i < 2; i++) {
		for (j = 0; j <= commandQueue[i]; j++) {
			if (message[i][j] != "") {
				// set to lowercase
				message[i][j].toLowerCase();

				// check command type
				if (checkCommand(message[i][j], "scroll")) {
					colon = message[i][j].indexOf(':');

				}

				message[i][j] = "";
			}
		}

		// reset commandQueue
		commandQueue[i] = 0;
	}
}

bool checkCommand(String command, String mode) {
	if (command.indexOf(mode) > 0) {
		return 1;
	} else {return 0;}
}

void randomSelector() {
	if (!status) {
		// generate a random number and check if that index is on
		do {
			modeToPlay = random(numModes);
		} while (!modeToggles[modeToPlay]);

		// now n is the mode to be picked
		// depending on the mode, choose a different index to generate
		indexToPlay = random(numOptionsInMode[modeToPlay]);
	}

	// play the mode and index generated
	status = playMode(modeToPlay, indexToPlay);
}

int scrollText(int index) {
	if (!isScrolling) {
		// if not scrolling, allow setup of scrolling text
		// clear background
		backgroundLayer.fillScreen(defaultBackgroundColor);
		backgroundLayer.swapBuffers();

		scrollingLayer.setColor({0xff, 0xff, 0xff});
		scrollingLayer.setFont(font8x13);
		scrollingLayer.setOffsetFromTop(defaultScrollOffset);
		scrollingLayer.setSpeed(40);

		scrollingLayer.setMode(wrapForward);
		int stringLength = textOptions[index].length() + 1;
		char temp [stringLength];
		textOptions[index].toCharArray(temp, stringLength);
		scrollingLayer.start(temp, 1);

		isScrolling = true;
	} else if (!scrollingLayer.getStatus()) {
		// if scrolling flag is true and getStatus is false, then scrolling has just stopped - reset isScrolling
		isScrolling = false;

		// clear background
		backgroundLayer.fillScreen(defaultBackgroundColor);
		backgroundLayer.swapBuffers();
	}

	return isScrolling;
}

int thankYouText(int index) {
	if (!isThanking) {
		// clear background
		backgroundLayer.fillScreen(defaultBackgroundColor);
		backgroundLayer.swapBuffers();

		scrollingLayer.setColor({0xff, 0xff, 0xff});
		scrollingLayer.setFont(font3x5);
		scrollingLayer.setOffsetFromTop(1);
		scrollingLayer.setMode(stopped);
		scrollingLayer.start("Thanks!", 1);

		scrollingLayer2.setColor({0xff, 0xff, 0xff});
		scrollingLayer2.setFont(font6x10);
		scrollingLayer2.setMode(wrapForward);
		scrollingLayer2.setSpeed(40);
		scrollingLayer2.setOffsetFromTop(matrix.getScreenHeight() / 2);

		int stringLength = thankOptions[index].length() + 1;
		char temp [stringLength];
		thankOptions[index].toCharArray(temp, stringLength);
		scrollingLayer2.start(temp, 1);

		isThanking = true;
	} else if (!scrollingLayer2.getStatus()) {
		// if scrolling flag is true and getStatus is false, then scrolling has just stopped - reset isThanking
		isThanking = false;

		// hack to make sure thanks goes away
		scrollingLayer.start("", 1);
	}

	return isThanking;
}

int playGif(int index) {
	// ucf - 41
	if (!gifPlaying){
		// clear background
		backgroundLayer.fillScreen(defaultBackgroundColor);
		backgroundLayer.swapBuffers();

		if (openGifFilenameByIndex(GIF_DIRECTORY, index) >= 0) {
			// Can clear screen for new animation here, but this might cause flicker with short animations
			// matrix.fillScreen(COLOR_BLACK);
			// matrix.swapBuffers();

			decoder.startDecoding();

			// Calculate time in the future to terminate animation
			gifEndMillis = millis() + (DISPLAY_TIME_SECONDS * 1000);

			gifPlaying = true;
		}
	}

	if (gifEndMillis < millis()) {
		gifPlaying = false;
	}
	decoder.decodeFrame();
	return gifPlaying;
}
