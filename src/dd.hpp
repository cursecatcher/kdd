#include <map>

#include <meddly.h>
#include <meddly_expert.h>

#include "buffer.hpp"

using domain_bounds_t = std::vector<int>;

template<class T>
class encoder; 

class ddmap; 
class dd; 



template<class T> 
class encoder {
    using keymap_t = T; 
    using valuemap_t = unsigned; 
    using map_t = std::map<keymap_t, valuemap_t>;

    map_t map; 
public:
    inline valuemap_t get(const T& x) {
        auto it = map.emplace(
            std::piecewise_construct, 
            std::forward_as_tuple(x), 
            std::forward_as_tuple(map.size())
        );
        return it.first->second;
    }

    inline bool check(const T& x) const {
        return map.find(x) != map.end(); 
    }

    inline size_t size() const {    
        return map.size(); 
    }
};

class ddmap {
    std::vector<MEDDLY::dd_edge> edges; 
    encoder<std::string> names; 
public:
    MEDDLY::dd_edge* new_dd(const std::string& name, MEDDLY::forest*& forest) {
        MEDDLY::dd_edge *new_edge = nullptr; 

        if (names.check(name) == false) {
            names.get(name);
            edges.emplace_back(forest); 
            new_edge = &(edges.back());
        }

        return new_edge;
    }

    MEDDLY::dd_edge* get(const std::string& name) {
        MEDDLY::dd_edge *p_edge = nullptr;

        if (names.check(name)) {
            unsigned index = names.get(name); 
            p_edge = &(edges.at(index));
        }
        return p_edge; 
    }

    void write(FILE* fo, MEDDLY::forest* forest) const {
        MEDDLY::FILE_output handler(fo);
        forest->writeEdges(handler, edges.data(), edges.size());
    }
}; 



class dd {
    MEDDLY::domain* domain = nullptr; 
    MEDDLY::forest* forest = nullptr; 

    ddmap edge_set;
public:
    dd(const domain_bounds_t& bounds) {
        MEDDLY::forest::policies policy(false); 
        policy.setPessimistic(); 

        domain = MEDDLY::createDomainBottomUp(bounds.data(), bounds.size()); 
        assert(domain != nullptr); 
        
        forest = domain->createForest(false, MEDDLY::forest::BOOLEAN, MEDDLY::forest::MULTI_TERMINAL, policy); 
        assert(forest != nullptr);  
    }

    ~dd() {
        MEDDLY::destroyForest(forest); 
        MEDDLY::destroyDomain(domain); 
    }

    inline void create_dd(const std::string& edge_name) {    
        if (!edge_set.new_dd(edge_name, forest))
            throw new std::exception();
    }

    void add_data(const std::string& edge_name, Buffer& buffer) {
        MEDDLY::dd_edge* edge = edge_set.get(edge_name);  

        if (edge) {
            MEDDLY::dd_edge tmp(forest), &curr = *edge; 
            forest->createEdge(buffer.data(), buffer.num_elements(), tmp); 
            MEDDLY::apply(MEDDLY::UNION, curr, tmp, curr); 
            tmp.clear(); 
        }
    }

    void stats() const {
        std::cout 
            << "num nodes: " << forest->getCurrentNumNodes() << "\n"
            << "peak nodes: " << forest->getPeakNumNodes() << "\n"
            << "memory used: " << forest->getCurrentMemoryUsed() << "\n"
            << "peak memory: " << forest->getPeakMemoryUsed() << "\n";    
    }


    void save(const std::string& outname) const {
        FILE *fp = fopen(outname.c_str(), "w"); 
        edge_set.write(fp, forest); 
        fclose(fp); 
    }
};