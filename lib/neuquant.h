#ifndef OCULAR_NEUQUANT_H
#define OCULAR_NEUQUANT_H

// Constants for NeuQuant algorithm
#define NETSIZE 256 // number of colors used
#define INITRAD 32  // initial radius
#define RADIUSBIASSHIFT 6
#define RADIUSBIAS (1 << RADIUSBIASSHIFT)
#define INITBIASRADIUS (INITRAD * RADIUSBIAS)
#define RADIUSDEC 90 // factor of 1/30 each cycle (Decreased from original 30 value for slower decay)
#define BIASWEIGHT 0.4 // Increased from 0.2
#define BETABIAS 0.02  // Increased from 0.01

#define ALPHA_BIASSHIFT 10
#define INIT_ALPHA (1 << ALPHA_BIASSHIFT)
#define GAMMA 1024
#define BETA (1.0 / GAMMA)
#define BETAGAMMA (BETA * GAMMA)

typedef struct {
    double r;
    double g;
    double b;
    double a;
} NeuronColor;

typedef struct {
    NeuronColor* network; // the network itself
    int netindex[256];    // for network lookup - really 256
    double bias[256];     // bias array for learning
    double freq[256];     // frequency array for learning
} NeuQuant;


// Generates an optimal palette using the NeuQuant algorithm
bool generateOptimalPaletteNeuQuant(unsigned char* image, int width, int height, int channels, int maxColors, OcPalette* palette);

#endif /* OCULAR_NEUQUANT_H */
