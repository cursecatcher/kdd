#include <cmath>
#include <fstream>
#include <list>
#include <vector>

#include "cxxopts.hpp"
#include "buffer.hpp"

#define BUFFERLENGTH 10000
#define BP_A 0 
#define BP_C 1 
#define BP_G 2 
#define BP_T 3 


using namespace std; 

using domain_bounds_t = std::vector<int>;


struct stats_dd {
    const double cardinality; 
    const uint64_t num_nodes, num_edges, num_ne; 

    stats_dd(const MEDDLY::dd_edge& e) : 
        cardinality(e.getCardinality()), 
        num_nodes(e.getNodeCount()), 
        num_edges(e.getEdgeCount()), 
        num_ne(num_nodes + num_edges) {

    }
};


class KmerExtractor : std::list<short> {
    using list_t = std::list<short>;
    const std::map<char, short> bp = {
        {'A', BP_A}, {'C', BP_C}, {'G', BP_G}, {'T', BP_T}};


    std::ofstream logfile; 
    int i_dna; //internal state given a string 
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
            force_flush(edge);  
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
        /* obtaining |s|-k + 1 kmers from the given string  */ 

        clear();        //empty queue
        i_dna = k;      //set pointer to next bp 

        //init queue with first kmer 
        for (int i = 0; i < k; ++i) 
            push_back( bp.at(s.at(i)) );
        
        save2buffer(edge); 

        for (int num_kmers = s.size() - k; num_kmers > 0; --num_kmers) {
            next(s); 
            save2buffer(edge); 
        }
    }

    inline void force_flush(MEDDLY::dd_edge& edge) {
        if (buffer.num_elements()) {
            stats_dd prev(edge); 
            buffer.flush(edge); 
            stats_dd curr(edge); 

            if ( logfile.is_open() ) {
                logfile << curr.cardinality << ","
                        << curr.num_nodes << ","
                        << curr.num_edges << ","
                        << curr.num_ne << ","
                        << curr.cardinality - prev.cardinality << ","
                        << curr.num_nodes - prev.num_nodes << ","
                        << curr.num_edges - prev.num_edges << ","
                        << curr.num_ne - prev.num_ne << "\n"; 
            }
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
    }


    void set_logfile(const std::string& logfilename) {
        logfile = ofstream(logfilename, ios::out); 
    }

};



int main(int argc, char **argv) {
    cxxopts::Options options(argv[0], "DD-MERS"); 
    options.add_options()
        ("k, kl", "k-mer length", cxxopts::value<unsigned>())
        ("c, counts", "counts file generated by UST", cxxopts::value<std::string>()->default_value(""))
        ("i, inputfile", "input file generated by UST", cxxopts::value<std::string>());
        // ("o, out", "output filename", cxxopts::value<string>());

    string filename, outfile, countsfile;
    unsigned kmer_length;

    try {
        cxxopts::ParseResult result = options.parse(argc, argv);

        kmer_length = result["kl"].as<unsigned>();
        filename.assign( result["inputfile"].as<string>() );
        // outfile.assign( result["out"].as<string>() );
        countsfile.assign( result["counts"].as<string>() );

        outfile = filename.substr(0, filename.find_first_of("."));
    } catch (std::domain_error& ex) {
        cout << "missing IO parameters\n" << options.help() << endl; 
        return 1; 
    }


    MEDDLY::initialize();

    domain_bounds_t bounds(kmer_length, 4);
    MEDDLY::forest::policies policy(false); 
    policy.setFullStorage();
    MEDDLY::domain *domain = MEDDLY::createDomainBottomUp(bounds.data(), bounds.size());
    MEDDLY::forest *forest = domain->createForest(
        false, MEDDLY::forest::BOOLEAN, MEDDLY::forest::MULTI_TERMINAL, policy
    ); 
    MEDDLY::dd_edge kmer_mdd(forest); 

    cout << "Decision diagram w/ " << kmer_length << " levels initialized\n";

    KmerExtractor extr(kmer_length);
    extr.set_logfile( outfile + ".dd_log" ); 

    cout << "Collecting k-mers from UST file..." << endl; 

    try {
        ifstream fi(filename, ios::in);
        string line;

        while ( getline(fi, line) ) {
            if (line[0] != '>') { 
                extr.kmering(line, kmer_mdd); 
            }
        }
        cout << "Flushing the buffer for the last time..." << endl; 
        extr.force_flush(kmer_mdd); 
    } catch (MEDDLY::error& e) { 
        std::cout << "Meddly error: " << e.getName() << std::endl;
        return 1;
    }

    cout << "Num of k-mers: " << kmer_mdd.getCardinality() << endl;

    double max_n_kmers = pow(4, kmer_length);
    double compactness = kmer_mdd.getCardinality() / max_n_kmers * 100;

    cout << "Compactness: " << compactness << std::endl; 

    cout << "Saving k-mers..." << endl;
    extr.save_kmers(kmer_mdd, outfile + ".kmers");

    cout << "Saving decision diagram" << endl; 
    
    FILE* fp = fopen( (outfile + ".kdd").c_str(), "w");
    MEDDLY::FILE_output handler(fp);
    forest->writeEdges(handler, &kmer_mdd, 1); 
    fclose(fp);  

    return 0; 
}