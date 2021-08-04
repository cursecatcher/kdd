#include <gatb/gatb_core.hpp>
#include <meddly.h>
#include <meddly_expert.h>

#include "dd.hpp"
#include "buffer.hpp"

#define BUFFLENGTH 1000000
#define KMER_LENGTH 31

static const size_t span = KMER_SPAN(0);


void store_kmers(
    Kmer<span>::ModelDirect& model, 
    Kmer<span>::ModelDirect::Iterator& it, 
    dd& dd_collection, 
    const std::string& ddname, 
    encoder<char>& nucleoenc,
    Buffer& buffer) {

    int_vector *slot = nullptr; 

    for (it.first(); !it.isDone(); it.next()) {
        bool more_space = buffer.get_slot(slot); 

        std::string kmer(model.toString(it->value()));
        std::string::iterator sit = kmer.begin(); 
        int_vector::iterator vit = slot->begin() + 1;  

        for (; sit != kmer.end() && vit != slot->end(); ++sit, ++vit) 
            *vit = nucleoenc.get(*sit); 

        if (!more_space) {
            dd_collection.add_data(ddname, buffer); 
            buffer.flush(); 
        }
    }
}


int main (int argc, char* argv[])
{
    //init meddly 
    MEDDLY::initialize(); 
    std::string infilename; 

    static const char* STR_BANK1 = "-one";
    static const char* STR_BANK2 = "-two";


    size_t kmer_size = KMER_LENGTH; 

    domain_bounds_t bounds(kmer_size, 4);
    dd mydd(bounds);

    OptionsParser parser ("dd +  kmer = kmerdd");
    parser.push_back (new OptionOneParam (STR_BANK1, "fastq r1", true));
    parser.push_back (new OptionOneParam (STR_BANK2, "fastq r2", true));
    try
    {
        IProperties* options = parser.parse (argc, argv);
        // We open the two banks
        infilename.assign(options->getStr(STR_BANK1));
        IBank* bank1 = Bank::open (options->getStr(STR_BANK1));  LOCAL (bank1);
        IBank* bank2 = Bank::open (options->getStr(STR_BANK2));  LOCAL (bank2);
        
        PairedIterator<Sequence> itPair(bank1->iterator(), bank2->iterator());

        Kmer<span>::ModelDirect model(kmer_size); 
        Kmer<span>::ModelDirect::Iterator k_it(model); 
        
        Buffer buffr1(BUFFLENGTH, bounds.size() + 1, false);
        Buffer buffr2(BUFFLENGTH, bounds.size() + 1, false); 
        
        mydd.create_dd("r1");
        mydd.create_dd("r2"); 

        encoder<char> enc; 

        for (itPair.first(); !itPair.isDone(); itPair.next()) {
            k_it.setData(itPair.item().first.getData());
            store_kmers(model, k_it, mydd, "r1", enc, buffr1); 

            k_it.setData(itPair.item().second.getData());
            store_kmers(model, k_it, mydd, "r2", enc, buffr2); 
        }

        if (buffr1.num_elements()) 
            mydd.add_data("r1", buffr1); 
        
        if (buffr2.num_elements()) 
            mydd.add_data("r2", buffr2);
    }
    catch (OptionFailure& e)
    {
        return e.displayErrors (std::cout);
    }
    catch (Exception& e)
    {
        std::cerr << "EXCEPTION: " << e.getMessage() << std::endl;
    }
    
    std::string outname(infilename.substr(0, infilename.find_last_of(".")));
    outname += ".kdd"; 

    mydd.stats(); 
    mydd.save(outname);

    return 0; 
}