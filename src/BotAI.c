// clear && gcc main.c BotAI.c -g -lm -o main `sdl2-config --cflags --libs` && ./main 
// clear && gcc main.c BotAI.c -g -lm -o main && ./main 

#include "BotAI.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef Array_for
	#define Array_for(Type, array, length, i)\
	for (Type *i = (array), *const i##_end = i + length; i < i##_end; i++)
#endif




float BotAI_normalize(float x, float min, float max) {
	if (max == min) {
		return 0.0f;
	}

	return -4.0f + 8.0f * (x - min) / (max - min);
}

void BotAI_init(BotAI* bot, const ushort* layers, uint layerLength) {
	bot->layers = layers;
	bot->layerLength = layerLength;

	// Get layerLength and weightLength
	uint weightLength = 0;
	const ushort* const layer_last = &layers[layerLength];
	ushort last = layers[0];
	ushort bufferSize = last;
	for (const ushort* ptr = &layers[1]; ptr < layer_last; ptr++) {
		ushort current = *ptr;
		if (current > bufferSize) {
			bufferSize = current;
		}

		weightLength += current * (last+1); // neurons * (entries + 1_for_biais)
		last = current;
	}
	

	bot->bufferSize = (uint)bufferSize;

	// Generate random weights
	float* weights = malloc(sizeof(float) * weightLength);
	bot->weights = weights;

	Array_for(float, weights, weightLength, w) {
		*w = 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
	}
}



void BotAI_load(BotAI* bot, const char* filename) {
    FILE* file = fopen(filename, "rb");
	if (!file) {
		perror("Error opening file for reading");
		return;
	}

	uint weightLength = 0;
	const ushort* const layer_last = &bot->layers[bot->layerLength];
	ushort last = bot->layers[0];
	ushort bufferSize = last;
	for (const ushort* ptr = &bot->layers[1]; ptr < layer_last; ptr++) {
		ushort current = *ptr;
		if (current > bufferSize) {
			bufferSize = current;
		}

		weightLength += current * (last+1); // neurons * (entries + 1_for_biais)
		last = current;
	}

	size_t read = fread(bot->weights, sizeof(float), weightLength, file);
	if (read != weightLength) {
		perror("Error reading from file");
        return;
	}
}



float BotAI_activation(float x) {
	return 1.0f / (1.0f + expf(-x));


	// if (x < 0)
		// return 0;
	// return x;


}


float* BotAI_run(
	uint layerLength,
	const ushort* layers,
	const float* weight,
	float* input,
	float* buffer
) {
	uint inputLayer = layers[0];
	Array_for(const ushort, &layers[1], layerLength - 1, layer_ptr) {		
		const uint outputLayer = (uint) *layer_ptr;
		// printf("FROM %d to %d\n", inputLayer, outputLayer);

		Array_for(float, buffer, outputLayer, r) {
			float vsum = *weight;
			float wsum = 0;
			weight++;
			
			// printf("	SUM:\n");
			for (uint inputIndex = 0; inputIndex < inputLayer; inputIndex++) {
				// printf("		%f %f\n", input[inputIndex], *weight);
				float w = *weight;
				vsum += input[inputIndex] * w;
				wsum += w;
				weight++;
			}

			// printf("		=> %f %f\n", vsum, BotAI_activation(vsum));

			*r = BotAI_activation(vsum);
		}
		
		inputLayer = outputLayer;

		// Result becomes the next input
		float* temp = buffer;
		buffer = input;
		input = temp;
	}
	
	return input;
}



void BotAI_cleanup(BotAI* bot) {
	free(bot->weights);
}





void BotPoolAI_createEmpty(
	BotPoolAI* pool,
	const ushort* layers,
	uint layerLength,
	uint botLength,
	bool fillRandom
) {
	pool->layers = layers;
	pool->layerLength = layerLength;
	pool->botLength = botLength;
	pool->scores = calloc(botLength, sizeof(float));

	// Get layerLength and weightLength
	uint weightLength = 0;
	const ushort* const layer_last = &layers[layerLength];
	ushort last = layers[0];
	ushort bufferSize = last;
	for (const ushort* ptr = &layers[1]; ptr < layer_last; ptr++) {
		ushort current = *ptr;
		if (current > bufferSize) {
			bufferSize = current;
		}

		weightLength += current * (last+1); // neurons * (entries + 1_for_biais)
		last = current;
	}
	

	pool->bufferSize = (uint)bufferSize;
	pool->weightLength = weightLength;
	
	// Generate random weights
	float** botWeights = malloc(sizeof(float*) * botLength);
	pool->botWeights = botWeights;

	for (uint i = botLength; i--; ) {
		float* weights = malloc(sizeof(float) * weightLength);
		botWeights[i] = weights;
		
		if (fillRandom) {
			Array_for(float, weights, weightLength, w) {
				*w = 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
			}
		}
		
	}
}


void BootPoolAI_create(
	BotPoolAI* pool,
	BotAI* bots,
	uint botLength
) {
	float** weights = malloc(sizeof(float*) * botLength);
	
	for (uint i = botLength; i--; ) {
		weights[i] = bots[i].weights;
	}


	pool->layers = bots[0].layers;
	pool->botWeights = weights;
	pool->scores = malloc(sizeof(float) * botLength);
	pool->layerLength = bots[0].layerLength;
	pool->bufferSize = bots[0].bufferSize;
	pool->botLength = botLength;
}





void BotPoolAI_openFile(BotPoolAI* pool,char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		perror("Error opening file for reading");
		return;
	}

	const size_t weightLength = pool->weightLength;

	for (
		float **ptr = pool->botWeights,
		**end = ptr + pool->botLength;
		ptr < end;
		ptr++
	) {
		size_t read = fread(*ptr, sizeof(float), weightLength, file);
		if (read != weightLength) {
			perror("Error reading from file");
			break;
		}
	}

}


