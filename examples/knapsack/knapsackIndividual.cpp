

#include <fstream>
#include <time.h>
#include <cstring>
#include <sstream>
#include "CRandomGenerator.h"
#include "CPopulation.h"
#include "COptionParser.h"
#include "CStoppingCriterion.h"
#include "CEvolutionaryAlgorithm.h"
#include "global.h"
#include "CIndividual.h"

using namespace std;

#include "knapsackIndividual.hpp"
bool INSTEAD_EVAL_STEP = false;

CRandomGenerator* globalRandomGenerator;
extern CEvolutionaryAlgorithm* EA;
#define STD_TPL

// User declarations
#line 24 "./knapsack.ez"

    #include <vector>

    // scenario settings
    #define MAX_ITEMS 10    // maximum chromosome size
    #define MAX_KNAPSACK_WEIGHT 10
    #define CROSSOVER_PROB 0.8
    #define MUTATION_PROB 0.1

    // MIN/MAX bounds for item creation
    #define MIN_VALUE_PER_ITEM 8
    #define MAX_VALUE_PER_ITEM 20
    #define MIN_WEIGHT_PER_ITEM 1
    #define MAX_WEIGHT_PER_ITEM 5






// User functions

#line 43 "./knapsack.ez"




// Initialisation function
void EASEAInitFunction(int argc, char *argv[]){
#line 61 "./knapsack.ez"

    std::cout << "Before everything else function called..." << std::endl;
}

// Finalization function
void EASEAFinalization(CPopulation* population){
#line 65 "./knapsack.ez"

}



void evale_pop_chunk(CIndividual** population, int popSize){
  
// No Instead evaluation step function.

}

void knapsackInit(int argc, char** argv){
	
  EASEAInitFunction(argc, argv);

}

void knapsackFinal(CPopulation* pop){
	
  EASEAFinalization(pop);
;
}

void EASEABeginningGenerationFunction(CEvolutionaryAlgorithm* evolutionaryAlgorithm){
	#line 213 "./knapsack.ez"

// No at beginning of generation function.

}

void EASEAEndGenerationFunction(CEvolutionaryAlgorithm* evolutionaryAlgorithm){
	
// No at end of generation function.

}

void EASEAGenerationFunctionBeforeReplacement(CEvolutionaryAlgorithm* evolutionaryAlgorithm){
	
// No generation function.

}


IndividualImpl::IndividualImpl() : CIndividual() {
   
  // Genome Initialiser
#line 68 "./knapsack.ez"

    std::cout << "initializer called..." << std::endl;
    // Populate list of items
    for (int i = 0; i < MAX_ITEMS; ++i) {
        Item *item = new Item;
        item->value = globalRandomGenerator->random(MIN_VALUE_PER_ITEM, MAX_VALUE_PER_ITEM);
        item->weight = globalRandomGenerator->random(MIN_WEIGHT_PER_ITEM, MAX_WEIGHT_PER_ITEM);
        (*this).itemList[i] = item;
    }
    // Populate genome
    for (int i = 0; i < MAX_ITEMS; i++) {
        (*this).x[i] = globalRandomGenerator->random(0,1);
    }

  valid = false;
  isImmigrant = false;
}

CIndividual* IndividualImpl::clone(){
	return new IndividualImpl(*this);
}

IndividualImpl::~IndividualImpl(){
  // Destructing pointers
    for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++)
      if(itemList[EASEA_Ndx]) delete itemList[EASEA_Ndx];

}


