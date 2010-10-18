#ifdef WIN32
#pragma comment(lib, "WinMM.lib")
#endif
/*
 * CEvolutionaryAlgorithm.cpp
 *
 *  Created on: 22 juin 2009
 *      Author: maitre
 */

#include "include/CEvolutionaryAlgorithm.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "include/CIndividual.h"
#include "include/Parameters.h"
#include "include/CGnuplot.h"
#include "include/global.h"
#include "include/CComUDPLayer.h"
#include "include/CRandomGenerator.h"
#include <stdio.h>
#include <sstream>
#include <fstream>

using namespace std;

extern CRandomGenerator* globalRandomGenerator;
extern CEvolutionaryAlgorithm* EA;
void EASEABeginningGenerationFunction(CEvolutionaryAlgorithm* evolutionaryAlgorithm);
void EASEAEndGenerationFunction(CEvolutionaryAlgorithm* evolutionaryAlgorithm);
void EASEAGenerationFunctionBeforeReplacement(CEvolutionaryAlgorithm* evolutionaryAlgorithm);

extern void evale_pop_chunk(CIndividual** pop, int popSize);
extern bool INSTEAD_EVAL_STEP;

/**
 * @DEPRECATED the next contructor has to be used instead of this one.
 */
/*CEvolutionaryAlgorithm::CEvolutionaryAlgorithm( size_t parentPopulationSize,
					      size_t offspringPopulationSize,
					      float selectionPressure, float replacementPressure, float parentReductionPressure, float offspringReductionPressure,
					      CSelectionOperator* selectionOperator, CSelectionOperator* replacementOperator,
					      CSelectionOperator* parentReductionOperator, CSelectionOperator* offspringReductionOperator,
					      float pCrossover, float pMutation,
					      float pMutationPerGene){

  CRandomGenerator* rg = globalRandomGenerator;

  CSelectionOperator* so = selectionOperator;
  CSelectionOperator* ro = replacementOperator;

  //CIndividual::initRandomGenerator(rg);
  CPopulation::initPopulation(so,ro,parentReductionOperator,offspringReductionOperator,selectionPressure,replacementPressure,parentReductionPressure,offspringReductionPressure);

  this->population = new CPopulation(parentPopulationSize,offspringPopulationSize,
				    pCrossover,pMutation,pMutationPerGene,rg,NULL);

  this->currentGeneration = 0;

  this->reduceParents = 0;
  this->reduceOffsprings = 0;
  
  // INITIALIZE SERVER OBJECT ISLAND MODEL
  if(params->remoteIslandModel){
  	this->server = new CComUDPServer(2909,0);
	this->treatedIndividuals = 0;
	this->numberOfClients = 0;

	this->initializeClients();
  }
}*/

/*****
 * REAL CONSTRUCTOR
 */
CEvolutionaryAlgorithm::CEvolutionaryAlgorithm(Parameters* params){
	this->params = params;

	CPopulation::initPopulation(params->selectionOperator,params->replacementOperator,params->parentReductionOperator,params->offspringReductionOperator,
			params->selectionPressure,params->replacementPressure,params->parentReductionPressure,params->offspringReductionPressure);

	this->population = new CPopulation(params->parentPopulationSize,params->offspringPopulationSize,
			params->pCrossover,params->pMutation,params->pMutationPerGene,params->randomGenerator,params);

	this->currentGeneration = 0;

	this->reduceParents = 0;
	this->reduceOffsprings = 0;
	this->gnuplot = NULL;
	if(params->plotStats || params->generateGnuplotScript){
		string fichier = params->outputFilename;
		fichier.append(".dat");
		remove(fichier.c_str());
	}
	if(params->generateGnuplotScript){
		string fichier = params->outputFilename;
		fichier.append(".plot");
		remove(fichier.c_str());
	}
	if(params->generateRScript || params->generateCSVFile){
		string fichier = params->outputFilename;
		fichier.append(".csv");
		remove(fichier.c_str());
	}
	if(params->generateRScript){
		string fichier = params->outputFilename;
		fichier.append(".r");
		remove(fichier.c_str());
	}
	#ifdef __linux__
	if(params->plotStats){
		this->gnuplot = new CGnuplot((this->params->offspringPopulationSize*this->params->nbGen)+this->params->parentPopulationSize);
	}
	#endif


	// INITIALIZE SERVER OBJECT ISLAND MODEL
	if(params->remoteIslandModel){
		server = new CComUDPServer(2909,0); //1 if debug
		this->treatedIndividuals = 0;
		this->numberOfClients = 0;
		this->myClientNumber=0;	
		this->initializeClients();
	}
}