void BotPoolAI_saveFile(const BotPoolAI* pool, char* filename) {
	FILE* file = fopen(filename, "wb");
	if (!file) {
		perror("Error opening file for writing");
		return;
	}

	const size_t weightLength = pool->weightLength;

	for (
		float **ptr = pool->botWeights,
		**end = ptr + pool->botLength;
		ptr < end;
		ptr++
	) {
		size_t written = fwrite(*ptr, sizeof(float), weightLength, file);
		if (written != weightLength) {
			perror("Error writing to file");
			break;
		}
	}

	fclose(file);
}






typedef struct {
	float* weights;
	float score;
} BotPoolAI_BotEntry;

static int BotPoolAI_compareBotEntries(const void* a, const void* b) {
	float scoreA = ((BotPoolAI_BotEntry*)a)->score;
	float scoreB = ((BotPoolAI_BotEntry*)b)->score;
	return (scoreB > scoreA) - (scoreB < scoreA); // tri décroissant
}

static void BotPoolAI_sortTopBots(
	float** botWeights,
	float* scores,
	int keptBotCount,
	int botLength
) {
	BotPoolAI_BotEntry* entries = malloc(sizeof(BotPoolAI_BotEntry) * botLength);
	if (!entries) return;

	for (int i = 0; i < botLength; ++i) {
		entries[i].weights = botWeights[i];
		entries[i].score = scores[i];
	}

	qsort(entries, botLength, sizeof(BotPoolAI_BotEntry), BotPoolAI_compareBotEntries);

	for (int i = 0; i < botLength; ++i) {
		botWeights[i] = entries[i].weights;
		scores[i] = entries[i].score;
	}

	free(entries);
}


static float random_gaussian() {
	static int hasSpare = 0;
	static double spare;

	if (hasSpare) {
		hasSpare = 0;
		return (float)spare;
	}

	hasSpare = 1;
	double u, v, s;
	do {
		u = (rand() / ((double) RAND_MAX)) * 2.0 - 1.0;
		v = (rand() / ((double) RAND_MAX)) * 2.0 - 1.0;
		s = u * u + v * v;
	} while (s >= 1.0 || s == 0.0);

	s = sqrt(-2.0 * log(s) / s);
	spare = v * s;
	return (float)(u * s);
}




