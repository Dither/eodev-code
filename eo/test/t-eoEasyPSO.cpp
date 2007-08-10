//-----------------------------------------------------------------------------
// t-eoEasyPSO.cpp
//-----------------------------------------------------------------------------

#ifndef __GNUG__
// to avoid long name warnings
#pragma warning(disable:4786)
#endif // __GNUG__

#include <eo>

//-----------------------------------------------------------------------------
typedef eoMinimizingFitness FitT;
typedef eoRealParticle < FitT > Particle;
//-----------------------------------------------------------------------------

// the objective function
double real_value (const Particle & _particle)
{
    double sum = 0;
    for (unsigned i = 0; i < _particle.size ()-1; i++)
        sum += pow(_particle[i],2);
    return (sum);
}


main()
{
    const unsigned int VEC_SIZE = 2;
    const unsigned int POP_SIZE = 20;
    const unsigned int NEIGHBORHOOD_SIZE= 5;
    unsigned i;

    // the population:
    eoPop<Particle> pop;

    // Evaluation
    eoEvalFuncPtr<Particle, double, const Particle& > eval(  real_value );

    // position init
    eoUniformGenerator < double >uGen (-3, 3);
    eoInitFixedLength < Particle > random (VEC_SIZE, uGen);

    // velocity init
    eoUniformGenerator < double >sGen (-2, 2);
    eoVelocityInitFixedLength < Particle > veloRandom (VEC_SIZE, sGen);

    // local best init
    eoFirstIsBestInit < Particle > localInit;


    // perform initialization
    pop.append (POP_SIZE, random);
    apply < Particle > (eval, pop);
    apply < Particle > (veloRandom, pop);
    apply < Particle > (localInit, pop);


    std::cout << "population:" << std::endl;
    for (i = 0; i < pop.size(); ++i)
        std::cout << "\t" << pop[i] << " " << pop[i].fitness() << std::endl;


    // topology
    eoLinearTopology<Particle> topology(NEIGHBORHOOD_SIZE);
    topology.setup(pop);

    // bounds
    eoRealVectorBounds bnds(VEC_SIZE,-1.5,1.5);

    // velocity
    eoStandardVelocity <Particle> velocity (topology,1.6,2,bnds);

    // flight
    eoStandardFlight <Particle> flight;

    // Terminators
    eoGenContinue <Particle> genCont (50);


    // PS flight
    eoEasyPSO<Particle> pso(genCont, eval, velocity, flight);


    // flight
    try
    {
        pso(pop);
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;;
        exit(EXIT_FAILURE);
    }

    std::cout << "pop" << std::endl;
    for (i = 0; i < pop.size(); ++i)
        std::cout << "\t" <<  pop[i] << " " << pop[i].fitness() << std::endl;

    return 0;
}

//-----------------------------------------------------------------------------