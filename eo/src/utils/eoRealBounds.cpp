#include <ctime>
#include <strstream>
#include "eoRealBounds.h"
#include "eoRealVectorBounds.h"

// the global dummy bounds 
// (used for unbounded variables when bounds are required)
eoRealNoBounds eoDummyRealNoBounds;
eoRealVectorNoBounds eoDummyVectorNoBounds(0);

///////////// helper read functions - could be somewhere else

// removes leading delimiters - return false if nothing else left
bool remove_leading(std::string & _s, const string _delim)
{
  size_t posDebToken = _s.find_first_not_of(_delim);
  if (posDebToken >= _s.size())
    return false;
  _s = _s.substr(posDebToken);
  return true;
}

double read_double(std::string _s)
{
  istrstream is(_s.c_str());
  double r;
  is >> r;
  return r;
}

int read_int(std::string _s)
{
  istrstream is(_s.c_str());
  int i;
  is >> i;
  return i;
}

// need to rewrite copy ctor and assignement operator because of ownedBounds
eoRealVectorBounds::eoRealVectorBounds(const eoRealVectorBounds & _b):
  eoRealBaseVectorBounds(_b)
{
  factor = _b.factor;
  ownedBounds = _b.ownedBounds;
  // duplicate all pointers!
  if (ownedBounds.size()>0)
    for (unsigned i=0; i<ownedBounds.size(); i++)
      ownedBounds[i] = ownedBounds[i]->dup();
}


// the readFrom method of eoRealVectorNoBounds:
// only calls the readFrom(string) - for param reading
void eoRealVectorBounds::readFrom(istream& _is) 
{
  string value;
  _is >> value;
  readFrom(value);
  return;
}

void eoRealVectorBounds::readFrom(std::string _value)
{
  // keep track of old size - to adjust in the end
  unsigned oldSize = size();
  // clean-up before filling in
  if (ownedBounds.size()>0)
    for (unsigned i = 0; i < ownedBounds.size(); ++i)
      {
	delete ownedBounds[i];
      }
  ownedBounds.resize(0);
  factor.resize(0);
  resize(0);
  
  // now read
  string delim(",; ");
  while (_value.size()>0)
    {
      if (!remove_leading(_value, delim)) // only delimiters were left
	break;
      // look for opening char
      size_t posDeb = _value.find_first_of("[(");
      if (posDeb >= _value.size())	// nothing left to read (though probably a syntax error there)
	{
	  break;
	}
      // ending char
      string closeChar = (_value[posDeb] == '(' ? string(")") : string("]") );

      size_t posFin = _value.find_first_of(string(closeChar));
      if (posFin >= _value.size())
	throw runtime_error("Syntax error when reading bounds");

  // y a-t-il un nbre devant
      unsigned count = 1;
      if (posDeb > 0)			// something before opening
	{
	  string sCount = _value.substr(0, posDeb);
	  count = read_int(sCount);
	  if (count <= 0)
	    throw runtime_error("Syntax error when reading bounds");
	}

      // the bounds
      string sBounds = _value.substr(posDeb+1, posFin-posDeb-1);
      // and remove from original string
      _value = _value.substr(posFin+1);
  
      remove_leading(sBounds, delim);
      size_t posDelim = sBounds.find_first_of(delim);
      if (posDelim >= sBounds.size())
	throw runtime_error("Syntax error when reading bounds");

      bool minBounded=false, maxBounded=false;
      double minBound=0, maxBound=0;

      // min bound
      string sMinBounds = sBounds.substr(0,posDelim);
      if (sMinBounds != string("-inf"))
	{
	  minBounded = true;
	  minBound = read_double(sMinBounds);
	}

      // max bound
      size_t posEndDelim = sBounds.find_first_not_of(delim,posDelim);

      string sMaxBounds = sBounds.substr(posEndDelim);
      if (sMaxBounds != string("+inf"))
	{
	  maxBounded = true;
	  maxBound = read_double(sMaxBounds);
	}

      // now create the eoRealBounds objects
      eoRealBounds *ptBounds;
      if (minBounded && maxBounded)
	ptBounds = new eoRealInterval(minBound, maxBound);
      else if (!minBounded && !maxBounded)	// no bound at all
	ptBounds = new eoRealNoBounds;
      else if (!minBounded && maxBounded)
	ptBounds = new eoRealAboveBound(maxBound);
      else if (minBounded && !maxBounded)
	ptBounds = new eoRealBelowBound(minBound);
      // store it for memory management
      ownedBounds.push_back(ptBounds);
      // push the count
      factor.push_back(count);
      // and add count of it to the actual bounds
      for (unsigned i=0; i<count; i++)
	push_back(ptBounds);
    }
  // now adjust the size to the initial value
  adjust_size(oldSize);
}

/** Eventually increases the size by duplicating last bound */
void eoRealVectorBounds::adjust_size(unsigned _dim)
{
  if ( size() < _dim )
    {
      // duplicate last bound
      unsigned missing = _dim-size();
      eoRealBounds * ptBounds = back();
      for (unsigned i=0; i<missing; i++)
	push_back(ptBounds);
      // update last factor (warning: can be > 1 already!)
      factor[factor.size()-1] += missing;
    }
}