/* DESTRUCTOR */
CEvolutionaryAlgorithm::~CEvolutionaryAlgorithm(){
	delete population;
        if(this->params->remoteIslandModel){
                delete this->server;
                if(this->numberOfClients>1){
                        for(int i=0; (unsigned)i<this->numberOfClients; i++)
                                delete this->Clients[i];
                        delete this->Clients;
                }
        }
}
void CEvolutionaryAlgorithm::addStoppingCriterion(CStoppingCriterion* sc){
  this->stoppingCriteria.push_back(sc);
}

/* MAIN FUNCTION TO RUN THE EVOLUTIONARY LOOP */
void CEvolutionaryAlgorithm::runEvolutionaryLoop(){
  CIndividual** elitistPopulation;

#ifdef WIN32
   clock_t begin(clock());
#else
  struct timeval begin;
  gettimeofday(&begin,0);
#endif

  std::cout << "Population initialisation (Generation 0)... "<< std::endl;
  this->initializeParentPopulation();

  if(!INSTEAD_EVAL_STEP)
    this->population->evaluateParentPopulation();
  else
    evale_pop_chunk(population->parents, population->parentPopulationSize);

  if(this->params->optimise){
        population->optimiseParentPopulation();
  }

  this->population->currentEvaluationNb += this->params->parentPopulationSize;
  if(this->params->printInitialPopulation){
  	std::cout << *population << std::endl;
  }

  showPopulationStats(begin);
  bBest = population->Best;
  currentGeneration += 1;

  //Initialize elitPopulation
  if(params->elitSize)
		elitistPopulation = (CIndividual**)malloc(params->elitSize*sizeof(CIndividual*));	

  // EVOLUTIONARY LOOP
  while( this->allCriteria() == false){

    EASEABeginningGenerationFunction(this);

    // Sending individuals if remote island model
    if(params->remoteIslandModel && this->numberOfClients>0)
	    this->sendIndividual();

    population->produceOffspringPopulation();

    if(!INSTEAD_EVAL_STEP)
      population->evaluateOffspringPopulation();
    else
      evale_pop_chunk(population->offsprings, population->offspringPopulationSize);
    population->currentEvaluationNb += this->params->offspringPopulationSize;

    if(this->params->optimise){
          population->optimiseOffspringPopulation();
    }

    EASEAGenerationFunctionBeforeReplacement(this);

    /* ELITISM */
    if(params->elitSize && this->params->parentPopulationSize>=params->elitSize){
	/* STRONG ELITISM */
	if(params->strongElitism){
		population->strongElitism(params->elitSize, population->parents, this->params->parentPopulationSize, elitistPopulation, params->elitSize);
		population->actualParentPopulationSize -= params->elitSize;
	}
	/* WEAK ELITISM */
	else{
		population->weakElitism(params->elitSize, population->parents, population->offsprings, &(population->actualParentPopulationSize), &(population->actualOffspringPopulationSize), elitistPopulation, params->elitSize);
	}
	
    }

    if( params->parentReduction )
      population->reduceParentPopulation(params->parentReductionSize);

    if( params->offspringReduction )
      population->reduceOffspringPopulation( params->offspringReductionSize );

    population->reduceTotalPopulation(elitistPopulation);

    population->sortParentPopulation();
    if( this->params->printStats  || this->params->generateCSVFile )
      showPopulationStats(begin);
    bBest = population->Best;
    EASEAEndGenerationFunction(this);

    //Receiving individuals if cluster island model
    if(params->remoteIslandModel){
	this->receiveIndividuals();
    }

    currentGeneration += 1;
  }
#ifdef __linux__
  if(this->params->plotStats && this->gnuplot->valid){
  	outputGraph();
  	delete this->gnuplot;
  }
#endif

  if(this->params->printFinalPopulation){
  	population->sortParentPopulation();
  	std::cout << *population << std::endl;
  }

  //IF SAVING THE POPULATION, ERASE THE OLD FILE
  if(params->savePopulation){
	
	string fichier = params->outputFilename;
	fichier.append(".pop");
	remove(fichier.c_str());
  	population->serializePopulation();
  }

  if(this->params->generateGnuplotScript || !this->params->plotStats)
	generateGnuplotScript();

  if(this->params->generateRScript)
	generateRScript();
  
  if(params->elitSize)
  	free(elitistPopulation);
}


