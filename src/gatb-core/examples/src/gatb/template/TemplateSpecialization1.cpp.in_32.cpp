
// since we didn't define the functions in a .h file, that trick removes linker errors,
// see http://www.parashift.com/c++-faq-lite/separate-template-class-defn-from-decl.html

#include <gatb/kmer/impl/Model.cpp>
#include <gatb/kmer/impl/ConfigurationAlgorithm.cpp>
#include <gatb/kmer/impl/RepartitionAlgorithm.cpp>

/********************************************************************************/
namespace gatb { namespace core { namespace kmer { namespace impl  {
/********************************************************************************/

template struct Kmer                        <32>;
template class ConfigurationAlgorithm       <32>;
template class RepartitorAlgorithm          <32>;

/********************************************************************************/
} } } } /* end of namespaces. */
/********************************************************************************/
