// has Node::Value all over, plus minia doesn't need them anymore.
#if 0

#include <gatb/debruijn/impl/Traversal.cpp>
#include <gatb/debruijn/impl/Terminator.cpp>
#include <gatb/debruijn/impl/Frontline.cpp>
#include <gatb/tools/math/Integer.hpp>
#include <gatb/debruijn/impl/GraphUnitigs.hpp>

using namespace gatb::core::kmer;
using namespace gatb::core::kmer::impl;

/********************************************************************************/
namespace gatb { namespace core { namespace debruijn { namespace impl  {
/********************************************************************************/

template class TraversalTemplate <NodeGU,EdgeGU, 
         GraphUnitigsTemplate<32>>; 
template class MonumentTraversalTemplate <NodeGU,EdgeGU, 
         GraphUnitigsTemplate<32>>; 
template class TerminatorTemplate <NodeGU,EdgeGU,  
         GraphUnitigsTemplate<32>>; 
template class MPHFTerminatorTemplate <NodeGU,EdgeGU, 
         GraphUnitigsTemplate<32>>; 
template class BranchingTerminatorTemplate <NodeGU,EdgeGU, 
         GraphUnitigsTemplate<32>>; 
template class FrontlineTemplate <NodeGU,EdgeGU, 
         GraphUnitigsTemplate<32>>; 
template class FrontlineBranchingTemplate <NodeGU,EdgeGU, 
         GraphUnitigsTemplate<32>>; 

/********************************************************************************/
} } } } /* end of namespaces. */
/********************************************************************************/
#endif
