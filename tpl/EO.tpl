\TEMPLATE_START//********************************************
//                                         
//  EASEA.cpp
//                                         
//  C++ file generated by AESAE-EO Millennium Edition (v0.6)
//                                         
//********************************************

#include <stdexcept>  // runtime_error 
#include <iostream>   // cout
#include <strstream>  // ostrstream, istrstream
#include <fstream>
#include <eo>   
#include <other/eoExternalEO.h> 
#include <other/eoExternalOpFunctions.h> 

unsigned  * pCurrentGeneration;
unsigned  * pEZ_NB_GEN;
double EZ_MUT_PROB, EZ_XOVER_PROB, EZ_REPL_PERC=0;
int EZ_NB_GEN, EZ_POP_SIZE;
unsigned long EZ_NB_EVALUATIONS=0;

inline int random(int b1=0, int b2=1){
  return rng.random(b2-b1)+b1;
}
inline double random(double b1=0, double b2=1){
  return rng.uniform(b2-b1)+b1;
}
inline float random(float b1=0, float b2=1){
  return rng.uniform(b2-b1)+b1;
}

\ANALYSE_PARAMETERS
\INSERT_USER_DECLARATIONS
\INSERT_INITIALISATION_FUNCTION \ANALYSE_USER_CLASSES
\INSERT_USER_CLASSES
struct EASEAGenome{
public:
  EASEAGenome::EASEAGenome() {
\GENOME_CTOR  }
  EASEAGenome(const EASEAGenome & orig) {
\GENOME_CTOR    copy(orig);
  }
  virtual ~EASEAGenome() {
\GENOME_DTOR  }
  EASEAGenome& operator=(const EASEAGenome &);
  virtual void copy(const EASEAGenome & c);

\INSERT_GENOME};

EASEAGenome& EASEAGenome::operator=(const EASEAGenome & arg) { 
  copy(arg); 
  return *this;
}

void EASEAGenome::copy(const EASEAGenome& genome) {
  if(&genome != this){
\GENOME_DTOR
\COPY_CTOR  }
}

typedef eoExternalEO<\MINIMAXI, EASEAGenome> Chrom;

istream & operator>>(istream &is, struct EASEAGenome &genome) {
\READ
return is;
}

ostream & operator<<(ostream &os, const struct EASEAGenome &genome) {
\WRITE
return os;
}
\INSERT_USER_FUNCTIONS
\INSERT_INITIALISER

\INSERT_CROSSOVER

\INSERT_MUTATOR

