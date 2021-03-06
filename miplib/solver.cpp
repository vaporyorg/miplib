#include "solver.hpp"

#ifdef WITH_GUROBI
#  include "gurobi/solver.hpp"
#endif

#ifdef WITH_SCIP
#  include "scip/solver.hpp"
#endif

#ifdef WITH_LPSOLVE
#  include "lpsolve/solver.hpp"
#endif

namespace miplib {

Solver::Solver(Backend backend): m_backend(backend), m_constraint_autoscale(false)
{
  if (backend == Solver::Backend::Gurobi)
  {
    #ifdef WITH_GUROBI
    p_impl = std::make_shared<GurobiSolver>();
    #else
    throw std::logic_error("Request for Gurobi backend but it was not compiled.");
    #endif
  }
  else if (backend == Solver::Backend::Scip)
  {
    #ifdef WITH_SCIP
    p_impl = std::make_shared<ScipSolver>();
    #else
    throw std::logic_error("Request for SCIP backend but it was not compiled.");
    #endif
  }
  else if (backend == Solver::Backend::Lpsolve)
  {
    #ifdef WITH_LPSOLVE
    p_impl = std::make_shared<LpsolveSolver>();
    #else
    throw std::logic_error("Request for Lpsolve backend but it was not compiled.");
    #endif
  }
  else if (backend == Solver::Backend::Any)
  {
    #if defined(WITH_GUROBI)
    p_impl = std::make_shared<GurobiSolver>();
    #elif defined(WITH_SCIP)
    p_impl = std::make_shared<ScipSolver>();
    #elif defined(WITH_LPSOLVE)
    p_impl = std::make_shared<LpsolveSolver>();
    #else
    throw std::logic_error("No MIP backends were compiled.");
    #endif
  }
}

void Solver::set_objective(Sense const& sense, Expr const& e)
{
  p_impl->set_objective(sense, e);
}

void Solver::add(Constr const& constr, bool scale)
{
  if (scale or m_constraint_autoscale)
    p_impl->add(constr.scale());
  else
    p_impl->add(constr);
}

void Solver::add(IndicatorConstr const& constr, bool scale)
{
  if (
    p_impl->m_indicator_constraint_policy == 
      Solver::IndicatorConstraintPolicy::Reformulate or
    (
      !supports_indicator_constraint(constr) and
      p_impl->m_indicator_constraint_policy == 
      Solver::IndicatorConstraintPolicy::ReformulateIfUnsupported
    ) or
    scale
  )
  {
    for (auto const& c: constr.reformulation())
      add(c, scale);
  }
  else
    p_impl->add(constr);
}

void Solver::set_lazy_constr_handler(LazyConstrHandler const& constr_handler)
{
  p_impl->set_lazy_constr_handler(constr_handler);
}

std::pair<Solver::Result, bool> Solver::solve()
{
  return p_impl->solve();
}

std::pair<Solver::Result, bool> Solver::maximize(Expr const& e)
{
  set_objective(Sense::Maximize, e);
  return solve();
}

std::pair<Solver::Result, bool> Solver::minimize(Expr const& e)
{
  set_objective(Sense::Minimize, e);
  return solve();
}

void Solver::set_non_convex_policy(NonConvexPolicy policy)
{
  p_impl->set_non_convex_policy(policy);
}

void Solver::set_indicator_constraint_policy(IndicatorConstraintPolicy policy)
{
  p_impl->set_indicator_constraint_policy(policy);
}

void Solver::set_verbose(bool value)
{
  p_impl->set_verbose(value);
}

void Solver::set_constraint_autoscale(bool autoscale)
{
  m_constraint_autoscale = autoscale;
}

void Solver::set_feasibility_tolerance(double value)
{
  p_impl->set_feasibility_tolerance(value);
}

void Solver::set_int_feasibility_tolerance(double value)
{
  p_impl->set_int_feasibility_tolerance(value);
}

bool Solver::supports_indicator_constraint(IndicatorConstr const& constr) const
{
  return p_impl->supports_indicator_constraint(constr); 
}

bool Solver::supports_quadratic_constraints() const
{
  return p_impl->supports_quadratic_constraints();
}

bool Solver::supports_quadratic_objective() const
{
  return p_impl->supports_quadratic_objective();
}

double Solver::infinity() const
{
  return p_impl->infinity();
}

void Solver::set_time_limit(double secs)
{
  p_impl->set_time_limit(secs);
}

bool Solver::supports_backend(Backend const& backend)
{
  switch (backend)
  {
    case  Solver::Backend::Gurobi:
      #ifdef WITH_GUROBI
      return true;
      #else
      return false;
      #endif
    case  Solver::Backend::Scip:
      #ifdef WITH_SCIP
      return true;
      #else
      return false;
      #endif
    case  Solver::Backend::Lpsolve:
      #ifdef WITH_LPSOLVE
      return true;
      #else
      return false;
      #endif
    default:
      return false;
  }
}

void Solver::dump(std::string const& filename) const
{
  p_impl->dump(filename);
}

namespace detail {
void ISolver::set_indicator_constraint_policy(Solver::IndicatorConstraintPolicy policy)
{
  m_indicator_constraint_policy = policy;
}

}
}  // namespace miplib