float IndividualImpl::evaluate(){
  if(valid)
    return fitness;
  else{
    valid = true;
    #line 105 "./knapsack.ez"
    // evaluate fitness
    int fitness;
    double totalValue, totalWeight;
    bool overweightFlag = false;
    std::vector<int> temp;

    do{ // while knapsack NOT overweight
        totalValue = totalWeight = fitness = 0;
        overweightFlag = false;
        for (int i = 0; i < MAX_ITEMS; ++i) {
            // if item selected in individual
            if((*this).x[i] == 1){
                // add value and weight to total
                totalValue += (*this).itemList[i]->value;
                totalWeight += (*this).itemList[i]->weight;
                temp.push_back(i);
                if(totalWeight > MAX_KNAPSACK_WEIGHT){
                    overweightFlag = true;
                    break;
                }
            }
        }
        // if knapsack overweight, remove one item from chromosome at random
        if (overweightFlag){
            // randomize chromosome
            std::vector<int> availableIndexes;
            int selectedElement;
            // find all elements in chromosome where item is '1'
            for (int j = 0; j < MAX_ITEMS; ++j) {
                if ((*this).x[j] == 1){
                    availableIndexes.push_back(j);
                }
            }
            // get a random element from temp vector
            selectedElement = temp.at(globalRandomGenerator->random(0, (availableIndexes.size()-1)));
            // remove item (element = 0)
            (*this).x[selectedElement] = 0;

            // unallocated memory for temp vector
            std::vector<int>().swap(availableIndexes);
        }else{
            fitness = totalValue;

        // log best fitness as a struct
        /*
            if(bestChromLog.fitness < fitness){
                bestChromLog.chrom = x;
                bestChromLog.index = count;
                bestChromLog.fitness = fitness;
                bestChromLog.maxValue = totalValue;
                bestChromLog.maxWeight = totalWeight;
                bestChromLog.itemList = temp;
            }
        */
            (*this).fitness = fitness;
            temp.clear();
        }
    }while(overweightFlag);

    std::vector<int>().swap(temp); // clear allocated memory
    std::cout << "fitness --> " << fitness;
    std::cout << totalWeight << " <-- weight" << std::endl;
    return fitness =  fitness;

  }
}

void IndividualImpl::boundChecking(){
	
// No Bound checking function.

}

string IndividualImpl::serialize(){
    ostringstream EASEA_Line(ios_base::app);
    // Memberwise serialization
	for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++){
		if(this->itemList[EASEA_Ndx] != NULL){
			EASEA_Line << "\a ";
			EASEA_Line << this->itemList[EASEA_Ndx]->serializer() << " ";
	}
		else
			EASEA_Line << "NULL" << " ";
}
	for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++)
		EASEA_Line << this->x[EASEA_Ndx] <<" ";

    EASEA_Line << this->fitness;
    return EASEA_Line.str();
}

void IndividualImpl::deserialize(string Line){
    istringstream EASEA_Line(Line);
    string line;
    // Memberwise deserialization
	EASEA_Line >> line;
	for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++){
		if(strcmp(line.c_str(),"NULL")==0)
			this->itemList[EASEA_Ndx] = NULL;
		else{
			this->itemList[EASEA_Ndx] = new Item;
			this->itemList[EASEA_Ndx]->deserializer(&EASEA_Line);
		}	}	for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++)
		EASEA_Line >> this->x[EASEA_Ndx];

    EASEA_Line >> this->fitness;
    this->valid=true;
    this->isImmigrant = false;
}

IndividualImpl::IndividualImpl(const IndividualImpl& genome){

  // ********************
  // Problem specific part
  // Memberwise copy
    for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++)
      if(genome.itemList[EASEA_Ndx]) itemList[EASEA_Ndx] = new Item(*(genome.itemList[EASEA_Ndx]));
      else itemList[EASEA_Ndx] = NULL;
    {for(int EASEA_Ndx=0; EASEA_Ndx<10; EASEA_Ndx++)
       x[EASEA_Ndx]=genome.x[EASEA_Ndx];}



  // ********************
  // Generic part
  this->valid = genome.valid;
  this->fitness = genome.fitness;
  this->isImmigrant = false;
}


CIndividual* IndividualImpl::crossover(CIndividual** ps){
	// ********************
	// Generic part
	IndividualImpl** tmp = (IndividualImpl**)ps;
	IndividualImpl parent1(*this);
	IndividualImpl parent2(*tmp[0]);
	IndividualImpl child(*this);

	//DEBUG_PRT("Xover");
	/*   cout << "p1 : " << parent1 << endl; */
	/*   cout << "p2 : " << parent2 << endl; */

	// ********************
	// Problem specific part
  	#line 84 "./knapsack.ez"

  for (int i=0; i<MAX_ITEMS; i++)
  {
    float alpha = (float)globalRandomGenerator->random(0.,1.);     // barycentric crossover
     child.x[i] = alpha*parent1.x[i] + (1.-alpha)*parent2.x[i];
  }



	child.valid = false;
	/*   cout << "child : " << child << endl; */
	return new IndividualImpl(child);
}


