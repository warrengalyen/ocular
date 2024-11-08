#include "neuquant.h"

// Initialize the NeuQuant structure
static void initNeuQuant(NeuQuant* nq) {
    nq->network = (NeuronColor*)malloc(NETSIZE * sizeof(NeuronColor));

    // Initialize with better RGB distribution
    for (int i = 0; i < NETSIZE; i++) {
        // Use prime numbers to create better distribution
        nq->network[i].r = (i * 13) % 256;
        nq->network[i].g = (i * 17) % 256;
        nq->network[i].b = (i * 23) % 256;
        nq->network[i].a = 0;

        nq->freq[i] = 1.0 / NETSIZE;
        nq->bias[i] = 0.0;
    }
}

// Learn the colors from the image
static void learn(NeuQuant* nq, unsigned char* image, int len, int samplefac) {
    int alphadec = 30 + ((samplefac - 1) / 3);
    int samplepixels = len / (3 * samplefac);
    int delta = samplepixels / INITBIASRADIUS;
    int alpha = INIT_ALPHA;
    int radius = INITBIASRADIUS;

    int rad = radius >> RADIUSBIASSHIFT;
    if (rad <= 1)
        rad = 0;

    int step = 0;
    int pos = 0;

    // Main learning loop
    for (int i = 0; i < samplepixels;) {
        int r = image[pos + 0];
        int g = image[pos + 1];
        int b = image[pos + 2];

        // Find best neuron (min dist)
        int bestpos = 0;
        double bestd = 1000000000.0;

        for (int j = 0; j < NETSIZE; j++) {
            // Use weighted Euclidean distance for better color perception
            double dr = (nq->network[j].r - r);
            double dg = (nq->network[j].g - g);
            double db = (nq->network[j].b - b);

            // Apply perceptual weights (human eye is more sensitive to green)
            double dist = (dr * dr * 0.299) + (dg * dg * 0.587) + (db * db * 0.114);

            // Stronger bias factor to prevent color dominance
            dist = dist * (1.0 + nq->bias[j] * BIASWEIGHT * 2.0);

            if (dist < bestd) {
                bestd = dist;
                bestpos = j;
            }
        }

        // Update frequency
        nq->freq[bestpos] += BETA * (1.0 - nq->freq[bestpos]);

        // Move neuron
        double alpha_gamma = alpha * GAMMA;

        nq->network[bestpos].r -= (alpha_gamma * (nq->network[bestpos].r - r)) / INIT_ALPHA;
        nq->network[bestpos].g -= (alpha_gamma * (nq->network[bestpos].g - g)) / INIT_ALPHA;
        nq->network[bestpos].b -= (alpha_gamma * (nq->network[bestpos].b - b)) / INIT_ALPHA;

        if (rad > 0) {
            // Update neighbors
            int lo = bestpos - rad;
            if (lo < 0)
                lo = 0;
            int hi = bestpos + rad;
            if (hi > NETSIZE)
                hi = NETSIZE;

            for (int j = lo; j < hi; j++) {
                double alpha_rad = alpha * (((rad * rad - (j - bestpos) * (j - bestpos)) * RADIUSBIAS) / (rad * rad));

                nq->network[j].r -= (alpha_rad * (nq->network[j].r - r)) / INIT_ALPHA;
                nq->network[j].g -= (alpha_rad * (nq->network[j].g - g)) / INIT_ALPHA;
                nq->network[j].b -= (alpha_rad * (nq->network[j].b - b)) / INIT_ALPHA;
            }
        }

        pos += 3 * samplefac;
        i++;

        if (i % delta == 0) {
            alpha = (int)(alpha * (1.0 - ((double)i / samplepixels) * 0.75));
            if (alpha < 2)
                alpha = 2; // Keep minimum learning rate higher

            // Slower radius decay
            radius = (radius * (RADIUSDEC + 10)) / 100;
            rad = radius >> RADIUSBIASSHIFT;
            if (rad <= 1)
                rad = 0;
        }

        // Update bias of winner
        nq->bias[bestpos] += BETABIAS * 2.0 * (1.0 - nq->bias[bestpos]);
    }
}

// Build the index for the network
static void buildIndex(NeuQuant* nq) {
    int previouscol = 0;
    int startpos = 0;

    for (int i = 0; i < NETSIZE; i++) {
        int smallpos = i;
        int smallval = (int)nq->network[i].g;

        for (int j = i + 1; j < NETSIZE; j++) {
            if ((int)nq->network[j].g < smallval) {
                smallpos = j;
                smallval = (int)nq->network[j].g;
            }
        }

        // Swap
        NeuronColor temp = nq->network[smallpos];
        nq->network[smallpos] = nq->network[i];
        nq->network[i] = temp;

        if (smallval != previouscol) {
            nq->netindex[previouscol] = (startpos + i) >> 1;
            for (int j = previouscol + 1; j < smallval; j++) {
                nq->netindex[j] = i;
            }
            previouscol = smallval;
            startpos = i;
        }
    }

    nq->netindex[previouscol] = (startpos + (NETSIZE - 1)) >> 1;
    for (int j = previouscol + 1; j < 256; j++) {
        nq->netindex[j] = NETSIZE - 1;
    }
}

bool generateOptimalPaletteNeuQuant(unsigned char* image, int width, int height, int channels, int maxColors, OcPalette* palette) {
    if (!image || !palette || maxColors <= 0 || maxColors > NETSIZE) {
        return false;
    }

    // Initialize palette
    palette->num_colors = 0;
    palette->capacity = maxColors;
    palette->colors = malloc(maxColors * sizeof(OcPaletteColor));
    strncpy(palette->name, "Generated NeuQuant Palette", 255);

    if (!palette->colors) {
        return false;
    }

    // Initialize NeuQuant
    NeuQuant nq;
    initNeuQuant(&nq);

    // Learn from the image
    int samplefac = 1; // Sampling factor, increase for speed at cost of quality
    learn(&nq, image, width * height * channels, samplefac);

    // Build index
    buildIndex(&nq);

    // Convert network to palette
    for (int i = 0; i < maxColors; i++) {
        palette->colors[i].r = (unsigned char)nq.network[i].r;
        palette->colors[i].g = (unsigned char)nq.network[i].g;
        palette->colors[i].b = (unsigned char)nq.network[i].b;
        palette->colors[i].name[0] = '\0';
        palette->num_colors++;
    }

    // Clean up
    free(nq.network);

    return true;
}