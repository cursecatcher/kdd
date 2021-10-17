/*
Copyright (c) 2020

GRAPES is provided under the terms of The MIT License (MIT):

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <iostream>
#include <vector>
#include <meddly.h>
#include <meddly_expert.h>

template<class T> class BufferMultiTerminal; 
class Buffer; 


using int_vector = std::vector<int>;
using buffer_slot = int_vector*;



// template<class T>
class Buffer : private std::vector<int_vector> { 
    using base = std::vector<int_vector>; 
    using value_vector = std::vector<long>; //template? 

    //iterator to the current buffer 
    Buffer::iterator current; 
    
    //container for output values 
    value_vector values; 
    //iterator to the current return value 
    value_vector::iterator current_value; 
    //enable the use of values vector 
    const bool enable_values; 

    size_t num_current_elements; 

    std::vector<int*> pbuffer; 


    virtual void buildNewEdge(MEDDLY::dd_edge& edge) {
        //creating a bdd -  providing only variable assignments
        MEDDLY::forest *forest = edge.getForest(); 
        forest->createEdge( data(), num_elements(), edge );
    }


public: 
    //size of an internal buffer 
    const size_t element_size; 

    //constructor for empty buffer
    inline Buffer(size_t elem_size, bool set_values)     
    :   enable_values(set_values),  
        num_current_elements(0), 
        element_size(elem_size) {
    }

    //constructor for buffer of buffersize elements 
    inline Buffer(size_t buffersize, size_t elem_size, bool set_values)     
        : Buffer(elem_size, set_values) {

        resize(buffersize); 

        if (enable_values) {
            values.resize(buffersize); 
            current_value = values.begin();
        } 

        for (auto it = begin(); it != end(); ++it) {
            it->resize(element_size); 
            pbuffer.push_back(it->data());
        }

        current = begin();  
    }

    inline void flush()  {
        current = begin(); 
        num_current_elements = 0; 

        if (enable_values)
            current_value = values.begin(); 
    }

    //flush buffer content to MDD
    void flush(MEDDLY::dd_edge& edge) {
        if (num_elements()) {
            MEDDLY::forest *forest = edge.getForest();
            MEDDLY::dd_edge tmp(forest);

            try {
                buildNewEdge(tmp);  //build temporary edge depending on the DD type 
                edge += tmp;        //merge the temporary edge to the main DD  (performing PLUS or UNION operation)
                tmp.clear();        //clear temporary edge 
                flush();            //clear buffer 
            } catch (MEDDLY::error& e) {
                std::cout << "Meddly error during UNION: " << e.getName() << std::endl; 
                throw MEDDLY::error(e); 
            }
        }
    }

    /** Save the first available buffer's slot;
     * returns true if there are other slots avaiable */
    inline bool get_slot(buffer_slot& slot) {
        slot = std::addressof(*current);
        ++num_current_elements; 

        if (++current == end()) {
            current = begin(); 
            return false; 
        }

        return true; 
    }

    inline int_vector& push_slot()  {
        //append a new int_vector of length=element_size to the buffer
        emplace_back(element_size);
        int_vector& last_vector = back(); 
        pbuffer.push_back(last_vector.data()); 
        // values.push_back(value); 
        ++num_current_elements; 
        return last_vector; 
    }

    inline Buffer::base::iterator begin() { return base::begin(); }
    inline Buffer::base::iterator end() { return base::end(); }

    // inline void save_value(const long v)  {
    //     if (enable_values) {
    //         *current_value = v; 

    //         if (++current_value == values.end())
    //             current_value = values.begin(); 
    //     }
    // }

    inline int** data() { return pbuffer.data(); }

    inline size_t size() { return base::size(); }

    // inline long* values_data() {
    //     return enable_values ? values.data() : nullptr; 
    // }

    inline unsigned num_elements() const {
        return num_current_elements;
    }

    inline void show_content(std::ostream& os) const {
        for (int i = 0; i < num_current_elements; ++i) {
            for (int val: at(i))
                os << val << " ";
            // if (enable_values)
            //     os << "  ==> " << values.at(i); 
            os << "\n"; 
        }
        os << std::endl; 
    } 
}; 


template<class T>
class BufferMultiTerminal : public Buffer {
    using value_vector = std::vector<T>; 

    value_vector terminals; 
    typename value_vector::iterator it_terminals;


    virtual void buildNewEdge(MEDDLY::dd_edge& edge) {
        //creating a multiterminal dd - providing var assignments & terminal values 
        MEDDLY::forest *forest = edge.getForest(); 
        forest->createEdge( Buffer::data(), data(), num_elements(), edge );
    }

public:
    BufferMultiTerminal(size_t buffersize, size_t elem_size) : Buffer(buffersize, elem_size, false) {
        terminals.resize(buffersize);
        it_terminals = terminals.begin(); 
    }

    void save_value(const T& v) {
        *it_terminals = v; 

        if (++it_terminals == terminals.end()) {
            it_terminals = terminals.begin();
        }
    }

    const T* get_terminals() const {
        return terminals.data(); 
    }

    inline void flush() {
        Buffer::flush();
        it_terminals = terminals.begin();
    }

    //flush buffer content to MDD
    void flush(MEDDLY::dd_edge& edge) {
        Buffer::flush(edge);
        it_terminals = terminals.begin();
    }


    inline int_vector& push_slot(int tvalue)  {
        terminals.push_back(tvalue);
        return Buffer::push_slot();

        // terminals.push_back()
        // //append a new int_vector of length=element_size to the buffer
        // emplace_back(element_size);
        // int_vector& last_vector = back(); 
        // pbuffer.push_back(last_vector.data()); 
        // // values.push_back(value); 
        // ++num_current_elements; 
        // return last_vector; 
    }
};

#endif