void IndividualImpl::printOn(std::ostream& os) const{
	

}

std::ostream& operator << (std::ostream& O, const IndividualImpl& B)
{
  // ********************
  // Problem specific part
  O << "\nIndividualImpl : "<< std::endl;
  O << "\t\t\t";
  B.printOn(O);

  if( B.valid ) O << "\t\t\tfitness : " << B.fitness;
  else O << "fitness is not yet computed" << std::endl;
  return O;
}


unsigned IndividualImpl::mutate( float pMutationPerGene ){
  this->valid=false;


  // ********************
  // Problem specific part
  #line 92 "./knapsack.ez"
      // return number of mutations
    int nbMutation = 0;
    for (int i = 0; i < MAX_ITEMS; i++){
        if (globalRandomGenerator->tossCoin(MUTATION_PROB)){
            (*this).x[i] = (*this).x[i] = ((*this).x[i] + 1) % 2;
            nbMutation++;
        }
    }
    return  nbMutation>0?true:false;

}

void ParametersImpl::setDefaultParameters(int argc, char** argv){

	this->minimizing = false;
	this->nbGen = setVariable("nbGen",(int)100);

	seed = setVariable("seed",(int)time(0));
	globalRandomGenerator = new CRandomGenerator(seed);
	this->randomGenerator = globalRandomGenerator;


	selectionOperator = getSelectionOperator(setVariable("selectionOperator","Roulette"), this->minimizing, globalRandomGenerator);
	replacementOperator = getSelectionOperator(setVariable("reduceFinalOperator","Tournament"),this->minimizing, globalRandomGenerator);
	parentReductionOperator = getSelectionOperator(setVariable("reduceParentsOperator","Tournament"),this->minimizing, globalRandomGenerator);
	offspringReductionOperator = getSelectionOperator(setVariable("reduceOffspringOperator","Tournament"),this->minimizing, globalRandomGenerator);
	selectionPressure = setVariable("selectionPressure",(float)0.000000);
	replacementPressure = setVariable("reduceFinalPressure",(float)2.000000);
	parentReductionPressure = setVariable("reduceParentsPressure",(float)2.000000);
	offspringReductionPressure = setVariable("reduceOffspringPressure",(float)2.000000);
	pCrossover = 0.800000;
	pMutation = 0.100000;
	pMutationPerGene = 0.05;

	parentPopulationSize = setVariable("popSize",(int)10);
	offspringPopulationSize = setVariable("nbOffspring",(int)10);


	parentReductionSize = setReductionSizes(parentPopulationSize, setVariable("survivingParents",(float)1.000000));
	offspringReductionSize = setReductionSizes(offspringPopulationSize, setVariable("survivingOffspring",(float)1.000000));

	this->elitSize = setVariable("elite",(int)1);
	this->strongElitism = setVariable("eliteType",(int)1);

	if((this->parentReductionSize + this->offspringReductionSize) < this->parentPopulationSize){
		printf("*WARNING* parentReductionSize + offspringReductionSize < parentPopulationSize\n");
		printf("*WARNING* change Sizes in .prm or .ez\n");
		printf("EXITING\n");
		exit(1);	
	} 
	if((this->parentPopulationSize-this->parentReductionSize)>this->parentPopulationSize-this->elitSize){
		printf("*WARNING* parentPopulationSize - parentReductionSize > parentPopulationSize - elitSize\n");
		printf("*WARNING* change Sizes in .prm or .ez\n");
		printf("EXITING\n");
		exit(1);	
	} 
	if(!this->strongElitism && ((this->offspringPopulationSize - this->offspringReductionSize)>this->offspringPopulationSize-this->elitSize)){
		printf("*WARNING* offspringPopulationSize - offspringReductionSize > offspringPopulationSize - elitSize\n");
		printf("*WARNING* change Sizes in .prm or .ez\n");
		printf("EXITING\n");
		exit(1);	
	} 
	

	/*
	 * The reduction is set to true if reductionSize (parent or offspring) is set to a size less than the
	 * populationSize. The reduction size is set to populationSize by default
	 */
	if(offspringReductionSize<offspringPopulationSize) offspringReduction = true;
	else offspringReduction = false;

	if(parentReductionSize<parentPopulationSize) parentReduction = true;
	else parentReduction = false;

	generationalCriterion = new CGenerationalCriterion(setVariable("nbGen",(int)100));
	controlCStopingCriterion = new CControlCStopingCriterion();
	timeCriterion = new CTimeCriterion(setVariable("timeLimit",0));

	this->optimise = 0;

	this->printStats = setVariable("printStats",1);
	this->generateCSVFile = setVariable("generateCSVFile",0);
	this->generatePlotScript = setVariable("generatePlotScript",0);
	this->generateRScript = setVariable("generateRScript",0);
	this->plotStats = setVariable("plotStats",0);
	this->printInitialPopulation = setVariable("printInitialPopulation",0);
	this->printFinalPopulation = setVariable("printFinalPopulation",0);
	this->savePopulation = setVariable("savePopulation",0);
	this->startFromFile = setVariable("startFromFile",0);

	this->outputFilename = (char*)"knapsack";
	this->plotOutputFilename = (char*)"knapsack.png";

	this->remoteIslandModel = setVariable("remoteIslandModel",0);
	std::string* ipFilename=new std::string();
	*ipFilename=setVariable("ipFile","ip.txt");

	this->ipFile =(char*)ipFilename->c_str();
	this->migrationProbability = setVariable("migrationProbability",(float)0.300000);
    this->serverPort = setVariable("serverPort",2929);
}