\INSERT_EVALUATOR
int main_function(int argc, char **argv) {
  // define a parser from the command-line arguments
    eoParser parser(argc, argv);

    // For each parameter, define Parameter, read it through the parser,
    // and assign the value to the variable
    eoValueParam<uint32> seedParam(time(0), "seed", "Random number seed", 'S');
    parser.processParam( seedParam );
    uint32 seed = seedParam.value();

// the parameter found in the .ez file is set as a default value
    eoValueParam<unsigned int> popSizeParam(\POP_SIZE, "POP_SIZE", "Population size",'P');
    parser.processParam( popSizeParam, "Evolution engine" );
    EZ_POP_SIZE = popSizeParam.value();

    eoValueParam<unsigned int> maxGenParam(\NB_GEN, "NB_GEN", "Maximum number of generations",'G');
    parser.processParam( maxGenParam, "Stopping criterion" );
    EZ_NB_GEN = maxGenParam.value();
    pEZ_NB_GEN = & maxGenParam.value();

    eoValueParam<double> pCrossParam(\XOVER_PROB, "XOVER_PROB", "Probability of Crossover", 'C'); 
    parser.processParam( pCrossParam, "Genetic Operators" );
    EZ_XOVER_PROB = pCrossParam.value();

    eoValueParam<double> pMutParam(\MUT_PROB, "MUT_PROB", "Probability of Mutation", 'M');
    parser.processParam( pMutParam, "Genetic Operators" );
    EZ_MUT_PROB = pMutParam.value();

    eoValueParam<unsigned int> freqSavParam(0, "freqSav", "Save population every freqSav generations (0=final only)",'f');
    parser.processParam( freqSavParam, "Persistence" );
    unsigned freqSav = freqSavParam.value();

    eoValueParam<unsigned int> boolGnuplotParam(1, "gnuplot", "Interactive gnuplot display (0/1)",'g');
    parser.processParam( boolGnuplotParam, "Persistence" );
    unsigned boolGnuplot = boolGnuplotParam.value();

    string str_status = "EASEA.status";
    eoValueParam<string> statusParam(str_status.c_str(), "status","Status file",'S');
 
     parser.processParam( statusParam, "Persistence" );

    if (parser.userNeedsHelp()){
        parser.printHelp(cout);
        exit(1);
      }

    if (statusParam.value() != ""){
       ofstream os(statusParam.value().c_str());
       os << parser;   // and you can use that file as parameter file
      }

   eoExternalInit<\MINIMAXI,EASEAGenome> ezInitialiser(&InitialiserFunction);
   eoExternalMonOp<\MINIMAXI,EASEAGenome> ezMutation(&MutationFunction);
   eoExternalQuadOp<\MINIMAXI,EASEAGenome> ezXOver(&CrossoverFunction);
   eoExternalEvalFunc<\MINIMAXI,EASEAGenome> ezExtEval(&EvaluationFunction);
   // ... to an object that counts the nb of actual evaluations
   eoEvalFuncCounter<Chrom> ezEval(ezExtEval);

  //reproductible random seed: if you don't change SEED above, 
  // you'll aways get the same result, NOT a random run
  rng.reseed(seed);
\INSERT_INIT_FCT_CALL
  // Initialization of the population
  eoPop<Chrom> pop(EZ_POP_SIZE, ezInitialiser);

  // and evaluate it (STL syntax)    
  apply<Chrom>(ezEval, pop);

  // Print best guy in intial population
  cout << "Best individual in initial population:\n" << pop.best_element() << endl;

  //////////////////////////////////////
  // The variation operators
  //////////////////////////////////////

  // The operators are  encapsulated into an eoTRansform object
  eoDynSGATransform<Chrom> transform(ezXOver, EZ_XOVER_PROB, ezMutation, EZ_MUT_PROB);
  
  /////////////////////////////////////
  // selection and replacement
  ////////////////////////////////////

  eo\SELECTORSelect<Chrom> selectOne\SELECT_PRM;
  eoSelectPerc<Chrom> select(selectOne,\REPL_PERC);

  eo\REPLACEMENTReplacement<Chrom> \ELITIST_REPLACE_NAMEeplace\DISCARD_PRM; 
\ELITISM

  eoGenContinue<Chrom> continuator(EZ_NB_GEN);
  
  // Declare a checkpoint (from a continuator)
  eoCheckPoint<Chrom> checkpoint(continuator);
  
  // Create a counter parameter
  eoValueParam<unsigned> generationCounter(0, "Generation");
  
  // Create an incrementor (which is an eoUpdater). Note that the 
  // Parameter's value is passed by reference, so every time the incrementer increments,
  // the data in generationCounter will change.
  eoIncrementor<unsigned> increment(generationCounter.value());

  // Add it to the checkpoint, this will result in the counter being incremented every generation
  checkpoint.add(increment);

  // now some statistics on the population:
  // Best fitness in population
  eoBestFitnessStat<Chrom> bestStat;
  eoAverageStat<Chrom> averageStat;
  // Second moment stats: average and stdev
  eoSecondMomentStats<Chrom> SecondStat;

  // Add them to the checkpoint to get them called at the appropriate time
  checkpoint.add(bestStat);
  checkpoint.add(averageStat);
  checkpoint.add(SecondStat);

  // The Stdout monitor will print parameters to the screen ...
  eoStdoutMonitor monitor(false);
   
  // when called by the checkpoint (i.e. at every generation)
  checkpoint.add(monitor);

  // the monitor will output a series of parameters: add them 
  monitor.add(generationCounter);
  monitor.add(ezEval);  // because now eval is an eoEvalFuncCounter!
  monitor.add(bestStat);
  monitor.add(SecondStat);

  // A file monitor: will print parameters to ... a File, yes, you got it!
  eoFileMonitor fileMonitor("stats.xg", " ");

  // interactive output: only if user wants it - hence have to use a pointer
  eoMonitor *gnuMonitor;

  if (boolGnuplot)    // plot only if user requires
    gnuMonitor = new   eoGnuplot1DMonitor("best_average.xg");
  else        // but create file anyway
    gnuMonitor = new   eoFileMonitor("best_average.xg");

   
  // the checkpoint mechanism can handle multiple monitors
  checkpoint.add(fileMonitor);
  checkpoint.add(*gnuMonitor);

  // the fileMonitor can monitor parameters, too, but you must tell it!
  fileMonitor.add(generationCounter);
  fileMonitor.add(bestStat);
  fileMonitor.add(SecondStat);
  // the fileMonitor can monitor parameters, too, but you must tell it!
  gnuMonitor->add(ezEval);
  gnuMonitor->add(bestStat);
  gnuMonitor->add(averageStat);

  eoState outState;
  // Register the algorithm into the state (so it has something to save!!)
  outState.registerObject(parser);
  outState.registerObject(pop);
  outState.registerObject(rng);

  // and feed the state to state savers
  // save state every freqSav  generation
  unsigned int freq = freqSav;
  if (!freqSav)      // only final generation
      freq = EZ_NB_GEN+1;    // so no save will take place before the end
  eoCountedStateSaver stateSaver(freq, outState, "generation", true);
  // Don't forget to add the saver to the checkpoint
  checkpoint.add(stateSaver);

  /////////////////////////////////////////
  // the algorithm
  ////////////////////////////////////////
  // Easy EA requires 
  // selection, transformation, eval, replacement, and stopping criterion
  eoEasyEA<Chrom> gga(checkpoint, ezEval, select, transform, replace);

  // Apply algo to pop
  gga(pop);
  
  // print best final guy
  cout << "\nBest individual in final population:\n" << pop.best_element() << endl;

  // will have to do this more properly - cf eoFunctorStore
  delete  gnuMonitor;
  return 0;
}

// A main that catches the exceptions
int main(int argc, char **argv)
{
#ifdef _MSC_VER
    int flag = _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
     flag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(flag);
//   _CrtSetBreakAlloc(100);
#endif

    try
    {
        main_function(argc, argv);
    }
    catch(exception& e)
    {
        cout << "Exception: " << e.what() << '\n';
    }

    return 1;
}

\TEMPLATE_END
