#ifndef BOTAI_H_
#define BOTAI_H_

#include <tools/tools.h>


#define BOTPOOLAI_MUTATION_FACTOR 0.1f
#define BOTPOOLAI_MUTATION_RATE .04f
enum {BOTPOOLAI_SIM_LAP_PRINT = 50};


structdef(BotAI);
structdef(BotPoolAI);

/**
 * @return score
 */
typedef float (BotPoolAI_Runner)
	(const float* output, void* data, uint index);


/**
 * Generates an input from data
 */
typedef void BotPoolAI_Inputer
	(float* input, const void* data, uint index);


typedef void BotPoolAI_Printer
	(const void* data, uint index);


typedef void (BotPoolAI_Initializer)(void* data);
typedef void (BotPoolAI_Lapper)(void* data);

struct BotAI {
	const ushort* layers;
	float* weights;
	uint layerLength;
	uint bufferSize;
};

struct BotPoolAI {
	const ushort* layers;
	float** botWeights;
	float* scores;
	uint layerLength;
	uint bufferSize;
	uint botLength;
	uint weightLength;
};


float BotAI_normalize(float x, float min, float max);


void BotAI_init(BotAI* bot, const ushort* layers, uint layerLength);

float BotAI_activation(float x);

/**
 * @param layerLength  	Number of layers
 * @param layers 		Layers
 * @param weight 		List of weights
 * @param input  		Input data with size `sizeof(float)*bot.bufferSize`
 * @param buffer 		Buffer data with size `sizeof(float)*bot.bufferSize`
 * 
 * @warning 			`input` data will be modified.
 * @return				Returns `input` or `buffer`
 * 
 * 
 */
float* BotAI_run(
	uint layerLength,
	const ushort* layers,
	const float* weight,
	float* input,
	float* buffer
);


void BotAI_cleanup(BotAI* bot);


void BotPoolAI_createEmpty(
	BotPoolAI* pool,
	const ushort* layers,
	uint layerLength,
	uint botLength
);

void BootPoolAI_create(
	BotPoolAI* pool,
	BotAI* bots,
	uint botLength
);

/**
 * Update scores
 * 
 * @param pool  		Pool
 * @param input  		Input data
 * @param runner 		Edits data object and returns score
 * @param printer 		Prints data
 * @param initializer 	Initialize data
 * @param data 			Simulation data
 * @param keptBotCount 	Number of bots used for crossing
 * @param eliteCount 	Number of bots not changed
 * @param crossMethod 	0=uniform ; 1=one-point ; 2=average
 * 
 */
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
	uchar crossMethod
);

void BotPoolAI_run(
	const BotPoolAI* pool,
	void* data,
	BotPoolAI_Runner runner,
	BotPoolAI_Inputer inputer
);


void BootPoolAI_cleanup(BotPoolAI* pool);



#endif