CEvolutionaryAlgorithm* ParametersImpl::newEvolutionaryAlgorithm(){

	pEZ_MUT_PROB = &pMutationPerGene;
	pEZ_XOVER_PROB = &pCrossover;
	//EZ_NB_GEN = (unsigned*)setVariable("nbGen",100);
	EZ_current_generation=0;
  EZ_POP_SIZE = parentPopulationSize;
  OFFSPRING_SIZE = offspringPopulationSize;

	CEvolutionaryAlgorithm* ea = new EvolutionaryAlgorithmImpl(this);
	generationalCriterion->setCounterEa(ea->getCurrentGenerationPtr());
	ea->addStoppingCriterion(generationalCriterion);
	ea->addStoppingCriterion(controlCStopingCriterion);
	ea->addStoppingCriterion(timeCriterion);	

	EZ_NB_GEN=((CGenerationalCriterion*)ea->stoppingCriteria[0])->getGenerationalLimit();
	EZ_current_generation=&(ea->currentGeneration);

	 return ea;
}

void EvolutionaryAlgorithmImpl::initializeParentPopulation(){
	if(this->params->startFromFile){
	  ifstream EASEA_File("knapsack.pop");
	  string EASEA_Line;
  	  for( unsigned int i=0 ; i< this->params->parentPopulationSize ; i++){
	  	  getline(EASEA_File, EASEA_Line);
		  this->population->addIndividualParentPopulation(new IndividualImpl(),i);
		  ((IndividualImpl*)this->population->parents[i])->deserialize(EASEA_Line);
	  }
	  
	}
	else{
  	  for( unsigned int i=0 ; i< this->params->parentPopulationSize ; i++){
		  this->population->addIndividualParentPopulation(new IndividualImpl(),i);
	  }
	}
        this->population->actualParentPopulationSize = this->params->parentPopulationSize;
}


EvolutionaryAlgorithmImpl::EvolutionaryAlgorithmImpl(Parameters* params) : CEvolutionaryAlgorithm(params){
	;
}

EvolutionaryAlgorithmImpl::~EvolutionaryAlgorithmImpl(){

}

