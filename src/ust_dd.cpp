#include <fstream>
#include <list>

// #include "dd.hpp"
#include "cxxopts.hpp"
#include "buffer.hpp"

#define BUFFERLENGTH 5000000
#define BP_A 0 
#define BP_C 1 
#define BP_G 2 
#define BP_T 3 


using namespace std; 

using domain_bounds_t = std::vector<int>;



class KmerExtractor : std::list<short> {
    using list_t = std::list<short>;
    const std::map<char, short> bp = {
        {'A', BP_A}, {'C', BP_C}, {'G', BP_G}, {'T', BP_T}};

    int i_dna; //state
    Buffer buffer;  

    void next(const std::string& dna_seq) {
        //go to the next k-mer 
        pop_front(); 
        push_back( bp.at(dna_seq.at(i_dna++)) );
    }

    inline void save2buffer(MEDDLY::dd_edge& edge) {
        //save current kmer to buffer, eventually flushing it to dd 
        buffer_slot slot; 
        bool flag = buffer.get_slot(slot); 

        save(slot); 

        if (!flag) {  
            flush(edge);    
        }
    }

    inline void save(buffer_slot& slot) const {
        //save the current k-mer in the buffer slot 
        list_t::const_iterator it = begin(); 

        for (int_vector::iterator sit = slot->begin() + 1; sit != slot->end(); ++sit, ++it) {
            *sit = *it; 
        }
    }

    

public:
    const unsigned k;

    KmerExtractor(unsigned kmer_length) : 
        k(kmer_length), 
        buffer(BUFFERLENGTH, kmer_length + 1, false) {

    }

    void kmering(const std::string& s, MEDDLY::dd_edge& edge) {
        clear();        //empty queue
        i_dna = k;      //set pointer to next bp 

        //init queue with first kmer 
        for (int i = 0; i < k; ++i) {
            push_back( bp.at(s.at(i)) );
        } 
        save2buffer(edge); 

        for (int num_kmers = s.size() - k; num_kmers > 0; --num_kmers) {
            next(s); 
            save2buffer(edge); 
        }
    }

    inline void flush(MEDDLY::dd_edge& edge) {
        //flush buffer to dd 

        //XXX - return a pair <old, new> cardinalities
        if (buffer.num_elements()) {
            MEDDLY::forest *f = edge.getForest(); 
            MEDDLY::dd_edge tmp(f);
            f->createEdge(buffer.data(), buffer.num_elements(), tmp); 
            MEDDLY::apply(MEDDLY::UNION, edge, tmp, edge); 
            tmp.clear(); 
            buffer.flush();
        }
    }




    void print() const {
        for (list_t::const_iterator it = begin(); it != end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl; 
    }

    void save_kmers(MEDDLY::dd_edge& edge, const string& outfile) {
        ofstream fo(outfile, ios::out); 
        cout << "Writing k-mers to " << outfile << endl; 

        for (MEDDLY::enumerator e(edge); e; ++e) {
            const int *p = e.getAssignments(); 
            for (int i = 1; i <= k; ++i) {
                fo << p[i] << " "; 
            }
            fo << "\n"; 
        }
        cout << "Done." << endl; 
    }


};



int main(int argc, char **argv) {
    MEDDLY::initialize();

    cxxopts::Options options(argv[0], "DD-MERS"); 
    options.add_options()
        ("k, kl", "k-mer length", cxxopts::value<unsigned>())
        ("c, counts", "counts file generated by UST", cxxopts::value<std::string>()->default_value(""))
        ("i, inputfile", "input file generated by UST", cxxopts::value<std::string>())
        ("o, out", "output filename", cxxopts::value<string>());

    string filename, outfile, countsfile;
    unsigned kmer_length;

    try {
        cxxopts::ParseResult result = options.parse(argc, argv);

        kmer_length = result["kl"].as<unsigned>();
        filename.assign( result["inputfile"].as<string>() );
        outfile.assign( result["out"].as<string>() );
        countsfile.assign( result["counts"].as<string>() );
    } catch (std::domain_error& ex) {
        cout << "missing IO parameters\n" << options.help() << endl; 
    }

    domain_bounds_t bounds(kmer_length, 4);
    MEDDLY::forest::policies policy(false); 
    MEDDLY::domain *domain = MEDDLY::createDomainBottomUp(bounds.data(), bounds.size());
    MEDDLY::forest *forest = domain->createForest(
        false, MEDDLY::forest::BOOLEAN, MEDDLY::forest::MULTI_TERMINAL, policy
    ); 
    MEDDLY::dd_edge kmer_dd(forest); 

    cout << "Decision diagram w/ " << kmer_length << " levels initialized\n";

    KmerExtractor extr(kmer_length);
    ifstream fi(filename, ios::in);

    cout << "Collecting k-mers from UST file..." << endl; 

    try{
        for (string line; getline(fi, line); ) {
            if (line[0] != '>') { 
                extr.kmering(line, kmer_dd); 
            }
        }
        cout << "Flushing the buffer for the last time..." << endl; 
        extr.flush(kmer_dd); 
    } catch (MEDDLY::error& e) { 
        std::cout << "Meddly error: " << e.getName() << std::endl;
        return 1;
    }

    cout << "Num of k-mers: " << kmer_dd.getCardinality() << endl;

    cout << "Saving k-mers..." << endl;
    extr.save_kmers(kmer_dd, outfile + ".kmers");
    cout << "Done" << endl; 

    cout << "Saving decision diagram" << endl; 
    FILE* fp = fopen(outfile.c_str(), "w"); 
    MEDDLY::FILE_output handler(fp);
    forest->writeEdges(handler, &kmer_dd, 1); 
    fclose(fp);  

    return 0; 
}