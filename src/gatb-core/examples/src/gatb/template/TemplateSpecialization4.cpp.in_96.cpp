
// since we didn't define the functions in a .h file, that trick removes linker errors,
// see http://www.parashift.com/c++-faq-lite/separate-template-class-defn-from-decl.html

#include <gatb/kmer/impl/LinearCounter.cpp>
#include <gatb/kmer/impl/MPHFAlgorithm.cpp>

/********************************************************************************/
namespace gatb { namespace core { namespace kmer { namespace impl  {
/********************************************************************************/

template class LinearCounter                <96>;
template class MPHFAlgorithm                <96>;

/********************************************************************************/
} } } } /* end of namespaces. */
/********************************************************************************/
