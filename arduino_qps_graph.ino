// Teensy/SmartMatrix code for displaying QPS graph.
// Takes updates over serial:
//    - integer (0-32) followed by a ','
//    - integer draws that height on the rightmost column
//    - all other columns left shifted, dropping column 0

#include <SmartMatrix3.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

//const int defaultBrightness = 100*(255/100);    // full brightness
const int defaultBrightness = 15 * (255 / 100); // dim: 15% brightness

const rgb24 defaultBackgroundColor = {0, 0, 0};

#define NUM_COLUMNS 32
#define NUM_TYPES 5
int qps[NUM_COLUMNS][NUM_TYPES];
rgb24 colors[NUM_TYPES];
char incoming_buffer[128];
int new_values[NUM_TYPES];

void setup() {
  Serial.begin(9600);

colors[0] = rgb24(255, 0, 0);
colors[1] = rgb24(0, 255, 0);
colors[2] = rgb24(0, 0, 255);
colors[3] = rgb24(255, 255, 0);
colors[4] = rgb24(255, 0, 255);


  matrix.addLayer(&backgroundLayer);
  matrix.addLayer(&indexedLayer);
  matrix.begin();

  matrix.setBrightness(defaultBrightness);

  backgroundLayer.enableColorCorrection(true);
}

void Draw() {
  // Clear the matrix.
  backgroundLayer.fillScreen(defaultBackgroundColor);
  backgroundLayer.swapBuffers();

  for (int col = 0; col < NUM_COLUMNS; col++) {
      int current_row = 0;

    for (int type = 0; type < NUM_TYPES; type++) {
      int num_rows = qps[col][type];
      for (int row = current_row; row < current_row + num_rows; row++) {
        backgroundLayer.drawPixel(row, col, colors[type]);
      }
      current_row += num_rows;
    }
  }
  backgroundLayer.swapBuffers();
}


void loop() {
  if (Serial.available() > 0) {
    Serial.readBytesUntil(';', incoming_buffer, 128);

    // Contents of incoming_buffer should be something like:
    // 10,5,1,0,8,;
    // Read out each value from the buffer.
    char* idx_start = incoming_buffer;
    char* idx_end = incoming_buffer;
    for (int i = 0; i < NUM_TYPES; ++i) {
      idx_end = strchr(idx_start, ',');
      new_values[i] = strtol(idx_start, &idx_end, 0);
      idx_start = idx_end + 1;
      Serial.print("Got value ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(new_values[i]);
      Serial.print('\n');
    }

    memset(incoming_buffer, 0, 128);

    // Left shift existing columns.
    // Column 0 gets dropped.
    for (int i = 0; i < NUM_COLUMNS - 1; ++i) {
      for (int j = 0; j < NUM_TYPES; ++j) {
        qps[i][j] = qps[i + 1][j];
      }
    }

    for (int j = 0; j < NUM_TYPES; ++j) {
            Serial.print("Adding value ");
      Serial.print(j);
      Serial.print(": ");
      Serial.print(new_values[j]);
      Serial.print('\n');
      qps[NUM_COLUMNS - 1][j] = new_values[j];
    }
    Draw();

  }
}