void BotPoolAI_learn(
	BotPoolAI pool,
	BotPoolAI_Inputer inputer,
	BotPoolAI_Runner runner,
	BotPoolAI_Printer printer,
	BotPoolAI_Initializer initializer,
	BotPoolAI_Lapper lapper,
	void* data,
	ushort gameLaps,
	ushort simLaps,
	ushort simLapPrintDecalage,
	uint keptBotCount,
	uint eliteCount,
	uchar crossMethod,
	ushort* lapPointer
) {

	if (lapPointer) {
		*lapPointer = 0;
	}

	float** nextTopWeights = malloc(sizeof(float*) * (keptBotCount - eliteCount));

	pool.weightLength *= sizeof(float);
	for (uint i = keptBotCount - eliteCount; i--;) {
		nextTopWeights[i] = malloc(pool.weightLength);
	}
	
	pool.bufferSize *= sizeof(float); // buffer size
	float* const input = malloc(pool.bufferSize);
	float* const buffer = malloc(pool.bufferSize);
	const uint scoresSize = sizeof(float) * pool.botLength;
	

	for (ushort simLap = 0; simLap < simLaps; simLap++) {
		if (lapPointer) {
			*lapPointer = simLap;
		}

		// Clear scores and init game
		memset(pool.scores, 0, scoresSize);
		(initializer)(data);


		// Run simulations
		for (ushort gameLap = 0; gameLap < gameLaps; gameLap++) {
			(lapper)(data);

			for (uint botIndex = 0; botIndex < pool.botLength; botIndex++) {
				(inputer)(input, data, botIndex);
				
				float* output = BotAI_run(
					pool.layerLength,
					pool.layers,
					pool.botWeights[botIndex],
					input,
					buffer
				);
		
				pool.scores[botIndex] += (runner)(output, data, botIndex);
			}
			
		}
		
		
		// Sort weights
		BotPoolAI_sortTopBots(pool.botWeights, pool.scores, keptBotCount, pool.botLength);
		
		// Print scores
		if (simLap % BOTPOOLAI_SIM_LAP_PRINT == 0) {
			printf("===SIMULATION %02d===\n", simLap + simLapPrintDecalage);
			for(uint i = 0; i < keptBotCount; i++) {
				printf("#%02d ", i);
				(printer)(data, i);
				printf("\tscore=%4.2f\n", pool.scores[i] / gameLaps);
			}
			printf("\n");
		}


		// Copy top weights (because they will be edited later)
		for (uint i = eliteCount; i < keptBotCount; i++) {
			memcpy(
				nextTopWeights[i-eliteCount],
				pool.botWeights[i],
				pool.weightLength
			);
		}

		
		// Cross nodes
		for (uint i = eliteCount; i < pool.botLength; i++) {
			float* const weights = pool.botWeights[i];

			
			// Select parents weights
			const float* parent1;
			const float* parent2;
			{
				int i1 = rand() % keptBotCount;
				int i2;
				do {
					i2 = rand() % keptBotCount;
				} while (i2 == i1);

				if (i1 >= eliteCount) {
					parent1 = nextTopWeights[i1 - eliteCount];
				} else {
					parent1 = pool.botWeights[i1];
				}

				if (i2 >= eliteCount) {
					parent2 = nextTopWeights[i2 - eliteCount];
				} else {
					parent2 = pool.botWeights[i2];
				}
			}



			uint pos = 0;
			ushort lastLayer = pool.layers[0];
			const ushort* const layer_last = &pool.layers[pool.layerLength];
			for (const ushort* ptr = &pool.layers[1]; ptr < layer_last; ptr++) {
				const float* source;
				uint currentLayer = *ptr;

				for (uint layerIndex = 0; layerIndex < currentLayer; layerIndex++) {
					// Select parent
					switch (crossMethod) {
					case 0: // Uniform
					{
						source = rand() & 1 ? parent1 : parent2;
						break;
					}
	
					default:
					{
						source = parent1;
						break;
					}
					}
	
	
					// Copy nodes
					uint size = (lastLayer+1);
					for (uint end = pos + size; pos < end; pos++) {
						if ((float)rand()/RAND_MAX < BOTPOOLAI_MUTATION_RATE)
							weights[pos] = source[pos] + random_gaussian() * BOTPOOLAI_MUTATION_FACTOR;
					}
					// memcpy(&weights[pos], &source[pos], sizeof(float) * size);
				}

				lastLayer = currentLayer;
			}

		}
	}
	


	if(lapPointer) {
		*lapPointer = simLaps;
	}

	// Free data

	for (ushort i = keptBotCount - eliteCount; i--;) {
		free(nextTopWeights[i]);
	}

	free(nextTopWeights);
	free(input);
	free(buffer);
}




void BotPoolAI_run(
	const BotPoolAI* pool,
	void* data,
	BotPoolAI_Runner runner,
	BotPoolAI_Inputer inputer
) {	
	float* const input = malloc(pool->bufferSize * sizeof(float));
	float* const buffer = malloc(pool->bufferSize * sizeof(float));

	for (uint botIndex = 0; botIndex < pool->botLength; botIndex++) {
		(inputer)(input, data, botIndex);

		float* output = BotAI_run(
			pool->layerLength,
			pool->layers,
			pool->botWeights[botIndex],
			input,
			buffer
		);
		(runner)(output, data, botIndex);
	}

	free(input);
	free(buffer);
}


void BootPoolAI_cleanup(BotPoolAI* pool) {
	float** const botWeights = pool->botWeights;
	float** ptr = botWeights;
	float** const end_ptr = &ptr[pool->botLength];

	for (; ptr < end_ptr; ptr++) {
		free(*ptr);
	}

	free(botWeights);
	free(pool->scores);
}


