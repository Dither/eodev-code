/** -*- mode: c++; c-indent-level: 4; c++-member-init-indent: 8; comment-column: 35; -*-

   -----------------------------------------------------------------------------
   eoPerf2Worth.h
   (c) Maarten Keijzer, Marc Schoenauer, 2001

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contact: todos@geneura.ugr.es, http://geneura.ugr.es
             Marc.Schoenauer@polytechnique.fr
             mkeijzer@dhi.dk
 */
//-----------------------------------------------------------------------------

#ifndef eoPerf2Worth_h
#define eoPerf2Worth_h

#include <utils/eoParam.h>
#include <algorithm>

/** Base class to transform raw fitnesses into fitness for selection

@see eoSelectFromWorth
*/
template <class EOT, class WorthT = double>
class eoPerf2Worth : public eoUF<const eoPop<EOT>&, void>, public eoValueParam<vector<WorthT> >
{
  public:
  eoPerf2Worth(std::string _description = "Worths"):eoValueParam<vector<WorthT> >(vector<WorthT>(0),
					      _description) {}

                  /**
  Sort population according to worth, will keep the worths and fitness_cache in sync with the population.
  */
  virtual void sort_pop(eoPop<EOT>& _pop)
  { // start with a vector of indices
    vector<unsigned> indices(_pop.size());

    for (unsigned i = 0; i < _pop.size();++i)
    { // could use generate, but who cares
      indices[i] = i;
    }

    sort(indices.begin(), indices.end(), compare_worth(value()));

    eoPop<EOT>      tmp_pop;
    tmp_pop.resize(_pop.size());
    vector<WorthT>  tmp_worths(value().size());

    for (unsigned i = 0; i < _pop.size(); ++i)
    {
      tmp_pop[i] = _pop[indices[i]];
      tmp_worths[i] = value()[indices[i]];
    }

    swap(_pop, tmp_pop);
    swap(value(), tmp_worths);
  }

  /** helper class used to sort indices into populations/worths
  */
  class compare_worth
  {
  public :
    compare_worth(const vector<WorthT>& _worths) : worths(_worths) {}

    bool operator()(unsigned a, unsigned b) const
    {
      return worths[b] < worths[a]; // sort in descending (!) order
    }

  private :

    const vector<WorthT>& worths;
  };

  virtual void resize(eoPop<EOT>& _pop, unsigned sz)
  {
    _pop.resize(sz);
    value().resize(sz);
  }

};

/**
Perf2Worth with fitness cache
*/
template <class EOT, class WorthT = typename EOT::Fitness>
class eoPerf2WorthCached : public eoPerf2Worth<EOT, WorthT>
{
  public:
  eoPerf2WorthCached(std::string _description = "Worths") : eoPerf2Worth<EOT, WorthT>(_description) {}


  /**
    Implementation of the operator(), updating a cache of fitnesses. Calls the virtual function
    calculate_worths when one of the fitnesses has changed. It is not virtual, but derived classes
    can remove the fitness caching trough the third template element
  */
  void operator()(const eoPop<EOT>& _pop)
  {
      if (fitness_cache.size() == _pop.size())
      {
        bool in_sync = true;
        for (unsigned i = 0; i < _pop.size(); ++i)
        {
          if (fitness_cache[i] != _pop[i].fitness())
          {
            in_sync = false;
            fitness_cache[i] = _pop[i].fitness();
          }
        }

        if (in_sync)
        { // worths are up to date
          return;
        }
      }
      else // just cache the fitness
      {
        fitness_cache.resize(_pop.size());
        for (unsigned i = 0; i < _pop.size(); ++i)
        {
          fitness_cache[i] = _pop[i].fitness();
        }
      }

    // call derived implementation of perf2worth mapping
    calculate_worths(_pop);
  }

  /** The actual virtual function the derived classes should implement*/
  virtual void calculate_worths(const eoPop<EOT>& _pop) = 0;

  /**
  Sort population according to worth, will keep the worths and fitness_cache in sync with the population.
  */
  virtual void sort_pop(eoPop<EOT>& _pop)
  { // start with a vector of indices
    vector<unsigned> indices(_pop.size());

    for (unsigned i = 0; i < _pop.size();++i)
    { // could use generate, but who cares
      indices[i] = i;
    }

    sort(indices.begin(), indices.end(), eoPerf2Worth<EOT,WorthT>::compare_worth(value()));

    eoPop<EOT>      tmp_pop;
    tmp_pop.resize(_pop.size());
    vector<WorthT>  tmp_worths(value().size());
    vector<typename EOT::Fitness> tmp_cache(_pop.size());

    for (unsigned i = 0; i < _pop.size(); ++i)
    {
      tmp_pop[i] = _pop[indices[i]];
      tmp_worths[i] = value()[indices[i]];

      tmp_cache[i] = fitness_cache[indices[i]];
    }

    swap(_pop, tmp_pop);
    swap(value(), tmp_worths);
    swap(fitness_cache, tmp_cache);
  }

  /** helper class used to sort indices into populations/worths
  */
  class compare_worth
  {
  public :
    compare_worth(const vector<WorthT>& _worths) : worths(_worths) {}

    bool operator()(unsigned a, unsigned b) const
    {
      return worths[b] < worths[a]; // sort in descending (!) order
    }

  private :

    const vector<WorthT>& worths;
  };

  virtual void resize(eoPop<EOT>& _pop, unsigned sz)
  {
    _pop.resize(sz);
    value().resize(sz);
    fitness_cache.resize(sz);
  }

  private :
    vector <typename EOT::Fitness> fitness_cache;
};

/**
  A dummy perf2worth, just in case you need it
*/
template <class EOT>
class eoNoPerf2Worth : public eoPerf2Worth<EOT, typename EOT::Fitness>
{
  public:

    // default behaviour, just copy fitnesses
    void operator()(const eoPop<EOT>& _pop)
    {
      value.resize(_pop.size());
      for (unsigned i = 0; i < _pop.size(); ++i)
        value()[i]=_pop[i];
    }
};

#endif