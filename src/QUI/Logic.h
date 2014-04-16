#ifndef QUI_LOGIC_H
#define QUI_LOGIC_H

/*!
 *  \file Logic.h 
 *
 *  \brief Defines the interface used in the Logic module.
 *
 *  This Logic module allows the set up of Rules based on state-dependent
 *  Conditions.  The evaluation is delayed, which means that the Condition is
 *  not tested until the Rule is applied.  Conditions can change their state
 *  during their life time.
 *
 *  A \typedef Condition is any function that takes zero arguments and returns a
 *  boolean.  This means the state must be held internal to the function.
 *
 *  A \typedef Action is any function that takes zero arguments and returns
 *  nothing.
 *
 *  A \typedef Rule is simply a Condition combined with up to two actions, one to
 *  take if the Condition is true and the other to take if it is false.
 *
 *
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "boost/tuple/tuple.hpp"


namespace Qui {

typedef boost::function<bool()> Condition;
typedef boost::function<void()> Action;
typedef boost::tuple<Condition, Action, Action> Rule;


inline void DoNothing() { }

//! Convenience function used to create a new Rule from an existing Condition
//! and Action(s)
inline Rule If(Condition const& condition, Action const& trueAction, 
   Action const& falseAction = DoNothing) 
{
   return boost::make_tuple(condition, trueAction, falseAction);
}



//! The following are really only requred for binding in the operators below.
inline bool Not(Condition const& cond) 
{
   return !cond(); 
}


inline bool  Or(Condition const& cond1, Condition const& cond2) 
{
   return cond1() || cond2(); 
}


inline bool And(Condition const& cond1, Condition const& cond2) 
{
   return cond1() && cond2(); 
}


inline void Plus(Action const& act1, Action const& act2) 
{
   act1(); act2(); 
}





//! The following operators can be used to combine Conditions in an intuitive
//! manner.  Note that they return new Conditions and not boolean values.
inline Condition const operator!(Condition const& cond) 
{
   return boost::bind(&Not, cond);
} 

inline Condition const operator||(Condition const& cond1, Condition const& cond2) 
{
   return boost::bind(&Or, cond1, cond2);
} 

inline Condition const operator&&(Condition const& cond1, Condition const& cond2) 
{
   return boost::bind(&And, cond1, cond2);
}

inline Action const operator+(Action const& act1, Action const& act2) 
{
   return boost::bind(&Plus, act1, act2);
}


} // end namespace Qui

#endif
