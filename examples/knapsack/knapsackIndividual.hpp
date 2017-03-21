

#ifndef PROBLEM_DEP_H
#define PROBLEM_DEP_H

//#include "CRandomGenerator.h"
#include <stdlib.h>
#include <iostream>
#include <CIndividual.h>
#include <Parameters.h>
#include <cstring>
#include <sstream>

using namespace std;

class CRandomGenerator;
class CSelectionOperator;
class CGenerationalCriterion;
class CEvolutionaryAlgorithm;
class CPopulation;
class Parameters;

extern int EZ_POP_SIZE;
extern int OFFSPRING_SIZE;

// User classes

class Item {
public:
// Default methods for class Item
  Item(){  // Constructor
  }
  Item(const Item &EASEA_Var) {  // Copy constructor
    weight=EASEA_Var.weight;
    value=EASEA_Var.value;
  }
  virtual ~Item() {  // Destructor
  }
  string serializer() {  // serialize
  	ostringstream EASEA_Line(ios_base::app);
	EASEA_Line << this->weight << " ";
	EASEA_Line << this->value << " ";
  	return EASEA_Line.str();
  }
  void deserializer(istringstream* EASEA_Line) {  // deserialize
  	string line;
	(*EASEA_Line) >> this->weight;
	(*EASEA_Line) >> this->value;
  }
  Item& operator=(const Item &EASEA_Var) {  // Operator=
    if (&EASEA_Var == this) return *this;
    weight = EASEA_Var.weight;
    value = EASEA_Var.value;
  return *this;
  }

  bool operator==(Item &EASEA_Var) const {  // Operator==
    if (weight!=EASEA_Var.weight) return false;
    if (value!=EASEA_Var.value) return false;
  return true;
  }

  bool operator!=(Item &EASEA_Var) const {return !(*this==EASEA_Var);} // operator!=

  friend ostream& operator<< (ostream& os, const Item& EASEA_Var) { // Output stream insertion operator
    os <<  "weight:" << EASEA_Var.weight << "\n";
    os <<  "value:" << EASEA_Var.value << "\n";
    return os;
  }

// Class members 
  int weight;
  int value;
};


class IndividualImpl : public CIndividual {

public: // in knapsack the genome is public (for user functions,...)
	// Class members
  	// Class members 
  Item* itemList[1000];
  int x[1000];


public:
	IndividualImpl();
	IndividualImpl(const IndividualImpl& indiv);
	virtual ~IndividualImpl();
	float evaluate();
	static unsigned getCrossoverArrity(){ return 2; }
	float getFitness(){ return this->fitness; }
	CIndividual* crossover(CIndividual** p2);
	void printOn(std::ostream& O) const;
	CIndividual* clone();

	unsigned mutate(float pMutationPerGene);

	void boundChecking();      

	string serialize();
	void deserialize(string EASEA_Line);

	friend std::ostream& operator << (std::ostream& O, const IndividualImpl& B) ;
	void initRandomGenerator(CRandomGenerator* rg){ IndividualImpl::rg = rg;}

};


class ParametersImpl : public Parameters {
public:
	void setDefaultParameters(int argc, char** argv);
	CEvolutionaryAlgorithm* newEvolutionaryAlgorithm();
};

/**
 * @TODO ces functions devraient s'appeler weierstrassInit, weierstrassFinal etc... (en gros knapsackFinal dans le tpl).
 *
 */

void knapsackInit(int argc, char** argv);
void knapsackFinal(CPopulation* pop);
void knapsackBeginningGenerationFunction(CEvolutionaryAlgorithm* evolutionaryAlgorithm);
void knapsackEndGenerationFunction(CEvolutionaryAlgorithm* evolutionaryAlgorithm);
void knapsackGenerationFunctionBeforeReplacement(CEvolutionaryAlgorithm* evolutionaryAlgorithm);


class EvolutionaryAlgorithmImpl: public CEvolutionaryAlgorithm {
public:
	EvolutionaryAlgorithmImpl(Parameters* params);
	virtual ~EvolutionaryAlgorithmImpl();
	void initializeParentPopulation();
};

#endif /* PROBLEM_DEP_H */