#ifdef WIN32
void CEvolutionaryAlgorithm::showPopulationStats(clock_t beginTime){
#else
void CEvolutionaryAlgorithm::showPopulationStats(struct timeval beginTime){
#endif
  
  currentAverageFitness=0.0;
  currentSTDEV=0.0;

  //Calcul de la moyenne et de l'ecart type
  population->Best=population->parents[0];
  for(size_t i=0; i<population->parentPopulationSize; i++){
    currentAverageFitness+=population->parents[i]->getFitness();

    // here we are looking for the smaller individual's fitness if we are minimizing
    // or the greatest one if we are not
    if( (params->minimizing && population->parents[i]->getFitness()<population->Best->getFitness()) ||
    (!params->minimizing && population->parents[i]->getFitness()>population->Best->getFitness()))
      population->Best=population->parents[i];
  }

  currentAverageFitness/=population->parentPopulationSize;

  for(size_t i=0; i<population->parentPopulationSize; i++){
    currentSTDEV+=(population->parents[i]->getFitness()-currentAverageFitness)*(population->parents[i]->getFitness()-currentAverageFitness);
  }
  currentSTDEV/=population->parentPopulationSize;
  currentSTDEV=sqrt(currentSTDEV);

#ifdef WIN32
  clock_t end(clock());
  double duration;
  duration = (double)(end-beginTime)/CLOCKS_PER_SEC;
#else
  struct timeval end, res;
  gettimeofday(&end,0);
  timersub(&end,&beginTime,&res);
#endif

  //Affichage
  if(params->printStats){
	  if(currentGeneration==0)
	    printf("GEN\tTIME\t\tEVAL\tBEST\t\t\t\tAVG\t\t\t\tSTDEV\n\n");
#ifdef WIN32
            printf("%lu\t%2.6f\t%lu\t%.15e\t\t%.15e\t\t%.15e\n",currentGeneration,duration,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
#else
	    printf("%d\t%ld.%06ld\t%d\t%.15e\t\t%.15e\t\t%.15e\n",currentGeneration,res.tv_sec,res.tv_usec,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
#endif
	  //printf("%lu\t%ld.%06ld\t%lu\t%f\t%f\t%f\n",currentGeneration,res.tv_sec,res.tv_usec,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
  }

  if((this->params->plotStats && this->gnuplot->valid) || this->params->generateGnuplotScript){
 	FILE *f;
	string fichier = params->outputFilename;
	fichier.append(".dat");
 	f = fopen(fichier.c_str(),"a"); //ajouter .csv
	if(f!=NULL){
	  if(currentGeneration==0)
		fprintf(f,"#GEN\tTIME\t\tEVAL\tBEST\t\tAVG\t\tSTDEV\n\n");
#ifdef WIN32
          fprintf(f,"%lu\t%2.6f\t%lu\t%.15e\t%.15e\t%.15e\n",currentGeneration,duration,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
#else
	  fprintf(f,"%d\t%ld.%06ld\t%d\t%.15e\t%.15e\t%.15e\n",currentGeneration,res.tv_sec,res.tv_usec,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
#endif
	  fclose(f);
        }
  }
  if(params->generateCSVFile || params->generateRScript){ //Generation du fichier CSV;
 	FILE *f;
	string fichier = params->outputFilename;
	fichier.append(".csv");
 	f = fopen(fichier.c_str(),"a"); //ajouter .csv
	if(f!=NULL){
	  if(currentGeneration==0)
		fprintf(f,"GEN,TIME,EVAL,BEST,AVG,STDEV\n");
#ifdef WIN32
	  fprintf(f,"%lu,%2.6f,%lu,%.15e,%.15e,%.15e\n",currentGeneration,duration,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
#else
	  fprintf(f,"%d,%ld.%d,%d,%f,%f,%f\n",currentGeneration,res.tv_sec,res.tv_usec,population->currentEvaluationNb,population->Best->getFitness(),currentAverageFitness,currentSTDEV);
#endif
	  fclose(f);
        }
  }
  //print Gnuplot
  #ifdef __linux__
  if(this->params->plotStats && this->gnuplot->valid){
	if(currentGeneration==0)
		fprintf(this->gnuplot->fWrit,"plot \'%s.dat\' using 3:4 t \'Best Fitness\' w lines ls 1, \'%s.dat\' using 3:5 t  \'Average\' w lines ls 4, \'%s.dat\' using 3:6 t \'StdDev\' w lines ls 3\n", params->outputFilename,params->outputFilename,params->outputFilename);
	else
		fprintf(this->gnuplot->fWrit,"replot\n");
	fflush(this->gnuplot->fWrit);
 }
 
#endif

  params->timeCriterion->setElapsedTime(res.tv_sec);
}

//REMOTE ISLAND MODEL FUNCTIONS
void CEvolutionaryAlgorithm::initializeClients(){
	int clientNumber=0;
	char (*clients)[16] = (char(*)[16])calloc(1,sizeof(char)*16);
	
	cout << "Reading IP address file" << endl;
	ifstream IP_File(this->params->ipFile);
	string line;
	while(getline(IP_File, line)){
		if(!isLocalMachine(line.c_str())){
			memmove(clients[this->numberOfClients],line.c_str(),sizeof(char)*16);
			this->numberOfClients++;
			clients = (char(*)[16])realloc(clients,sizeof(char)*16*(this->numberOfClients*16));
			clientNumber++;
		}
		else{
			this->myClientNumber = clientNumber;	
		}
	}

	this->Clients = (CComUDPClient**)malloc(this->numberOfClients*sizeof(CComUDPClient*));
	for(int i=0; i<(signed)this->numberOfClients; i++){
		this->Clients[i] = new CComUDPClient(2909,(const char*)clients[i],0);
	}
	free(clients);
}

void CEvolutionaryAlgorithm::sendIndividual(){
	//Sending an individual every n generations	
	if(this->currentGeneration%(10+this->myClientNumber)==0){
		//cout << "I'm going to send an Individual now" << endl;
		//this->population->selectionOperator->initialize(this->population->parents, 7, this->population->actualParentPopulationSize);
		//size_t index = this->population->selectionOperator->selectNext(this->population->actualParentPopulationSize);
		//cout << "Going to send individual " << index << " with fitness " << this->population->parents[index]->fitness << endl;
	
		//selecting a client randomly
		int client = globalRandomGenerator->getRandomIntMax(this->numberOfClients);
		cout << "Going to send and individual to client " << client << endl;
		cout << "His IP is " << this->Clients[client]->getIP() << endl;
		//cout << "Sending individual " << index << " to client " << client << " nomw" << endl;
		//cout << this->population->parents[index]->serialize() << endl;
		this->Clients[client]->CComUDP_client_send((char*)bBest->serialize().c_str());
		
	}
}

void CEvolutionaryAlgorithm::receiveIndividuals(){
	//Checking every generation for received individuals
	if(this->treatedIndividuals<(unsigned)this->server->nb_data){
		//cout << "number of received individuals :" << this->server->nb_data << endl;
		//cout << "number of treated individuals :" << this->treatedIndividuals << endl;

		//Treating all the individuals before continuing
		while(this->treatedIndividuals < (unsigned)this->server->nb_data){
			//selecting the individual to erase
			CSelectionOperator *antiTournament = getSelectionOperator("Tournament",!this->params->minimizing, globalRandomGenerator);		
			antiTournament->initialize(this->population->parents, 7, this->population->actualParentPopulationSize);
			size_t index = antiTournament->selectNext(this->population->actualParentPopulationSize);

			//cout << "old individual fitness :" << this->population->parents[index]->fitness << endl;
			//cout << "old Individual :" << this->population->parents[index]->serialize() << endl;
			this->server->read_data_lock();
			string line = this->server->parm->data[this->treatedIndividuals].data;
			this->population->parents[index]->deserialize(line);
			this->server->read_data_unlock();
			//cout << "new Individual :" << this->population->parents[index]->serialize() << endl;
			this->treatedIndividuals++;
		}
	}
}

void CEvolutionaryAlgorithm::outputGraph(){
      	fprintf(this->gnuplot->fWrit,"set term png\n");
      	fprintf(this->gnuplot->fWrit,"set output \"%s\"\n",params->plotOutputFilename);
	fprintf(this->gnuplot->fWrit,"set xrange[0:%d]\n",population->currentEvaluationNb);
	fprintf(this->gnuplot->fWrit,"set xlabel \"Number of Evaluations\"\n");
        fprintf(this->gnuplot->fWrit,"set ylabel \"Fitness\"\n");
        fprintf(this->gnuplot->fWrit,"replot \n");
	fflush(this->gnuplot->fWrit);
}

void CEvolutionaryAlgorithm::generateGnuplotScript(){
	FILE* f;
	string fichier = this->params->outputFilename;
	fichier.append(".plot");
	f = fopen(fichier.c_str(),"a");
	fprintf(f,"set term png\n");
	fprintf(f,"set output \"%s\"\n",params->plotOutputFilename);
	fprintf(f,"set xrange[0:%d]\n",population->currentEvaluationNb);
	fprintf(f,"set xlabel \"Number of Evaluations\"\n");
        fprintf(f,"set ylabel \"Fitness\"\n");
	fprintf(f,"plot \'%s.dat\' using 3:4 t \'Best Fitness\' w lines, \'%s.dat\' using 3:5 t  \'Average\' w lines, \'%s.dat\' using 3:6 t \'StdDev\' w lines\n", params->outputFilename,params->outputFilename,params->outputFilename);
	fclose(f);	
}

void CEvolutionaryAlgorithm::generateRScript(){
	FILE* f;
	string fichier = this->params->outputFilename;
	fichier.append(".r");
	f=fopen(fichier.c_str(),"a");
	fprintf(f,"#Plotting for R\n"),
	fprintf(f,"png(\"%s\")\n",params->plotOutputFilename);
	fprintf(f,"data <- read.table(\"./%s.csv\",sep=\",\")\n",params->outputFilename);
	fprintf(f,"plot(0, type = \"n\", main = \"Plot Title\", xlab = \"Number of Evaluations\", ylab = \"Fitness\", xlim = c(0,%d) )\n",population->currentEvaluationNb);
	fprintf(f,"grid() # add grid\n");
	fprintf(f,"lines(data[,3], data[,4], lty = 1) #draw first dataset\n");
	fprintf(f,"lines(data[,3], data[,5], lty = 2) #draw second dataset\n");
	fprintf(f,"lines(data[,3], data[,6], lty = 3) #draw third dataset\n");
	fprintf(f,"legend(\"topright\", c(\"Best Fitness\", \"Average\", \"StdDev\"), lty = c(1, 2, 3) )\n");
	fclose(f);
	
}

bool CEvolutionaryAlgorithm::allCriteria(){

	for( size_t i=0 ; i<stoppingCriteria.size(); i++ ){
		if( stoppingCriteria.at(i)->reached() ){
			std::cout << "Stopping criterion reached" << std::endl;
			return true;
		}
	}
	return false;
}

#ifdef WIN32
int gettimeofday
(struct timeval* tp, void* tzp) {
	DWORD t;
	t = timeGetTime();
	tp->tv_sec = t / 1000;
	tp->tv_usec = t % 1000;
	/* 0 indicates success. */
	return 0;
}

void timersub( const timeval * tvp, const timeval * uvp, timeval* vvp )
{
	vvp->tv_sec = tvp->tv_sec - uvp->tv_sec;
	vvp->tv_usec = tvp->tv_usec - uvp->tv_usec;
	if( vvp->tv_usec < 0 )
	{
		--vvp->tv_sec;
		vvp->tv_usec += 1000000;
	}
} 
#endif

